#pragma once

#include <stdint.h>

constexpr int PAGE_SIZE_KB = 4;
constexpr int PAGE_SIZE_BITS = 12;


uint64_t get_page_number(uint64_t vpn) {
  return vpn >> PAGE_SIZE_BITS;
}