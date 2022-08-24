#pragma once

#include <cstdint>

namespace nes {

class Bus;

class Ppu2C02 {
public:
	Ppu2C02(Bus* bus);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);

	void Tick();
private:
	Bus* bus_ = nullptr;
};

} // namespace nes
