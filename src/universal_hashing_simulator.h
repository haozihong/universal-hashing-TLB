#pragma once

#include "vm_simulator.h"
#include "constants+helper.h"
#include "page_frame.h"

#include <cstdio>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>

#define XXH_STATIC_LINKING_ONLY // should keep this marco for xxhash
#define XXH_IMPLEMENTATION      // should keep this marco for xxhash
#include "include/xxhash.h"

class UniversalHashingSimulator : public VmSimulator {

enum Mode {
  M_STATIC,
  M_DYNAMIC_INDIE_HASH,
  M_DYNAMIC_ONE_HASH,
  M_DYNAMIC_ONE_HASH_WITH_TABLE,
  M_DYNAMIC_ONE_HASH_XOR
};

// function pointer to the function that is used to hash the VPN
using Indexer = uint32_t(UniversalHashingSimulator::*)(uint64_t, uint64_t, int);

inline static std::unordered_map<std::string, Mode> options_map {
  { "uni-static", M_STATIC },
  { "uni-dyn", M_DYNAMIC_ONE_HASH},
  { "uni-dyn-ind", M_DYNAMIC_INDIE_HASH },
  { "uni-dyn-tbl", M_DYNAMIC_ONE_HASH_WITH_TABLE },
  { "uni-dyn-xor", M_DYNAMIC_ONE_HASH_XOR }
};

public:
  UniversalHashingSimulator(double mem_size_mb, int bank_count, const std::string& mode)
      : bank_count(bank_count), sim_mode_name(mode) {

    if (options_map.count(mode) == 1) {
      sim_mode = options_map[mode];
    }

    frame_per_bank = mem_size_mb * 1024 / PAGE_SIZE_KB / bank_count;

    print_info();

    memory.resize(bank_count);
    for (auto& bank : memory) {
      bank.resize(frame_per_bank);
    }

    // Select a hash function according to the hash strategy
    if (sim_mode == M_STATIC) {
      indexer = &UniversalHashingSimulator::get_index_in_bank_static;
    }
    else if (sim_mode == M_DYNAMIC_INDIE_HASH) {
      indexer = &UniversalHashingSimulator::get_index_in_bank_dynamic_indie;
    }
    else if (sim_mode == M_DYNAMIC_ONE_HASH) {
      indexer = &UniversalHashingSimulator::get_index_in_bank_dynamic;
    }
    else if (sim_mode == M_DYNAMIC_ONE_HASH_XOR) {
      indexer = &UniversalHashingSimulator::get_index_in_bank_dynamic_xor;
    }
    else if (sim_mode == M_DYNAMIC_ONE_HASH_WITH_TABLE) {
      indexer = &UniversalHashingSimulator::get_index_in_bank_with_table;

      std::mt19937 generator(1u);
      std::uniform_int_distribution<int> distribution(INT_MIN, INT_MAX);

      offset_table.resize(1 << offset_table_size_bit);
      for (auto& row : offset_table) {
        row.resize(bank_count);
        for (int i = 0; i < bank_count; i++) {
          row[i] = distribution(generator);
        }
      }
    }
  }

  void access(uint64_t addr, char rw) override {
    time_tick += 1;
    stats.total_mem_access += 1;

    uint64_t vpn = get_page_number(addr);
    vpn_set.insert(vpn);

    uint64_t vpn_hashed = 0;
    if (sim_mode == M_DYNAMIC_ONE_HASH || sim_mode == M_DYNAMIC_ONE_HASH_WITH_TABLE) {
      vpn_hashed = XXH64(&vpn, sizeof(vpn), 0);
    }

    auto find_res = page_table.find(vpn);

    if (find_res != page_table.end()) {
      // page is in the memory
      uint32_t bank_idx = find_res->second;
      uint32_t frame_idx = (this->*indexer)(vpn, vpn_hashed, bank_idx);

      memory[bank_idx][frame_idx].timestamp = time_tick;
    }
    else {
      // page is not in the memory, should find a frame for it
      stats.num_page_fault += 1;

      uint32_t bank_selected = 0;
      uint64_t min_lru_time = UINT64_MAX;
      PageFrame *frame_selected = nullptr;

      bool need_evict = true;

      #ifdef DBG
      printf("VPN: %lld\n", vpn);
      printf("frame index: ");
      
      for (int bank = 0; bank < bank_count; bank++) {
        uint32_t frame_idx = (this->*indexer)(vpn, vpn_hashed, bank);
        printf("%u, ", frame_idx);
      }
      printf("\n");
      #endif

      // Check all possible frames, if an empty frame is found, occupy it without evicting a page.
      // If there is no empty frame, evict a page according to the LRU policy.
      for (int bank = 0; bank < bank_count; bank++) {

        uint32_t frame_idx = (this->*indexer)(vpn, vpn_hashed, bank);
        auto& the_frame = memory[bank][frame_idx];

        if (the_frame.free) {
          need_evict = false;
          bank_selected = bank;
          frame_selected = &the_frame;
          break;
        }

        if (the_frame.timestamp < min_lru_time) {
          min_lru_time = the_frame.timestamp;
          bank_selected = bank;
          frame_selected = &the_frame;
        }
      }

      if (need_evict) {
        // If it's the first swap, record memory utilization
        if (stats.num_swap_out == 0) {
          stats.mem_util_pct = (double)vpn_set.size() / (bank_count * frame_per_bank);
        }
        stats.num_swap_out += 1;
        stats.total_age_of_swapped_out_pages += time_tick - frame_selected->timestamp;
        page_table.erase(frame_selected->vpn);
      }

      page_table[vpn] = bank_selected;
      frame_selected->vpn = vpn;
      frame_selected->free = false;
      frame_selected->timestamp = time_tick;
    }
  }

