#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <span>

#include "palette.h"

namespace nes {

class Bus;

class Ppu2C02 {
public:
	using Palette = std::array<uint8_t, 4>;

	Ppu2C02(Bus* bus);

	uint8_t Read(uint16_t addr, bool silent);
	std::span<uint8_t> ReadN(uint16_t addr, uint16_t count);
	void Write(uint16_t addr, uint8_t val);

	void SetFramebuffer(RGBA* buf);

	const std::array<Palette, 8>& GetFramePalette() const;
	const std::array<RGBA, 8*8>& GetSpriteZero() const;

	void Tick();
private:
	struct BufferDot {
		RGBA color;
		bool isOpaque = true;
		bool isSprite0 = false;
	};
	using BackingBuffer = std::array<BufferDot, kScreenColCount * kScreenRowCount>;

	Bus* bus_ = nullptr;
	RGBA* frameBuffer_ = nullptr;

	std::array<BackingBuffer, 4> backgroundBuffers_;
	BackingBuffer spriteBuffer_;
	bool spriteZeroReported_ = false;

	uint8_t oamAddress_ = 0;
	std::array<uint8_t, 0xFF> oamStorage_;

	uint16_t vramAddress_ = 0;
	uint8_t vramBuffer_ = 0;
	std::array<uint8_t, 0x0800> vramStorage_;

	std::array<Palette, 8> framePalette_;

	std::array<uint8_t, 16> rawTileBuffer_;

	uint8_t scrollSetIndex_ = 0;
	std::array<uint8_t, 2> scrollBuffer_{0, 0}; // X, Y
	uint8_t status_ = 0;

	uint32_t dotIdx_ = 0;
	bool oddFrame_ = false;

	struct ControlState {
		uint16_t nameTableId = 0;
		uint16_t spriteTableAddr;
		uint16_t backgroundTableIdx = 0;
		uint16_t addressIncrement = 0;
		enum SpriteSize {
			k8x8,
			k8x16,
		} spriteSize;
		enum Select {
			kInput,
			kOutput,
		} select;
		bool generateNMI = false;
	} controlState_;

	struct MaskState {
		bool grayscale : 1 = false;
		bool showBackgroundLeft : 1 = false;
		bool showSpritesLeft : 1 = false;
		bool showBackground : 1 = false;
		bool showSprites : 1 = false;
		bool emphasizeRed : 1 = false;
		bool emphasizeGreen : 1 = false;
		bool emphasizeBlue : 1 = false;
	} maskState_;

	std::array<RGBA, 8 * 8> spriteZeroData_;

	void ParseControlMessage(uint8_t val);
	void ParseMaskMessage(uint8_t val);
	uint8_t HandleDataRead(bool silent);
	void HandleDataWrite(uint8_t val);

	void FetchPattern(uint8_t nameTableIdx, uint8_t row, uint8_t col);
	uint8_t GetPaletteIdx(uint16_t attrTableBase, uint8_t row, uint8_t col);
	void DrawBackgroundLayers();
	void DrawSpriteLayer();
};

} // namespace nes
