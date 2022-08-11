#pragma once

#include <cstdint>

namespace nes {

constexpr uint16_t kOAMDMA = 0x4014;

bool IsInRange(uint16_t beg, uint16_t end, uint16_t val);

} // namespace nes