  virtual void print_info(std::ostream& os = std::cout) override {
    os << "Simulator: Universal Hashing Simulator\n"
       << "----------------"
       << "\nsim_mode = " << sim_mode_name 
       << "\nbank_count = " << bank_count
       << "\nframe_per_bank = " << frame_per_bank
       << "\n" << std::endl;
  }

private:
  uint32_t xorBits(uint64_t low64, uint64_t high64, int ord) {
    uint64_t low32;
    uint64_t high32;

    /*
     *    high64            low64
     *|________________|________________|
     *     |________|________|
     *       high32   low32
     *              ^                   ^
     *              |--------ord--------|
     */

    if (0 <= ord && ord <= 32) {
        high32 = low64 >> ord;
        low32 = (low64 << (32 - ord)) | (high64 >> (32 + ord));
    }
    else if (33 <= ord && ord <= 64) {
        high32 = (low64 >> ord) | (high64 << (64 - ord));
        low32 = low64 >> (ord - 32);
    }
    else if (65 <= ord && ord <= 96) {
        high32 = high64 >> (ord - 64);
        low32 = (high64 << (96 - ord)) | (low64 >> (ord - 32));
    }
    else if (97 <= ord && ord <= 127) {
        high32 = high64 >> (ord - 64) | (low64 << (128 - ord));
        low32 = high64 >> (ord - 96);
    } else {
      high32 = 0;
      low32 = 0;
    }
    
    return (low32 ^ high32) & 0xFFFFFFFF;
  }

  uint32_t get_index_in_bank_static(uint64_t vpn, uint64_t vpn_hashed, int bank_index) {
    return vpn % (uint64_t)frame_per_bank;
  }

  // f(h(x), bank_index)
  uint32_t get_index_in_bank_dynamic(uint64_t vpn, uint64_t vpn_hashed, int bank_index) {
    uint64_t vpn_bank_mix =  vpn_hashed ^ (uint64_t)bank_index;
    return XXH64(&vpn_bank_mix, sizeof(vpn_bank_mix), 0) % (uint64_t)frame_per_bank;
  }

  uint32_t get_index_in_bank_with_table(uint64_t vpn, uint64_t vpn_hashed, int bank_index) {
    std::vector<int>& table_row = offset_table[vpn_hashed >> (64 - offset_table_size_bit)];
    uint64_t idx = (vpn_hashed + table_row[bank_index]) % (uint64_t)frame_per_bank;
    return idx;
  }

  uint32_t get_index_in_bank_dynamic_xor(uint64_t vpn, uint64_t vpn_hashed, int bank_index) {
    XXH128_hash_t res = XXH128(&vpn, sizeof(vpn), 0);
    return xorBits(res.low64, res.high64, bank_index) % (uint64_t)frame_per_bank;
  }

  uint32_t get_index_in_bank_dynamic_indie(uint64_t vpn, uint64_t vpn_hashed, int bank_index) {
    return XXH64(&vpn, sizeof(vpn), bank_index) % (uint64_t)frame_per_bank;
  }

  int bank_count;
  int frame_per_bank;

  // map VPN to CPFN
  std::unordered_map<uint64_t, uint32_t> page_table;
  std::vector<std::vector<PageFrame>> memory;

  std::string sim_mode_name;
  Mode sim_mode {Mode::M_DYNAMIC_ONE_HASH};
  Indexer indexer {nullptr};

  uint64_t time_tick {0};

  // for table-based secondary hash
  std::vector<std::vector<int>> offset_table;
  int offset_table_size_bit {5};

};
