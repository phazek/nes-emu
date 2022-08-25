#pragma once

#include <cstdint>

namespace nes {

class Bus;

class Ppu2C02 {
public:
	Ppu2C02(Bus* bus);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);

	void Tick();
private:
	Bus* bus_ = nullptr;
	uint8_t status_ = 0;

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

	void ParseControlMessage(uint8_t val);
};

} // namespace nes
