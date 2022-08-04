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

	struct Operand {
		uint8_t val = 0;
		uint16_t addr = 0;
		bool boundaryCrossed = false;
	};

	uint16_t pc_;
	uint8_t acc_;
	uint8_t x_;
	uint8_t y_;
	uint8_t stackPtr_;
	uint8_t status_;

	uint64_t cycle_ = 0;
	uint8_t cycleLeft_ = 0;

	Bus* bus_ = nullptr;

	Operand FetchOperand(AddressMode m);
	bool IsSet(Flag f) const;
	void SetFlag(Flag f, bool active);
	void PushStack(uint8_t val);
	uint8_t PopStack();
};

} // namespace nes
