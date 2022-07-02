//
// Created by JerryGuo on 2022/6/28.
//

#ifndef RISC_V_TOMASULO__MEMORY_HPP_
#define RISC_V_TOMASULO__MEMORY_HPP_

#include <cstdint>
#include <cstring>
#include <ostream>
#include "mydef.hpp"

namespace RISC_V {
template<size_t SIZE = RAM_LIMIT>
class RAM {
  uint8_t Storage[SIZE] = {};
 public:
  RAM() = default;

  uint8_t &operator[](uint32_t pos) {
    return Storage[pos];
  };

  const uint8_t &operator[](uint32_t pos) const {
    return Storage[pos];
  }

  void Write(uint32_t pos, uint8_t Byte) {
    Storage[pos] = Byte;
  }

  uint32_t Read8(uint32_t pos) {
    return Storage[pos];
  }

  uint32_t Read16(uint32_t pos) {
    return ((uint32_t) Storage[pos + 1] << 8) + (uint32_t) Storage[pos];
  }

  uint32_t Read32(uint32_t pos) {
    return ((uint32_t) Storage[pos + 3] << 24) + ((uint32_t) Storage[pos + 2] << 16)
        + ((uint32_t) Storage[pos + 1] << 8) + (uint32_t) Storage[pos];
  }

};
}

#endif //RISC_V_TOMASULO__MEMORY_HPP_
