#include "cpu6502.h"
#include "instructions.h"

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
    if (cycleLeft_) {
		cycleLeft_--;
		return;
    }

    // compensate for multi-cycle instructions
    auto opCode = bus_->Read(pc_);
    auto op = kOpDecoder.at(opCode);
    uint8_t operand;
    bool boundaryCrossed = FetchOperand(op.addrMode, operand);

    switch (op.instr) {
		case Instruction::kADC: {
			uint16_t tmp = acc_ + (int8_t)operand + (IsSet(Flag::C) ? 1 : 0);
			acc_ = tmp & 0xFF;

			SetFlag(Flag::N, acc_ & 0x80);
			SetFlag(Flag::Z, acc_ == 0);
			SetFlag(Flag::C, tmp & 0x100);
			SetFlag(Flag::V, (!((acc_ ^ operand) & 0x80) && ((acc_ ^ tmp) & 0x80)));

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 4 + (boundaryCrossed ? 1 : 0);
					break;
				case AddressMode::kABY:
					cycleLeft_ += 4 + (boundaryCrossed ? 1 : 0);
					break;
				case AddressMode::kIMM:
					cycleLeft_ += 2;
					break;
				case AddressMode::kINX:
					cycleLeft_ += 6;
					break;
				case AddressMode::kINY:
					cycleLeft_ += 5 + (boundaryCrossed ? 1 : 0);
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 4;
					break;
				default:;
			}
			break;
		}
    }
}

bool Cpu6502::FetchOperand(AddressMode m, uint8_t& operand) {
	switch (m) {
		case AddressMode::kACC: {
			operand = 0;
			return false;
		}
		case AddressMode::kABS: {
			auto LL = bus_->Read(pc_ + 1);
			auto HH = bus_->Read(pc_ + 2);
			operand = bus_->Read(Join(LL, HH));
			return false;
		}
		case AddressMode::kABX: {
			auto LL = bus_->Read(pc_ + 1);
			auto HH = bus_->Read(pc_ + 2);
			operand = bus_->Read(Join(LL, HH) + x_);
			return (uint8_t)(LL + x_) < x_;
		}
		case AddressMode::kABY: {
			auto LL = bus_->Read(pc_ + 1);
			auto HH = bus_->Read(pc_ + 2);
			operand = bus_->Read(Join(LL, HH) + y_);
			return (uint8_t)(LL + y_) < y_;
		}
		case AddressMode::kIMM: {
			operand = bus_->Read(pc_ + 1);
			return false;
		}
		case AddressMode::kIMP: {
			operand = 0;
			return false;
		}
		case AddressMode::kIND: {
			auto LL = bus_->Read(pc_ + 1);
			auto HH = bus_->Read(pc_ + 2);
			auto addr = Join(LL, HH);
			LL = bus_->Read(addr);
			HH = bus_->Read(addr + 1);
			operand = bus_->Read(Join(LL, HH));
			return false;
		}
		case AddressMode::kINX: {
			uint16_t addr = (bus_->Read(pc_ + 1) + x_) & 0xFF;
			auto LL = bus_->Read(addr);
			auto HH = bus_->Read(addr + 1);
			operand = bus_->Read(Join(LL, HH));
			return false;
		}
		case AddressMode::kINY: {
			uint16_t addr = bus_->Read(pc_ + 1) & 0xFF;
			auto LL = bus_->Read(addr);
			auto HH = bus_->Read(addr + 1);
			operand = bus_->Read(Join(LL, HH) + y_);
			return (uint8_t)(LL + y_) < y_;
		}
		case AddressMode::kREL: {
			operand = bus_->Read(pc_ + 1);
			return ((pc_ + (int8_t)operand) & 0xFF00) != (pc_ & 0xFF00);
		}
		case AddressMode::kZP: {
			uint16_t addr = bus_->Read(pc_ + 1);
			operand = bus_->Read(addr);
			return false;
		}
		case AddressMode::kZPX: {
			uint16_t addr = (bus_->Read(pc_ + 1) + x_) & 0xFF;
			operand = bus_->Read(addr);
			return false;
		}
		case AddressMode::kZPY: {
			uint16_t addr = (bus_->Read(pc_ + 1) + y_) & 0xFF;
			operand = bus_->Read(addr);
			return false;
		}
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
