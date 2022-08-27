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

constexpr uint16_t kScanlineRowCount = 262;
constexpr uint16_t kScanlineColCount = 341;

constexpr std::array<uint16_t, 2> kPatternTableStart = {0x0000, 0x1000};
constexpr std::array<uint16_t, 4> kNameTableStart = {0x2000, 0x2400, 0x2800, 0x2C00};
constexpr uint16_t kPaletteTableStart = 0x3F00;

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
	memset(vramStorage_.data(), 0, 0x0800);
}

uint8_t Ppu2C02::Read(uint16_t addr) {
	//tfm::printf("PPU read %s (0x%04X)\n", AddressToString(addr), addr);
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

			vramBuffer_ = 0;
			scrollBuffer_ = 0;

			return tmp;
		}
		case kOAMADDR: {
			// write-only
			break;
		}
		case kOAMDATA: {
			return oamStorage_[oamAddress_];
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
			return HandleDataRead();
		}
		default: {
			assert(false);
		}
	}
	return 0;
}

void Ppu2C02::Write(uint16_t addr, uint8_t val) {
	//tfm::printf("PPU write %s (0x%04X) -> 0x%02X\n", AddressToString(addr), addr, val);
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
			oamAddress_ = val;
			break;
		}
		case kOAMDATA: {
			oamStorage_[oamAddress_++] = val;
			break;
		}
		case kPPUSCROLL: {
			scrollBuffer_ <<= 8;
			scrollBuffer_ |= val;
			break;
		}
		case kPPUADDR: {
			vramAddress_ <<= 8;
			vramAddress_ |= val;
			break;
		}
		case kPPUDATA: {
			HandleDataWrite(val);
			break;
		}
		default: {
			assert(false);
		}
	}
}

void Ppu2C02::SetFramebuffer(RGBA* buf) {
	frameBuffer_ = buf;
}

void Ppu2C02::Tick() {
	uint32_t newDot = (dotIdx_ + 1) % (kScanlineRowCount * kScanlineColCount);

	if (newDot == 0) {
		oddFrame_ = !oddFrame_;
		if (oddFrame_) { // Skip first dot on odd frame
			++newDot;
		}
	}

	if (newDot == (240 * kScanlineColCount + 1)) { // Set VBLANK
		status_ |= 0x80;
		if (controlState_.generateNMI) {
			bus_->TriggerNMI();
		}
	}

	if (newDot == (260 * kScanlineColCount + 1)) {
		// Clear VBLANK, SPRITE0 hit and sprite overflow flags
		status_ = 0x00;
	}

	dotIdx_ = newDot;
}

void Ppu2C02::ParseControlMessage(uint8_t val) {
	controlState_.baseNameTableAddr = kNameTableStart[val & 0x03];
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

uint8_t Ppu2C02::HandleDataRead() {
	auto addr = vramAddress_;
	uint8_t result = 0;

	// Pattern table 0
	if (IsInRange(kPatternTableStart[0], kPatternTableStart[0] + 0x0FFF, addr)) {
		result = bus_->ReadChr(addr - kPatternTableStart[0]);
	}

	// Pattern table 1
	if (IsInRange(kPatternTableStart[1], kPatternTableStart[1] + 0x0FFF, addr)) {
		result = bus_->ReadChr(addr - kPatternTableStart[1]);
	}

	// Mirror 0x2000-0x2EFF
	if (IsInRange(0x3000, 0x3EFF, addr)) {
		addr -= 0x1000;
	}

	// Nametable 0
	if (IsInRange(kNameTableStart[0], kNameTableStart[0] + 0x03FF, addr)) {
		result = vramStorage_[addr - kNameTableStart[0]];
	}

	// Nametable 1
	if (IsInRange(kNameTableStart[1], kNameTableStart[1] + 0x03FF, addr)) {
		result = vramStorage_[addr - kNameTableStart[1]];
	}

	// Nametable 2
	if (IsInRange(kNameTableStart[2], kNameTableStart[2] + 0x03FF, addr)) {
		// TODO
		result = 0;
	}

	// Nametable 3
	if (IsInRange(kNameTableStart[3], kNameTableStart[3] + 0x03FF, addr)) {
		// TODO
		result = 0;
	}

	// Mirror 0x3F00-0x3F1F
	if (IsInRange(0x3F20, 0x3FFF, addr)) {
		addr = ((addr - 0x3F20) % 0x20) + kPaletteTableStart;
	}
	// Palette indexes
	if (IsInRange(kPaletteTableStart, kPaletteTableStart + 0x0020, addr)) {
		auto idx = addr - kPaletteTableStart;
		auto palIdx = idx / 4;
		auto colorIdx = idx % 4;
		result = framePalette_[palIdx][colorIdx];
	}

	if (IsInRange(kPaletteTableStart, kPaletteTableStart + 0x00FF, addr)) {
		vramBuffer_ = result ;
	}

	vramAddress_ += controlState_.addressIncrement;
	std::swap(vramBuffer_, result);
	return result;
}

void Ppu2C02::HandleDataWrite(uint8_t val) {
	auto addr = vramAddress_;
	// Pattern table 0
	if (IsInRange(kPatternTableStart[0], kPatternTableStart[0] + 0x0FFF, addr)) {
		//bus_->WriteChr(addr - kPatternTableStart[0], val);
	}

	// Pattern table 1
	if (IsInRange(kPatternTableStart[1], kPatternTableStart[1] + 0x0FFF, addr)) {
		//bus_->WriteChr(addr - kPatternTableStart[1], val);
	}

	// Mirror 0x2000-0x2EFF
	if (IsInRange(0x3000, 0x3EFF, addr)) {
		addr -= 0x1000;
	}

	// Nametable 0
	if (IsInRange(kNameTableStart[0], kNameTableStart[0] + 0x03FF, addr)) {
		vramStorage_[addr - kNameTableStart[0]] = val;
	}

	// Nametable 1
	if (IsInRange(kNameTableStart[1], kNameTableStart[1] + 0x03FF, addr)) {
		vramStorage_[addr - kNameTableStart[1]] = val;
	}

	// Nametable 2
	if (IsInRange(kNameTableStart[2], kNameTableStart[2] + 0x03FF, addr)) {
		// TODO
	}

	// Nametable 3
	if (IsInRange(kNameTableStart[3], kNameTableStart[3] + 0x03FF, addr)) {
		// TODO
	}

	// Mirror 0x3F00-0x3F1F
	if (IsInRange(0x3F20, 0x3FFF, addr)) {
		addr = ((addr - 0x3F20) % 0x20) + kPaletteTableStart;
	}
	// Palette indexes
	if (IsInRange(kPaletteTableStart, kPaletteTableStart + 0x0020, addr)) {
		auto idx = addr - kPaletteTableStart;
		auto palIdx = idx / 4;
		auto colorIdx = idx % 4;
		framePalette_[palIdx][colorIdx] = val;
	}

	vramAddress_ += controlState_.addressIncrement;
}

} // namespace nes
