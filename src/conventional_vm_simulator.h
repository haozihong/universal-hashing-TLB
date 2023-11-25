#pragma once

#include "vm_simulator.h"
#include "constants+helper.h"

#include <unordered_map>
#include <list>

class ConventionalVmSimulator : public VmSimulator {

public:
  ConventionalVmSimulator(double mem_size_mb) {
    num_frames = mem_size_mb * 1024 / PAGE_SIZE_KB;

    print_info();
  }

  void access(uint64_t addr, char rw) override {
    time_tick += 1;
    stats.total_mem_access += 1;

    uint64_t vpn = get_page_number(addr);
    vpn_set.insert(vpn);

    auto find_res = page_table.find(vpn);

    if (find_res != page_table.end()) {
      // move this page to the end (most recent used position) of the list.
      lru_queue.splice(lru_queue.end(), lru_queue, find_res->second);
      lru_queue.back().timestamp = time_tick;
    }
    else {
      stats.num_page_fault += 1;

      if (page_table.size() >= num_frames) {
        stats.num_swap_out += 1;

        PageFrame& victim = lru_queue.front();
        stats.total_age_of_swapped_out_pages += time_tick - victim.timestamp;
        page_table.erase(victim.vpn);

        lru_queue.pop_front();
      }

      lru_queue.emplace_back(vpn, time_tick, false);
    }

    page_table[vpn] = --lru_queue.end();
  }

  virtual void print_info(std::ostream& os = std::cout) {
    os << "Simulator: Conventional Simulator\n"
       << "----------------"
       << "\nnum_frames = " << num_frames
       << "\n" << std::endl;
  }

private:
  std::list<PageFrame> lru_queue;
  std::unordered_map<uint64_t, decltype(lru_queue)::iterator> page_table;
  uint64_t num_frames;
  uint64_t time_tick {0};
};
