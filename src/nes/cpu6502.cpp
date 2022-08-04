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
    auto operand = FetchOperand(op.addrMode);

    switch (op.instr) {
		case Instruction::kADC: {
			const uint16_t sum = acc_ + operand.val + (IsSet(Flag::C) ? 1 : 0);
			const uint8_t result = sum & 0xFF;
			SetFlag(Flag::C, sum >> 8);
			SetFlag(Flag::V, !!((acc_ ^ result) & (operand.val^ result) & 0x80));
			SetFlag(Flag::N, !!(result & 0x80));
			SetFlag(Flag::Z, !result);
			acc_ = result;

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 4 + (operand.boundaryCrossed ? 1 : 0);
					break;
				case AddressMode::kABY:
					cycleLeft_ += 4 + (operand.boundaryCrossed ? 1 : 0);
					break;
				case AddressMode::kIMM:
					cycleLeft_ += 2;
					break;
				case AddressMode::kINX:
					cycleLeft_ += 6;
					break;
				case AddressMode::kINY:
					cycleLeft_ += 5 + (operand.boundaryCrossed ? 1 : 0);
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
		case Instruction::kAND: {
			acc_ = acc_ & operand.val;

			SetFlag(Flag::N, acc_ & 0x80);
			SetFlag(Flag::Z, acc_ == 0);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 4 + (operand.boundaryCrossed ? 1 : 0);
					break;
				case AddressMode::kABY:
					cycleLeft_ += 4 + (operand.boundaryCrossed ? 1 : 0);
					break;
				case AddressMode::kIMM:
					cycleLeft_ += 2;
					break;
				case AddressMode::kINX:
					cycleLeft_ += 6;
					break;
				case AddressMode::kINY:
					cycleLeft_ += 5 + (operand.boundaryCrossed ? 1 : 0);
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

Cpu6502::Operand Cpu6502::FetchOperand(AddressMode m) {
	Cpu6502::Operand res;
	uint16_t addr;
	switch (m) {
		case AddressMode::kACC: {
			res.val = acc_;
			res.addr = 0xFFFF;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kABS: {
			auto LL = bus_->Read(pc_ + 1);
			auto HH = bus_->Read(pc_ + 2);
			auto addr = Join(LL, HH);

			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kABX: {
			auto LL = bus_->Read(pc_ + 1);
			auto HH = bus_->Read(pc_ + 2);
			auto addr = Join(LL, HH) + x_;

			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = (uint8_t)(LL + x_) < x_;
			break;
		}
		case AddressMode::kABY: {
			auto LL = bus_->Read(pc_ + 1);
			auto HH = bus_->Read(pc_ + 2);
			auto addr = Join(LL, HH) + y_;

			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = (uint8_t)(LL + y_) < y_;
			break;
		}
		case AddressMode::kIMM: {
			res.val = bus_->Read(pc_ + 1);
			res.addr = pc_ + 1;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kIMP: {
			res.val = 0;
			res.addr = 0xFFFF;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kIND: {
			auto LL = bus_->Read(pc_ + 1);
			auto HH = bus_->Read(pc_ + 2);
			auto addr = Join(LL, HH);
			LL = bus_->Read(addr);
			HH = bus_->Read(addr + 1);
			addr = Join(LL, HH);

			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kINX: {
			addr = (bus_->Read(pc_ + 1) + x_) & 0xFF;
			auto LL = bus_->Read(addr);
			auto HH = bus_->Read(addr + 1);
			auto addr = Join(LL, HH);
			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kINY: {
			addr = bus_->Read(pc_ + 1) & 0xFF;
			auto LL = bus_->Read(addr);
			auto HH = bus_->Read(addr + 1);
			auto addr = Join(LL, HH) + y_;

			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = (uint8_t)(LL + y_) < y_;
			break;
		}
		case AddressMode::kREL: {
			res.val = bus_->Read(pc_ + 1);
			res.addr = pc_ + 1;
			res.boundaryCrossed = ((pc_ + (int8_t)res.val) & 0xFF00) != (pc_ & 0xFF00);
			break;
		}
		case AddressMode::kZP: {
			addr = bus_->Read(pc_ + 1);
			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kZPX: {
			addr = (bus_->Read(pc_ + 1) + x_) & 0xFF;
			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kZPY: {
			addr = (bus_->Read(pc_ + 1) + y_) & 0xFF;
			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = false;
			break;
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
