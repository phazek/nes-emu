#pragma once

#include "olc/olcPixelGameEngine.h"

#include <array>
#include <cstdint>
#include <span>

namespace nes {

struct RGBA {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	olc::Pixel ToPixel() const {
		return {r, g, b};
	}
};

// Constants
constexpr uint16_t kOAMDMA = 0x4014;
constexpr uint16_t kScanlineRowCount = 262;
constexpr uint16_t kScanlineColCount = 341;
constexpr uint16_t kScreenRowCount = 240;
constexpr uint16_t kScreenColCount = 256;

struct Tile {
	std::array<uint8_t, 8 * 8> data;

	void FromData(const std::span<uint8_t>& src) {
		for (int row = 0; row < 8; ++row) {
			for (int col = 0; col < 8; ++col) {
				bool ll = !!(src[row] & (1 << (7 - col)));
				bool hh = !!(src[8 + row] & (1 << (7 - col)));
				data[row * 8 + col] = (hh ? 2 : 0) | (ll ? 1 : 0);
			}
		}
	}
};


} // namespace nes
