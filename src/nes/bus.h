#pragma once

#include <cstdint>
#include <array>

namespace nes {

class Bus {
public:
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);
private:
	std::array<uint8_t, 2048> memory_;
};

} // namespace nes
