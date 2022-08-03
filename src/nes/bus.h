#pragma once

#include <cstdint>

namespace nes {

class Bus {
public:
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);
};

} // namespace nes
