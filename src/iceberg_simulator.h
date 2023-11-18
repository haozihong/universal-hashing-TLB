#pragma once

#include <algorithm>
#include <cstdlib>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <utility>

#include "constants+helper.h"
#include "page_frame.h"
#include "vm_simulator.h"

#define XXH_STATIC_LINKING_ONLY // should keep this marco for xxhash
#define XXH_IMPLEMENTATION      // should keep this marco for xxhash
#include "include/xxhash.h"

class IcebergSimulator : public VmSimulator {
public:
  IcebergSimulator(double mem_size_mb, int frontyard_size, int backyard_size)
      : fyard_size(frontyard_size), byard_size(backyard_size),
        yard_num(mem_size_mb * 1024 / PAGE_SIZE_KB / (frontyard_size + backyard_size)),
        mem_fyards(yard_num), mem_byards(yard_num), byard_avail(yard_num, backyard_size),
        byard_candi(byard_candi_num) {
    for (auto& yard : mem_fyards) {
      yard.resize(fyard_size);
    }
    for (auto& yard : mem_byards) {
      yard.resize(byard_size);
    }
  }

  void access(uint64_t addr, char rw) override {
    time_tick += 1;
    stats.total_mem_access += 1;

    uint64_t vpn = get_page_number(addr);
    vpn_set.insert(vpn);

    auto find_res = page_table.find(vpn);
    if (find_res != page_table.end()) {
      // page is in the memory
      uint32_t cpfn = find_res->second;
      auto *frame_p = find_frame(vpn, cpfn);
      frame_p->timestamp = time_tick;
      return;
    }

    // page is not in the memory, should find a frame for it
    stats.num_page_fault += 1;

    PageFrame *victim_frame = nullptr;
    uint32_t victim_cpfn = 0;
    do {
      auto [fyard_frame, fyard_cpfn] = pick_from_frontyard(vpn);
      if (fyard_frame->free) {
        victim_frame = fyard_frame;
        victim_cpfn = fyard_cpfn;
        break;
      }
      auto [byard_frame, byard_cpfn, byard_idx] = pick_from_backyards(vpn);
      if (byard_frame->free) {
        victim_frame = byard_frame;
        victim_cpfn = byard_cpfn;
        byard_avail[byard_idx]--;
        break;
      }

      // if no free frame
      // If it's the first swap, record memory utilization
      if (stats.num_swap_out == 0) {
        size_t page_cnt = vpn_set.size();
        size_t total_frame_cnt = yard_num * (fyard_size + byard_size);
        stats.mem_util_pct = (double)page_cnt / total_frame_cnt;
      }
      stats.num_swap_out += 1;
        
      if (fyard_frame->timestamp < byard_frame->timestamp) {
        victim_frame = fyard_frame;
        victim_cpfn = fyard_cpfn;
      }
      else {
        victim_frame = byard_frame;
        victim_cpfn = byard_cpfn;
      }
      stats.total_age_of_swapped_out_pages += time_tick - victim_frame->timestamp;

    } while(0);

    // eviction process
    page_table.erase(victim_frame->vpn);

    page_table[vpn] = victim_cpfn;
    victim_frame->vpn = vpn;
    victim_frame->free = false;
    victim_frame->timestamp = time_tick;
  }

private:
  uint64_t time_tick {0};

  size_t fyard_size;
  size_t byard_size;
  int yard_num;
  static constexpr int byard_candi_num = 6;

    // map VPN to CPFN
  std::unordered_map<uint64_t, uint32_t> page_table;
  std::vector<std::vector<PageFrame>> mem_fyards;
  std::vector<std::vector<PageFrame>> mem_byards;
  std::vector<int> byard_avail;

  std::vector<uint64_t> byard_candi;

  uint64_t iceberg_hash(uint64_t vpn, int hash_index) {
    return XXH64(&vpn, sizeof(vpn), hash_index); 
  }

  // Returns the first free page in the frontyard.
  // Otherwise return the frame with oldest timestamp.
  // Returns <PageFrame*, CPFN>
  std::pair<PageFrame*, uint32_t> pick_from_frontyard(uint64_t vpn) {
    int fyard_id = iceberg_hash(vpn, 0) % yard_num;
    for (size_t j = 0; j < fyard_size; j++) {
      if (mem_fyards[fyard_id][j].free) {
        return {&mem_fyards[fyard_id][j], j};
      }
    }
    auto it = min_element(mem_fyards[fyard_id].begin(), mem_fyards[fyard_id].end(),
                          [](auto& f1, auto& f2) { return f1.timestamp < f2.timestamp; });
    return {&*it, it - mem_fyards[fyard_id].begin()};
  }
  
  // Returns the first free page in the most vacant backyard.
  // Otherwise return the frame with oldest timestamp.
  // Returns <PageFrame*, CPFN, backyard index>
  std::tuple<PageFrame*, uint32_t, size_t> pick_from_backyards(uint64_t vpn) {
    for (int i = 0; i < byard_candi_num; i++) {
      byard_candi[i] = iceberg_hash(vpn, i + 1) % yard_num;
    }
    int max_avail_candi_index = -1;
    int max_avail_bucket_size = 0;
    for (int i = 0; i < byard_candi_num; i++) {
      auto byard_idx = byard_candi[i];
      if (max_avail_bucket_size < byard_avail[byard_idx]) {
        max_avail_candi_index = i;
        max_avail_bucket_size = byard_avail[byard_idx];
      }
    }
    if (max_avail_bucket_size > 0) {
      // search for a free frame
      auto byard_idx = byard_candi[max_avail_candi_index];
      for (size_t i = 0; i < byard_size; i++) {
        if (mem_byards[byard_idx][i].free) {
          return {&mem_byards[byard_idx][i], fyard_size + max_avail_candi_index * byard_size + i, byard_idx};
        }
      }

      // should never reach here
      std::exit(EXIT_FAILURE);
    }
    else {
      size_t oldest_candi_id = 0, oldest_offset = 0;
      for (size_t i = 0; i < byard_candi.size(); i++) {
        for (size_t j = 0; j < byard_size; j++) {
          if (mem_byards[byard_candi[i]][j].timestamp <
              mem_byards[byard_candi[oldest_candi_id]][oldest_offset].timestamp) {
            oldest_candi_id = i;
            oldest_offset = j;
          }
        }
      }
      return {&mem_byards[byard_candi[oldest_candi_id]][oldest_offset],
              fyard_size + oldest_candi_id * byard_size + oldest_offset,
              byard_candi[oldest_candi_id]};
    }
  }

  PageFrame *find_frame(uint64_t vpn, uint32_t cpfn) {
    // if frame in front yard
    if (cpfn < fyard_size) {
      int idx = iceberg_hash(vpn, 0) % fyard_size;
      return &mem_fyards[idx][cpfn];
    }
    else {
      for (int i = 0; i < byard_candi_num; i++) {
        byard_candi[i] = iceberg_hash(vpn, i + 1) % yard_num;
      }
      int candi_index = (cpfn - fyard_size) / byard_size;
      int byard_offset = (cpfn - fyard_size) % byard_size;
      return &mem_byards[byard_candi[candi_index]][byard_offset];
    }

    // should never reach here
    std::exit(EXIT_FAILURE);
  }
};
