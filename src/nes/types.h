#pragma once

#include <cstdint>

namespace nes {

struct RGBA {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

// Constants
constexpr uint16_t kOAMDMA = 0x4014;
constexpr uint16_t kScanlineRowCount = 262;
constexpr uint16_t kScanlineColCount = 341;
constexpr uint16_t kScreenRowCount = 240;
constexpr uint16_t kScreenColCount = 256;


} // namespace nes
