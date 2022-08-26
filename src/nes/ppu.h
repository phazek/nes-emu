#pragma once

#include <cstdint>
#include <array>
#include <vector>

#include "palette.h"

namespace nes {

class Bus;

class Ppu2C02 {
public:
	Ppu2C02(Bus* bus);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);

	void SetFramebuffer(RGBA* buf);

	void Tick();
private:
	Bus* bus_ = nullptr;
	RGBA* frameBuffer_ = nullptr;

	using Palette = std::array<RGBA, 4>;
	struct FramePalette {
		std::array<Palette, 4> backgroundPalettes;
		std::array<Palette, 4> spritePalettes;
	} framePalette_;

	std::array<uint8_t, 0x20 * 0x1E> nameTable_;
	std::array<uint8_t, 0x08 * 0x08> attributeTable_;

	uint8_t oamAddress_ = 0;
	std::array<uint8_t, 0xFF> oamStorage_;

	uint8_t status_ = 0;

	uint32_t dotIdx_ = 0;
	bool oddFrame_ = false;

	struct ControlState {
		uint16_t baseNameTableAddr;
		uint16_t spriteTableAddr;
		uint16_t backgroundTableAddr;
		uint16_t addressIncrement;
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

	void ParseControlMessage(uint8_t val);
	void ParseMaskMessage(uint8_t val);
};

} // namespace nes
