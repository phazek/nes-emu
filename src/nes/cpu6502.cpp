#include "cpu6502.h"

namespace nes {

namespace {

uint16_t Join(uint8_t LL, uint8_t HH) {
	return (uint16_t)HH << 8 | LL;
}

constexpr uint16_t kNMIVectorLo = 0xFFFA;
constexpr uint16_t kNMIVectorHi = 0xFFFB;
constexpr uint16_t kResetVectorLo = 0xFFFC;
constexpr uint16_t kResetVectorHi = 0xFFFD;
constexpr uint16_t kInterruptVectorLo = 0xFFFE;
constexpr uint16_t kInterruptVectorHi = 0xFFFF;
} // namespace

Cpu6502::Cpu6502(Bus* bus): bus_(bus) {
	assert(bus_ != nullptr);
	Reset();
}

void Cpu6502::Reset() {
	cycle_ = 7; // startup sequence
	auto LL = bus_->Read(kResetVectorLo);
	auto HH = bus_->Read(kResetVectorHi);
	pc_ = Join(LL, HH);
	stack_ = 0xFF;
}

void Cpu6502::Tick() {
}

}

bool Cpu6502::IsSet(Flag f) const {
	return stack_ & f;
}

void Cpu6502::SetFlag(Flag f, bool active) {
	if (active) {
		stack_ |= f;
	} else {
		stack_ &= ~f;
	}
}

} // namespace nes
