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

} // namespace nes
