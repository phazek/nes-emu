#include "bus.h"

#include "ppu.h"
#include "utils.h"
#include "types.h"

namespace nes {

uint8_t Bus::Read(uint16_t addr) {
	if (IsInRange(0x0000, 0x17FF, addr)) { // internal memory
		return memory_[addr % 0x0800];
	}
	if (IsInRange(0x2000, 0x3FFF, addr)) { // PPU registers
		return ppu_->Read(0x2000 + ((addr - 0x2000) % 0x008));
	}
	if (IsInRange(0x4000, 0x4017, addr)) { // APU and I/O registers
		if (addr == kOAMDMA) {
			return ppu_->Read(addr);
		}
		// TODO
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
		ppu_->Write(0x2000 + ((addr - 0x2000) % 0x008), val);
	}
	if (IsInRange(0x4000, 0x4017, addr)) { // APU and I/O registers
		if (addr == kOAMDMA) {
			ppu_->Write(addr, val);
		}
		// TODO
	}
	if (IsInRange(0x4018, 0x401F, addr)) { // Test mode

	}
	if (IsInRange(0x4020, 0xFFFF, addr)) { // Cartridge
		// TODO
	}
}

uint8_t Bus::ReadChr(uint16_t addr) {
	return cartridge_->ReadChar(addr);
}

void Bus::InsertCartridge(Cartridge* cart) {
	cartridge_ = cart;
}

void Bus::AttachPPU(Ppu2C02* ppu) {
	ppu_ = ppu;
}

void Bus::TriggerNMI() {
	triggerNMI_ = true;
}

bool Bus::CheckNMI() {
	auto tmp = triggerNMI_;
	triggerNMI_ = false;
	return tmp;
}

} // namespace nes
