#pragma once

#include "bus.h"

namespace nes {

enum class AddressMode;

class Cpu6502 {
public:
	Cpu6502(Bus* bus);

	void Reset();
	void Tick();

private:
	enum Flag {
		N = 1 << 7, // negative
		V = 1 << 6, // overflow
		X = 1 << 5, // ignored
		B = 1 << 4, // break
		D = 1 << 3, // decimal
		I = 1 << 2, // interrupt
		Z = 1 << 1, // zero
		C = 1		// carry
	};

	uint16_t pc_;
	uint8_t acc_;
	uint8_t x_;
	uint8_t y_;
	uint8_t stack_;
	uint8_t status_;

	uint64_t cycle_ = 0;
	uint8_t cycleLeft_ = 0;

	Bus* bus_ = nullptr;

	bool FetchOperand(AddressMode m, uint8_t& operand);
	bool IsSet(Flag f) const;
	void SetFlag(Flag f, bool active);
};

} // namespace nes
