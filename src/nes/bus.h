#pragma once

#include <cstdint>
#include <array>
#include <span>

#include "cartridge.h"
#include "controller.h"

namespace nes {

class Ppu2C02;

class Bus {
public:
	uint8_t Read(uint16_t addr, bool silent = false);
	std::span<uint8_t> ReadN(uint16_t addr, uint16_t count);
	void Write(uint16_t addr, uint8_t val);

	uint8_t ReadChr(uint16_t addr);
	std::span<uint8_t> ReadChrN(uint16_t addr, uint16_t count);

	void InsertCartridge(Cartridge* cart);
	void AttachPPU(Ppu2C02* ppu);
	void AttachController(Controller* con, bool playerOne);
	void TriggerNMI();
	void TriggerDMA();
	bool CheckNMI();
	bool CheckDMA();
private:
	Cartridge* cartridge_ = nullptr;
	Ppu2C02* ppu_ = nullptr;
	Controller* controller1_ = nullptr;
	Controller* controller2_ = nullptr;
	bool triggerNMI_ = false;
	bool triggerDMA_ = false;

	std::array<uint8_t, 2048> memory_;
};

} // namespace nes
