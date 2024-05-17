#include "nes/ppu.h"

#include "nes/bus.h"
#include "nes/types.h"
#include "nes/utils.h"

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

constexpr std::array<uint16_t, 2> kPatternTableStart = {0x0000, 0x1000};
constexpr std::array<uint16_t, 4> kNameTableStart = {0x2000, 0x2400, 0x2800, 0x2C00};
constexpr uint16_t kPaletteTableStart = 0x3F00;
constexpr uint16_t kNameTableSize = 0x03FF;
constexpr uint16_t kAttributeTableOffset = 0x3C0;
constexpr uint8_t kTileDataSize = 16;

struct OAMEntry {
	uint8_t y;
	uint8_t id;
	uint8_t attr;
	uint8_t x;
};

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
	    return tfm::format("<0x%04X>", addr);
    }
}

} // namespace

Ppu2C02::Ppu2C02(Bus* bus)
: bus_(bus)
{
	bus_->AttachPPU(this);
	memset(vramStorage_.data(), 0, 0x0800);
}

uint8_t Ppu2C02::Read(uint16_t addr, bool silent) {
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

			if (!silent) {
			    status_ &= 0x7F;

			    vramBuffer_ = 0;
			    scrollBuffer_ = {0, 0};
			}

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
			return HandleDataRead(silent);
		}
		default: {
			tfm::printf("ERROR: PPU read from %s\n", AddressToString(addr));
			assert(false);
		}
	}
	return 0;
}

