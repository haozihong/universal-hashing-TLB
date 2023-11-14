#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <utility>

#include "include/xxhash64.h"
#include "constants+helper.h"
#include "page_frame.h"
#include "vm_simulator.h"

// using namespace std;

class IcebergSimulator : public VmSimulator {
public:
  IcebergSimulator(double mem_size_mb, int frontyard_size, int backyard_size)
      : fyard_size(frontyard_size), byard_size(backyard_size),
        yard_num(mem_size_mb * 1024 / PAGE_SIZE_KB / (frontyard_size + backyard_size)),
        memory_fyard(yard_num), memory_byard(yard_num), byard_avail(yard_num, byard_size) {
    for (auto& yard : memory_fyard) {
      yard.resize(fyard_size);
    }
    for (auto& yard : memory_byard) {
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
      auto [byard_frame, byard_cpfn] = pick_from_backyards(vpn);
      if (byard_frame->free) {
        victim_frame = byard_frame;
        victim_cpfn = byard_cpfn;
        break;
      }

      // if no free frame
      stats.num_swap_out += 1;

      if (fyard_frame->timestamp < byard_frame->timestamp) {
        victim_frame = fyard_frame;
        victim_cpfn = fyard_cpfn;
      }
      else {
        victim_frame = byard_frame;
        victim_cpfn = byard_cpfn;
      }
    } while(0);

    page_table.erase(victim_frame->vpn);

    page_table[vpn] = victim_cpfn;
    victim_frame->vpn = vpn;
    victim_frame->free = false;
    victim_frame->timestamp = time_tick;
  }

private:
  uint64_t time_tick {0};

  double mem_size_mb;

  int fyard_size;
  int byard_size;
  int yard_num;
  int byard_candi_num = 6;

    // map VPN to CPFN
  std::unordered_map<uint64_t, uint32_t> page_table;
  std::vector<std::vector<PageFrame>> memory_fyard;
  std::vector<std::vector<PageFrame>> memory_byard;
  std::vector<int> byard_avail;

  uint64_t iceberg_hash(uint64_t vpn, int hash_index) {
    return XXHash64::hash(&vpn, sizeof(vpn), hash_index); 
  }

  // Returns the first free page in the frontyard.
  // Otherwise return the frame with oldest timestamp.
  std::pair<PageFrame*, uint32_t> pick_from_frontyard(uint64_t vpn) {
    int fyard_id = iceberg_hash(vpn, 0) % fyard_size;
    for (int j = 0; j < fyard_size; j++) {
      if (memory_fyard[fyard_id][j].free) {
        return {&memory_fyard[fyard_id][j], j};
      }
    }
    auto it = min_element(memory_fyard[fyard_id].begin(), memory_fyard[fyard_id].end(),
                          [](auto& f1, auto& f2) { return f1.timestamp < f2.timestamp; });
    return {&*it, it - memory_fyard[fyard_id].begin()};
  }
  
  // Returns the first free page in the most vacant backyard.
  // Otherwise return the frame with oldest timestamp.
  std::pair<PageFrame*, uint32_t> pick_from_backyards(uint64_t vpn) {
    std::vector<uint64_t> byard_candi;
    int max_avail_bucket_index = -1;
    int max_avail_bucket_size = 0;
    for (int j = 0; j < byard_candi_num; j++) {
      byard_candi.push_back(iceberg_hash(vpn, j + 1) % byard_size);
      if (max_avail_bucket_size < byard_avail[j]) {
        max_avail_bucket_index = j;
        max_avail_bucket_size = byard_avail[j];
      }
    }
    if (max_avail_bucket_size > 0) {
      // find free frame
      for (int j = 0; j < byard_size; j++) {
        if (memory_byard[max_avail_bucket_index][j].free) {
          return {&memory_byard[max_avail_bucket_index][j], j};
        }
      }

      return {nullptr, 0};
    }
    else {
      int oldest_candi_id = 0, oldest_offset = 0;
      for (size_t i = 0; i < byard_candi.size(); i++) {
        for (int j = 0; j < byard_size; j++) {
          if (memory_byard[byard_candi[i]][j].timestamp <
              memory_byard[byard_candi[oldest_candi_id]][oldest_offset].timestamp) {
            oldest_candi_id = (int)i;
            oldest_offset = j;
          }
        }
      }
      return {&memory_byard[byard_candi[oldest_candi_id]][oldest_offset],
              byard_size + oldest_candi_id * byard_size + oldest_offset};
    }
  }

  PageFrame *find_frame(uint64_t vpn, uint32_t cpfn) {
    // if frame in front yard
    if ((int)cpfn < fyard_size) {
      int idx = iceberg_hash(vpn, 0) % fyard_size;
      return &memory_fyard[idx][cpfn];
    }
    else {
      std::vector<uint64_t> byard_candi;
      for (int j = 0; j < byard_candi_num; j++) {
        byard_candi.push_back(iceberg_hash(vpn, j + 1) % byard_size);
      }
      int byard_index = (cpfn - fyard_size) / byard_size;
      int byard_offset = (cpfn - fyard_size) % byard_size;
      return &memory_byard[byard_candi[byard_index]][byard_offset];
    }

    return nullptr;  // placeholder
  }


};
