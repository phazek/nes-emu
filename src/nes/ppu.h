#pragma once

#include <cstdint>

namespace nes {

class Ppu2C02 {
public:
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);

private:
};

} // namespace nes