std::span<uint8_t> Ppu2C02::ReadN(uint16_t addr, uint16_t count) {
	switch (addr) {
		case 0x0000: { // Nametable0
			return {vramStorage_.data(), kNameTableSize};
		}
		case 0x1000: { // Nametable1
			return {vramStorage_.data() + kNameTableSize, kNameTableSize};
		}
		case kOAMDATA: {
			return {oamStorage_.data() + oamAddress_, count};
		}
	}

	assert(false);
	return {};
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
			scrollBuffer_[scrollSetIndex_] = val;
			scrollSetIndex_ ^= 1;
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
		case kOAMDMA: {
			uint16_t baseAddr = val << 8;
			auto data = bus_->ReadN(baseAddr, 256);
			memcpy(oamStorage_.data(), data.data(), 256);
			oamAddress_+=256;
			bus_->TriggerDMA();
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

const std::array<Ppu2C02::Palette, 8>& Ppu2C02::GetFramePalette() const {
	return framePalette_;
}

const std::array<RGBA, 8 * 8>& Ppu2C02::GetSpriteZero() const {
	return spriteZeroData_;
}

void Ppu2C02::Tick() {
	uint32_t newDot = (dotIdx_ + 1) % (kScanlineRowCount * kScanlineColCount);

	if (newDot == 0) {
		controlState_.nameTableId=0;
		DrawBackgroundLayers();
		DrawSpriteLayer();
		spriteZeroReported_ = false;

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

	auto col = dotIdx_ % kScanlineColCount;
	auto row = dotIdx_ / kScanlineColCount;
	BufferDot bgDot;
	if (col < kScreenColCount && row < kScreenRowCount) {
		int dstIdx = row * kScreenColCount + col;
		int sCol = col + scrollBuffer_[0];
		int sRow = row + scrollBuffer_[1];
		if (sCol >= kScreenColCount) {
			int srcIdx = sRow * kScreenColCount + (sCol % kScreenColCount);
			bgDot = backgroundBuffers_[1 - controlState_.nameTableId][srcIdx];
		} else {
			int srcIdx = sRow * kScreenColCount + sCol;
			bgDot = backgroundBuffers_[controlState_.nameTableId][srcIdx];
		}
		frameBuffer_[dstIdx] = bgDot.color;

	    auto spriteDot = spriteBuffer_[dstIdx];
	    if (spriteDot.color.a != 0 && spriteDot.isOpaque) {
			if (!spriteDot.isBehind || !bgDot.isOpaque) {
			    frameBuffer_[dstIdx] = spriteDot.color;
			}

			if (bgDot.isOpaque && spriteDot.isSprite0 &&
				!spriteZeroReported_) {
				status_ |= 0x40;
				spriteZeroReported_ = true;
			}
	    }
	}
}

void Ppu2C02::ParseControlMessage(uint8_t val) {
	controlState_.nameTableId = val & 0x03;
	controlState_.addressIncrement = (val & 0x04) ? 32 : 1;
	controlState_.spriteTableAddr = (val & 0x08) ? 0x1000 : 0x0000;
	controlState_.backgroundTableIdx = (val & 0x10) ? 1 : 0;
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

uint8_t Ppu2C02::HandleDataRead(bool silent) {
	if (silent) {
		return vramBuffer_;
	}

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
	if (IsInRange(kNameTableStart[0], kNameTableStart[0] + kNameTableSize, addr)) {
		result = vramStorage_[addr - kNameTableStart[0]];
	}

	// Nametable 1
	if (IsInRange(kNameTableStart[1], kNameTableStart[1] + kNameTableSize, addr)) {
		result = vramStorage_[addr - kNameTableStart[0]];
	}

	// Nametable 2
	if (IsInRange(kNameTableStart[2], kNameTableStart[2] + kNameTableSize, addr)) {
		// TODO
		result = 0;
	}

	// Nametable 3
	if (IsInRange(kNameTableStart[3], kNameTableStart[3] + kNameTableSize, addr)) {
		// TODO
		result = 0;
	}

	// Palette indexes
	if (IsInRange(kPaletteTableStart + 0x0020, 0x3FFF, addr)) {
		addr = kPaletteTableStart + ((addr - kPaletteTableStart) % 0x0020);
	}

	if (IsInRange(kPaletteTableStart, kPaletteTableStart + 0x001F, addr)) {
		switch (addr) {
			case 0x3F10:
				addr = 0x3F00;
				break;
			case 0x3F14:
				addr = 0x3F04;
				break;
			case 0x3F18:
				addr = 0x3F08;
				break;
			case 0x3F1C:
				addr = 0x3F0C;
				break;
		}
		auto idx = addr - kPaletteTableStart;
		auto palIdx = idx / 4;
		auto colorIdx = idx % 4;
		result = framePalette_[palIdx][colorIdx];
	}

	if (IsInRange(kPaletteTableStart, kPaletteTableStart + 0x001F, addr)) {
		vramBuffer_ = result;
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
	if (IsInRange(kNameTableStart[0], kNameTableStart[0] + kNameTableSize, addr)) {
		vramStorage_[addr - kNameTableStart[0]] = val;
	}

	// Nametable 1
	if (IsInRange(kNameTableStart[1], kNameTableStart[1] + kNameTableSize, addr)) {
		vramStorage_[addr - kNameTableStart[0]] = val;
	}

	// Nametable 2
	if (IsInRange(kNameTableStart[2], kNameTableStart[2] + kNameTableSize, addr)) {
		// TODO
	}

	// Nametable 3
	if (IsInRange(kNameTableStart[3], kNameTableStart[3] + kNameTableSize, addr)) {
		// TODO
	}

	// Palette indexes
	if (IsInRange(kPaletteTableStart + 0x0020, 0x3FFF, addr)) {
		addr = kPaletteTableStart + ((addr - kPaletteTableStart) % 0x0020);
	}

	if (IsInRange(kPaletteTableStart, kPaletteTableStart + 0x001F, addr)) {
		switch (addr) {
			case 0x3F10:
				addr = 0x3F00;
				break;
			case 0x3F14:
				addr = 0x3F04;
				break;
			case 0x3F18:
				addr = 0x3F08;
				break;
			case 0x3F1C:
				addr = 0x3F0C;
				break;
		}
		auto idx = addr - kPaletteTableStart;
		auto palIdx = idx / 4;
		auto colorIdx = idx % 4;
		framePalette_[palIdx][colorIdx] = val;
	}

	vramAddress_ += controlState_.addressIncrement;
}

void Ppu2C02::DrawBackgroundLayers() {
	if (!maskState_.showBackground && !maskState_.showBackgroundLeft) {
		return;
	}

	Tile t;
	for (int bufIdx = 0; bufIdx < 2; ++bufIdx) {
		auto& bgBuffer = backgroundBuffers_[bufIdx];
		memset(bgBuffer.data(), 0, bgBuffer.size() * sizeof(BufferDot));

		const uint16_t nameTableBase = kNameTableStart[bufIdx] - kNameTableStart[0];
		const uint16_t attrTableBase = nameTableBase + kAttributeTableOffset;

		for (int row = 0; row < 30; ++row) {
			for (int col = 0; col < 32; ++col) {
				auto idx = nameTableBase + row * 32 + col;
				auto patternIdx = vramStorage_[idx];
				auto patternStartAddr =
				    controlState_.backgroundTableIdx * 0x1000 +
				    patternIdx * kTileDataSize;

				t.FromData(bus_->ReadChrN(patternStartAddr, kTileDataSize));

				auto paletteIdx = GetPaletteIdx(attrTableBase, row, col);
				for (int i = 0; i < 8*8; ++i) {
					auto pxColorIdx = t.data[i];
					// Pal0 contains global bg color
					auto colorIdx = framePalette_[pxColorIdx ? paletteIdx : 0][pxColorIdx];
					bgBuffer[(row * 8 + i / 8) * 256 + col * 8 + i % 8] = {
						kColorPalette[colorIdx], pxColorIdx != 0, false};
				}
			}
		}
	}
}

void Ppu2C02::DrawSpriteLayer() {
	if (!maskState_.showSprites && !maskState_.showSpritesLeft) {
		return;
	}

	Tile t;
	memset(spriteBuffer_.data(), 0, spriteBuffer_.size() * sizeof(BufferDot));
	memset(spriteZeroData_.data(), 0, spriteZeroData_.size() * sizeof(RGBA));
	auto* entries = reinterpret_cast<OAMEntry*>(oamStorage_.data());
	for (int i = 63; i >= 0; --i) {
		auto& entry = entries[i];
		if (entry.y >= 0xEF || entry.x >= 240) {
			continue;
		}

		auto patternStartAddr = controlState_.spriteTableAddr + entry.id * 16;
		t.FromData(bus_->ReadChrN(patternStartAddr, 16));
		auto& palette = framePalette_[4 + (entry.attr & 0x03)];

		for (int pxInd = 0; pxInd < 8*8; ++pxInd) {
			auto px = t.data[pxInd];
			auto colorIdx = palette[px];
			auto c = kColorPalette[colorIdx];
			int x = pxInd % 8;
			int y = pxInd / 8;
			if (entry.attr & 0x40) { // horizontal flip
				x = 7 - x;
			}
			if (entry.attr & 0x80) { // vertical flip
				y = 7 - y;
			}

			if (i == 0) {
				spriteZeroData_[y * 8 + x] = c;
			}

			bool isOpaque = px != 0;
			auto idxY = entry.y + 1 + y;
			auto idxX = entry.x + x;
			if (idxY >= kScreenRowCount || idxX >= kScreenColCount) {
				continue;
			}
			auto idx = idxY * kScreenColCount + idxX;
			if (!isOpaque && spriteBuffer_[idx].color.a != 0) {
				continue;
			}

			spriteBuffer_[idx] = {c, isOpaque, (entry.attr & 0x20) != 0, i == 0};
		}
	}

}

uint8_t Ppu2C02::GetPaletteIdx(uint16_t attrTableBase, uint8_t row, uint8_t col) {
	auto attrIdx = (row / 4) * 8 + (col / 4);
	auto attr = vramStorage_[attrTableBase + attrIdx];
	uint8_t id = 0;
	id |= (row % 4 < 2) ? 0 : 2;
	id |= (col % 4 < 2) ? 0 : 1;

	//   -----------
	//  | r0  | r0  |
	//  | c0  | c1  |
	//  |-----------|
	//  | r1  | r1  |
	//  | c0  | c1  |
	//   -----------

	switch (id) {
		case 0b00: return attr  & 0x03;
		case 0b01: return (attr & 0x0C) >> 2;
		case 0b10: return (attr & 0x30) >> 4;
		case 0b11: return (attr & 0xC0) >> 6;
		default: {
			assert(false);
			return 0;
		}
	}
}

} // namespace nes
