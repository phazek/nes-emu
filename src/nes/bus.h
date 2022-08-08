#pragma once

#include <cstdint>
#include <array>

#include "cartridge.h"

namespace nes {

class Bus {
public:
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);

	void InsertCartridge(Cartridge* cart);
private:
	Cartridge* cartridge_ = nullptr;

	std::array<uint8_t, 2048> memory_;
};

} // namespace nes
