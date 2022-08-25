#include "ppu.h"

#include "bus.h"
#include "utils.h"

#include <tfm/tinyformat.h>

namespace nes {

namespace {

constexpr uint16_t kPPUCTRL = 0x2000;   // WRITE
constexpr uint16_t kPPUMASK = 0x2001;   // WRITE
constexpr uint16_t kPPUSTATUS = 0x2002; // READ
constexpr uint16_t kOAMADDR = 0x2003;   // WRITE
constexpr uint16_t kOAMDATA = 0x2004;   // READ/WRITE
constexpr uint16_t kPPUSCROLL = 0x2005; // WRITE x2
constexpr uint16_t kPPUADDR = 0x2006;   // WRITE x2
constexpr uint16_t kPPUDATA = 0x2007;   // READ/WRITE

std::string AddressToString(uint16_t addr) {
    switch (addr) {
	case kPPUCTRL:
	    return "PPUCTRL(W)";
	case kPPUMASK:
	    return "PPUMASK(W)";
	case kPPUSTATUS:
	    return "PPUSTATUS(R)";
	case kOAMADDR:
	    return "OAMADDR(W)";
	case kOAMDATA:
	    return "OAMDATA(R/W)";
	case kPPUSCROLL:
	    return "PPUSCROLL(Wx2)";
	case kPPUADDR:
	    return "PPUADDR(Wx2)";
	case kPPUDATA:
	    return "PPUDATA(R/W)";
	case kOAMDMA:
	    return "OAMDMA(W)";
	default:
	    return "<UNKNOWN>";
    }
}

} // namespace

Ppu2C02::Ppu2C02(Bus* bus)
: bus_(bus)
{
	bus_->AttachPPU(this);
}

uint8_t Ppu2C02::Read(uint16_t addr) {
	tfm::printf("PPU read %s (0x%04X)\n", AddressToString(addr), addr);
	switch (addr) {
		case kPPUCTRL: {
			// write-only
			break;
		}
		case kPPUMASK: {
			// write-only
			break;
		}
		case kPPUSTATUS: {
			auto tmp = status_;
			status_ &= 0x7F;
			return tmp;
		}
		case kOAMADDR: {
			// write-only
			break;
		}
		case kOAMDATA: {

		}
		case kPPUSCROLL: {
			// write-only
			break;
		}
		case kPPUADDR: {
			// write-only
			break;
		}
		case kPPUDATA: {

		}
		default: {
			assert(false);
		}
	}
	return 0;
}

void Ppu2C02::Write(uint16_t addr, uint8_t val) {
	tfm::printf("PPU write %s (0x%04X) -> 0x%02X\n", AddressToString(addr), addr, val);
	switch (addr) {
		case kPPUCTRL: {
			ParseControlMessage(val);
			break;
		}
		case kPPUMASK: {
			ParseMaskMessage(val);
			break;
		}
		case kPPUSTATUS: {
			// read-only
			break;
		}
		case kOAMADDR: {
			break;
		}
		case kOAMDATA: {
			break;
		}
		case kPPUSCROLL: {
			break;
		}
		case kPPUADDR: {
			break;
		}
		case kPPUDATA: {

		}
		default: {
			assert(false);
		}
	}
}

void Ppu2C02::Tick() {
	// TODO
}

void Ppu2C02::ParseControlMessage(uint8_t val) {
	switch (val & 0x03) {
		case 0x00: controlState_.baseNameTableAddr = 0x2000; break;
		case 0x01: controlState_.baseNameTableAddr = 0x2400; break;
		case 0x02: controlState_.baseNameTableAddr = 0x2800; break;
		case 0x03: controlState_.baseNameTableAddr = 0x2C00; break;
	}

	controlState_.addressIncrement = (val & 0x04) ? 32 : 1;
	controlState_.spriteTableAddr = (val & 0x08) ? 0x1000 : 0x0000;
	controlState_.backgroundTableAddr = (val & 0x10) ? 0x1000 : 0x0000;
	controlState_.spriteSize =
	    (val & 0x20) ? ControlState::SpriteSize::k8x16 : ControlState::SpriteSize::k8x8;
	controlState_.select =
	    (val & 0x40) ? ControlState::Select::kOutput : ControlState::Select::kInput;
	controlState_.generateNMI = !!(val & 0x80);
}

void Ppu2C02::ParseMaskMessage(uint8_t val) {
	maskState_.grayscale = !!(val & 0x01);
	maskState_.showBackgroundLeft = !!(val & 0x02);
	maskState_.showSpritesLeft = !!(val & 0x04);
	maskState_.showBackground = !!(val & 0x08);
	maskState_.showSprites = !!(val & 0x10);
	// TODO: PAL/NTSC
	maskState_.emphasizeRed = !!(val & 0x20);
	maskState_.emphasizeGreen = !!(val & 0x40);
	maskState_.emphasizeBlue = !!(val & 0x80);
}

} // namespace nes
