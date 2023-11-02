#pragma once

#include "vm_simulator.h"
#include "constants+helper.h"

#include <unordered_map>
#include <list>

class ConventionalVmSimulator : public VmSimulator {

public:
  ConventionalVmSimulator(double mem_size_mb) {
    num_frames = mem_size_mb * 1024 / PAGE_SIZE_KB;
  }

  void access(uint64_t addr, char rw) override {
    uint64_t vpn = get_page_number(addr);
    vpn_set.insert(vpn);

    auto find_res = page_table.find(vpn);

    if (find_res != page_table.end()) {
      lru_queue.erase(find_res->second);
    }
    else {
      stats.num_page_fault += 1;

      if (page_table.size() >= num_frames) {
        stats.num_swap_out += 1;

        page_table.erase(lru_queue.front());
        lru_queue.pop_front();
      }
    }

    lru_queue.push_back(vpn);
    page_table[vpn] = --lru_queue.end();

    stats.total_mem_access += 1;
  }

private:
  std::list<uint64_t> lru_queue;
  std::unordered_map<uint64_t, std::list<uint64_t>::iterator> page_table;
  uint64_t num_frames;
};
