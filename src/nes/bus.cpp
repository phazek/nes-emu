#include "bus.h"

#include "utils.h"

namespace nes {

uint8_t Bus::Read(uint16_t addr) {
	if (IsInRange(0x0000, 0x17FF, addr)) { // internal memory
		return memory_[addr % 0x0800];
	}
	if (IsInRange(0x2000, 0x3FFF, addr)) { // PPU registers

	}
	if (IsInRange(0x4000, 0x4017, addr)) { // APU and I/O registers

	}
	if (IsInRange(0x4018, 0x401F, addr)) { // Test mode

	}
	if (IsInRange(0x4020, 0xFFFF, addr)) { // Cartridge
		if (cartridge_) {
			return cartridge_->ReadPrg(addr);
		}
	}

	return 0;
}

void Bus::Write(uint16_t addr, uint8_t val) {
	if (IsInRange(0x0000, 0x17FF, addr)) { // internal memory
		memory_[addr % 0x0800] = val;
	}
	if (IsInRange(0x2000, 0x3FFF, addr)) { // PPU registers

	}
	if (IsInRange(0x4000, 0x4017, addr)) { // APU and I/O registers

	}
	if (IsInRange(0x4018, 0x401F, addr)) { // Test mode

	}
	if (IsInRange(0x4020, 0xFFFF, addr)) { // Cartridge

	}
}

void Bus::InsertCartridge(Cartridge* cart) {
	cartridge_ = cart;
}

} // namespace nes
