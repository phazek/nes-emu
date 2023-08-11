#include "bus.h"

#include "ppu.h"
#include "utils.h"
#include "types.h"

namespace nes {

uint8_t Bus::Read(uint16_t addr, bool silent) {
	if (IsInRange(0x0000, 0x1FFF, addr)) { // internal memory
		return memory_[addr % 0x0800];
	}
	if (IsInRange(0x2000, 0x3FFF, addr)) { // PPU registers
		return ppu_->Read(0x2000 + ((addr - 0x2000) % 0x008), silent);
	}
	if (IsInRange(0x4000, 0x4017, addr)) { // APU and I/O registers
		if (addr == 0x4016 && controller1_) {
			return controller1_->Read();
		}
		if (addr == 0x4017 && controller2_) {
			return controller2_->Read();
		}
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

std::span<uint8_t> Bus::ReadN(uint16_t addr, uint16_t count) {
	if (IsInRange(0x0000, 0x1FFF, addr)) {	// internal memory
		return {memory_.data() + (addr % 0x0800), count};
	}

	if (IsInRange(0x2000, 0x3FFF, addr)) { // PPU registers
		return ppu_->ReadN(0x2000 + ((addr - 0x2000) % 0x008), count);
	}

	if (IsInRange(0x4020, 0xFFFF, addr)) {	// Cartridge
		if (cartridge_) {
			return cartridge_->ReadPrgN(addr, count);
		}
	}

	assert(false);
	return {};
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

		if (addr == 0x4016) {
			if (controller1_) {
				controller1_->Write(val);
			}
			if (controller2_) {
				controller2_->Write(val);
			}
		}
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

std::span<uint8_t> Bus::ReadChrN(uint16_t addr, uint16_t count) {
	return cartridge_->ReadChrN(addr, count);
}

void Bus::InsertCartridge(Cartridge* cart) {
	cartridge_ = cart;
}

void Bus::AttachPPU(Ppu2C02* ppu) {
	ppu_ = ppu;
}

void Bus::AttachController(Controller* con, bool playerOne) {
	if (playerOne) {
		controller1_ = con;
	} else {
		controller2_ = con;
	}
}

void Bus::TriggerNMI() {
	triggerNMI_ = true;
}

void Bus::TriggerDMA() {
	triggerDMA_ = true;
}

bool Bus::CheckNMI() {
	auto tmp = triggerNMI_;
	triggerNMI_ = false;
	return tmp;
}

bool Bus::CheckDMA() {
	auto tmp = triggerDMA_;
	triggerDMA_ = false;
	return tmp;
}

} // namespace nes
