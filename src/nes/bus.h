#pragma once

#include <cstdint>
#include <array>

#include "cartridge.h"

namespace nes {

class Ppu2C02;

class Bus {
public:
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);

	uint8_t ReadChr(uint16_t addr);

	void InsertCartridge(Cartridge* cart);
	void AttachPPU(Ppu2C02* ppu);
private:
	Cartridge* cartridge_ = nullptr;
	Ppu2C02* ppu_ = nullptr;

	std::array<uint8_t, 2048> memory_;
};

} // namespace nes
