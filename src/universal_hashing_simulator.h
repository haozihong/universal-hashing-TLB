#pragma once

#include "vm_simulator.h"
#include "constants+helper.h"
#include "page_frame.h"

#include <cstdio>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define XXH_STATIC_LINKING_ONLY // should keep this marco for xxhash
#define XXH_IMPLEMENTATION      // should keep this marco for xxhash
#include "include/xxhash.h"

class UniversalHashingSimulator : public VmSimulator {

enum Mode {
  M_STATIC,
  M_DYNAMIC_INDIE_HASH,
  M_DYNAMIC_ONE_HASH,
};

// function pointer to the function that is used to hash the VPN
using Indexer = uint32_t(UniversalHashingSimulator::*)(uint64_t, uint64_t, int);

inline static std::unordered_map<std::string, Mode> options_map {
  { "uni-static", M_STATIC },
  { "uni-dyn", M_DYNAMIC_ONE_HASH},
  { "uni-dyn-ind", M_DYNAMIC_INDIE_HASH },
};

public:
  UniversalHashingSimulator(double mem_size_mb, int bank_count, const std::string& mode)
      : bank_count(bank_count) {

    if (options_map.count(mode) == 1) {
      sim_mode = options_map[mode];
    }

    frame_per_bank = mem_size_mb * 1024 / PAGE_SIZE_KB / bank_count;

    printf("Universal Hashing Simulator initializing, \n");
    printf("----------------\n");
    printf("sim_mode = %s\n", mode.c_str());
    printf("bank_count = %d\n", bank_count);
    printf("frame_per_bank = %d\n", frame_per_bank);

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
  }

  void access(uint64_t addr, char rw) override {
    time_tick += 1;
    stats.total_mem_access += 1;

    uint64_t vpn = get_page_number(addr);
    vpn_set.insert(vpn);

    uint64_t vpn_hashed = 0;
    if (sim_mode == M_DYNAMIC_ONE_HASH) {
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

private:

  uint32_t get_index_in_bank_static(uint64_t vpn, uint64_t vpn_hashed, int bank_index) {
    return vpn % (uint64_t)frame_per_bank;
  }

  // f(h(x), bank_index)
  uint32_t get_index_in_bank_dynamic(uint64_t vpn, uint64_t vpn_hashed, int bank_index) {
    uint64_t vpn_bank_mix =  vpn_hashed ^ (uint64_t)bank_index;
    return XXH64(&vpn_bank_mix, sizeof(vpn_bank_mix), 0) % (uint64_t)frame_per_bank;
  }

  uint32_t get_index_in_bank_dynamic_indie(uint64_t vpn, uint64_t vpn_hashed, int bank_index) {
    return XXH64(&vpn, sizeof(vpn), bank_index) % (uint64_t)frame_per_bank;
  }

  int bank_count;
  int frame_per_bank;

  // map VPN to CPFN
  std::unordered_map<uint64_t, uint32_t> page_table;
  std::vector<std::vector<PageFrame>> memory;

  Mode sim_mode {Mode::M_DYNAMIC_ONE_HASH};
  Indexer indexer {nullptr};

  uint64_t time_tick {0};
};
