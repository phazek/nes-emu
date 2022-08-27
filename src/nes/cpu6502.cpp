#include "cpu6502.h"
#include "instructions.h"

#include "tfm/tinyformat.h"

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
	stackPtr_ = 0xFF;
}

void Cpu6502::Tick() {
	UpdateState();

	++cycle_;
    if (cycleLeft_) {
		cycleLeft_--;
		return;
    }

	// Check NMI
	if (bus_->CheckNMI()) {
		PushStack(pc_ >> 8); // HH
		PushStack(pc_ & 0xFF); // LL
		PushStack(status_);

		auto LL = bus_->Read(kNMIVectorLo);
		auto HH = bus_->Read(kNMIVectorHi);
		pc_ = Join(LL, HH);
	}

    // compensate for multi-cycle instructions
    auto opCode = bus_->Read(pc_);
    auto op = kOpDecoder.at(opCode);
    auto operand = FetchOperand(op.addrMode);
	pc_ += OpSizeByMode(op.addrMode);

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
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
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
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kASL: {
			uint8_t res = operand.val << 1;

			SetFlag(Flag::C, operand.val & 0x80);
			SetFlag(Flag::N, res & 0x80);
			SetFlag(Flag::Z, res == 0);

			if (operand.addr == 0xFFFF) {
				acc_ = res;
			} else {
				bus_->Write(operand.addr, res);
			}

			switch (op.addrMode) {
				case AddressMode::kACC:
					cycleLeft_ += 2;
					break;
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
		    break;
		}
		case Instruction::kBCC: {
			auto oldPc = pc_;
		    if (!IsSet(Flag::C)) {
				pc_ += (int8_t)operand.val;
				cycleLeft_ += (oldPc & 0xFF00) == (pc_ & 0xFF00) ? 1 : 2;
		    }
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kBCS: {
			auto oldPc = pc_;
		    if (IsSet(Flag::C)) {
				pc_ += (int8_t)operand.val;
				cycleLeft_ += (oldPc & 0xFF00) == (pc_ & 0xFF00) ? 1 : 2;
		    }
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kBEQ: {
			auto oldPc = pc_;
		    if (IsSet(Flag::Z)) {
				pc_ += (int8_t)operand.val;
				cycleLeft_ += (oldPc & 0xFF00) == (pc_ & 0xFF00) ? 1 : 2;
		    }
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kBIT: {
			SetFlag(Flag::N, operand.val & 0b10000000);
			SetFlag(Flag::V, operand.val & 0b01000000);
			SetFlag(Flag::Z, !(acc_ & operand.val));

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kBMI: {
			auto oldPc = pc_;
		    if (IsSet(Flag::N)) {
				pc_ += (int8_t)operand.val;
				cycleLeft_ += (oldPc & 0xFF00) == (pc_ & 0xFF00) ? 1 : 2;
		    }
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kBNE: {
			auto oldPc = pc_;
		    if (!IsSet(Flag::Z)) {
				pc_ += (int8_t)operand.val;
				cycleLeft_ += (oldPc & 0xFF00) == (pc_ & 0xFF00) ? 1 : 2;
		    }
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kBPL: {
			auto oldPc = pc_;
		    if (!IsSet(Flag::N)) {
				pc_ += (int8_t)operand.val;
				cycleLeft_ += (oldPc & 0xFF00) == (pc_ & 0xFF00) ? 1 : 2;
		    }
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kBRK: {
			auto addr = pc_ + 1;
			PushStack(addr >> 8); // HH
			PushStack(addr & 0xFF); // LL
			SetFlag(Flag::I, true);
			PushStack(status_);
			cycleLeft_ += 7;
			break;
		}
		case Instruction::kBVC: {
			auto oldPc = pc_;
		    if (!IsSet(Flag::V)) {
				pc_ += (int8_t)operand.val;
				cycleLeft_ += (oldPc & 0xFF00) == (pc_ & 0xFF00) ? 1 : 2;
		    }
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kBVS: {
			auto oldPc = pc_;
		    if (IsSet(Flag::V)) {
				pc_ += (int8_t)operand.val;
				cycleLeft_ += (oldPc & 0xFF00) == (pc_ & 0xFF00) ? 1 : 2;
		    }
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kCLC: {
			SetFlag(Flag::C, false);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kCLD: {
			SetFlag(Flag::D, false);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kCLI: {
			SetFlag(Flag::I, false);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kCLV: {
			SetFlag(Flag::V, false);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kCMP: {
			auto res = acc_ - operand.val;
			SetFlag(Flag::N, res != 0 ? (res & 0x80) : 0);
			SetFlag(Flag::Z, res == 0);
			SetFlag(Flag::C, res >= 0);

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
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kCPX: {
			auto res = x_ - operand.val;
			SetFlag(Flag::N, res != 0 ? (res & 0x80) : 0);
			SetFlag(Flag::Z, res == 0);
			SetFlag(Flag::C, res >= 0);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kIMM:
					cycleLeft_ += 2;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kCPY: {
			auto res = y_ - operand.val;
			SetFlag(Flag::N, res != 0 ? (res & 0x80) : 0);
			SetFlag(Flag::Z, res == 0);
			SetFlag(Flag::C, res >= 0);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kIMM:
					cycleLeft_ += 2;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kDEC: {
			uint8_t res = operand.val - 1;
			SetFlag(Flag::N, res & 0x80);
			SetFlag(Flag::Z, res == 0);
			bus_->Write(operand.addr, res);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kDEX: {
			x_--;
			SetFlag(Flag::N, x_ & 0x80);
			SetFlag(Flag::Z, x_ == 0);

			cycleLeft_ += 2;
			break;
		}
		case Instruction::kDEY: {
			y_--;
			SetFlag(Flag::N, y_ & 0x80);
			SetFlag(Flag::Z, y_ == 0);

			cycleLeft_ += 2;
			break;
		}
		case Instruction::kEOR: {
			acc_ ^= operand.val;
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
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kINC: {
			uint8_t res = operand.val + 1;
			SetFlag(Flag::N, res & 0x80);
			SetFlag(Flag::Z, res == 0);
			bus_->Write(operand.addr, res);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kINX: {
			x_++;
			SetFlag(Flag::N, x_ & 0x80);
			SetFlag(Flag::Z, x_ == 0);

			cycleLeft_ += 2;
			break;
		}
		case Instruction::kINY: {
			y_++;
			SetFlag(Flag::N, y_ & 0x80);
			SetFlag(Flag::Z, y_ == 0);

			cycleLeft_ += 2;
			break;
		}
		case Instruction::kJMP: {
			pc_ = operand.addr;

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 3;
					break;
				case AddressMode::kIND:
					cycleLeft_ += 5;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kJSR: {
			auto addr = pc_ - 1;
			PushStack(addr >> 8); // HH
			PushStack(addr & 0xFF); // LL
			pc_ = operand.addr;

			cycleLeft_ += 6;
			break;
		}
		case Instruction::kLDA: {
			acc_ = operand.val;
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
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kLDX: {
			x_ = operand.val;
			SetFlag(Flag::N, x_ & 0x80);
			SetFlag(Flag::Z, x_ == 0);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kABY:
					cycleLeft_ += 4 + (operand.boundaryCrossed ? 1 : 0);
					break;
				case AddressMode::kIMM:
					cycleLeft_ += 2;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				case AddressMode::kZPY:
					cycleLeft_ += 4;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kLDY: {
			y_ = operand.val;
			SetFlag(Flag::N, y_ & 0x80);
			SetFlag(Flag::Z, y_ == 0);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 4 + (operand.boundaryCrossed ? 1 : 0);
					break;
				case AddressMode::kIMM:
					cycleLeft_ += 2;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 4;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kLSR: {
			uint8_t res = operand.val >> 1;

			SetFlag(Flag::C, operand.val & 0x01);
			SetFlag(Flag::N, false);
			SetFlag(Flag::Z, res == 0);

			if (operand.addr == 0xFFFF) {
				acc_ = res;
			} else {
				bus_->Write(operand.addr, res);
			}

			switch (op.addrMode) {
				case AddressMode::kACC:
					cycleLeft_ += 2;
					break;
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
		    break;
		}
		case Instruction::kNOP: {
			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 4 + (operand.boundaryCrossed ? 1 : 0);
					break;
				case AddressMode::kIMM:
					cycleLeft_ += 2;
					break;
				case AddressMode::kIMP:
					cycleLeft_ += 2;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 4;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kORA: {
			acc_ = acc_ | operand.val;
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
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kPHA: {
			PushStack(acc_);
			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 3;
			break;
		}
		case Instruction::kPHP: {
			PushStack(status_ | Flag::X | Flag::B);
			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 3;
			break;
		}
		case Instruction::kPLA: {
			acc_ = PopStack();
			SetFlag(Flag::N, acc_ & 0x80);
			SetFlag(Flag::Z, acc_ == 0);

			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 4;
			break;
		}
		case Instruction::kPLP: {
		    status_ = PopStack() & ~Flag::B | Flag::X;
		    assert(op.addrMode == AddressMode::kIMP);
		    cycleLeft_ += 4;
		    break;
		}
		case Instruction::kROL: {
			uint8_t res = operand.val << 1;
			res |= IsSet(Flag::C) ? 0x01 : 0x00;

			SetFlag(Flag::C, operand.val & 0x80);
			SetFlag(Flag::N, res & 0x80);
			SetFlag(Flag::Z, res == 0);

			if (operand.addr == 0xFFFF) {
				acc_ = res;
			} else {
				bus_->Write(operand.addr, res);
			}

			switch (op.addrMode) {
				case AddressMode::kACC:
					cycleLeft_ += 2;
					break;
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kROR: {
			uint8_t res = operand.val >> 1;
			res |= IsSet(Flag::C) ? 0x80 : 0x00;

			SetFlag(Flag::C, operand.val & 0x01);
			SetFlag(Flag::N, res & 0x80);
			SetFlag(Flag::Z, res == 0);

			if (operand.addr == 0xFFFF) {
				acc_ = res;
			} else {
				bus_->Write(operand.addr, res);
			}

			switch (op.addrMode) {
				case AddressMode::kACC:
					cycleLeft_ += 2;
					break;
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kRTI: {
		    status_ = PopStack() & ~Flag::B | Flag::X;
			uint16_t addr = PopStack(); // LL
			addr |= PopStack() << 8; // HH
			pc_ = addr;

			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 6;
			break;
		}
		case Instruction::kRTS: {
			uint16_t addr = PopStack(); // LL
			addr |= PopStack() << 8; // HH
			pc_ = addr + 1;

			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 6;
			break;
		}
		case Instruction::kSBC: {
			const uint16_t sum = acc_ + ~operand.val + (IsSet(Flag::C) ? 1 : 0);
			const uint8_t result = sum & 0xFF;
			SetFlag(Flag::C, !(sum >> 8));
			SetFlag(Flag::V, !!((~(acc_ ^ ~operand.val)) & (acc_ ^ result) & 0x80));
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
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kSEC: {
			SetFlag(Flag::C, true);
			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kSED: {
			SetFlag(Flag::D, true);
			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kSEI: {
			SetFlag(Flag::I, true);
			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kSTA: {
			bus_->Write(operand.addr, acc_);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 5;
					break;
				case AddressMode::kABY:
					cycleLeft_ += 5;
					break;
				case AddressMode::kINX:
					cycleLeft_ += 6;
					break;
				case AddressMode::kINY:
					cycleLeft_ += 6;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 4;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kSTX: {
			bus_->Write(operand.addr, x_);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				case AddressMode::kZPY:
					cycleLeft_ += 4;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kSTY: {
			bus_->Write(operand.addr, y_);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 4;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kTAX: {
			x_ = acc_;
			SetFlag(Flag::N, acc_ & 0x80);
			SetFlag(Flag::Z, acc_ == 0);

			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kTAY: {
			y_ = acc_;
			SetFlag(Flag::N, acc_ & 0x80);
			SetFlag(Flag::Z, acc_ == 0);

			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kTSX: {
			x_ = stackPtr_;
			SetFlag(Flag::N, stackPtr_ & 0x80);
			SetFlag(Flag::Z, stackPtr_ == 0);

			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kTXA: {
			acc_ = x_;
			SetFlag(Flag::N, x_ & 0x80);
			SetFlag(Flag::Z, x_ == 0);

			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kTXS: {
			stackPtr_ = x_;

			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kTYA: {
			acc_ = y_;
			SetFlag(Flag::N, y_ & 0x80);
			SetFlag(Flag::Z, y_ == 0);

			assert(op.addrMode == AddressMode::kIMP);
			cycleLeft_ += 2;
			break;
		}
		// "Illegal" Opcodes and Undocumented Instructions
		case Instruction::kLAX: {
			acc_ = x_ = operand.val;
			SetFlag(Flag::N, acc_ & 0x80);
			SetFlag(Flag::Z, acc_ == 0);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kABY:
					cycleLeft_ += 4 + (operand.boundaryCrossed ? 1 : 0);
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
				case AddressMode::kZPY:
					cycleLeft_ += 4;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kSAX: {
			bus_->Write(operand.addr, acc_ & x_);

			switch (op.addrMode) {
				case AddressMode::kABS:
					cycleLeft_ += 4;
					break;
				case AddressMode::kINX:
					cycleLeft_ += 6;
					break;
				case AddressMode::kZP:
					cycleLeft_ += 3;
					break;
				case AddressMode::kZPY:
					cycleLeft_ += 4;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kUSBC: {
			const uint16_t sum = acc_ + ~operand.val + (IsSet(Flag::C) ? 1 : 0);
			const uint8_t result = sum & 0xFF;
			SetFlag(Flag::C, !(sum >> 8));
			SetFlag(Flag::V, !!((~(acc_ ^ ~operand.val)) & (acc_ ^ result) & 0x80));
			SetFlag(Flag::N, !!(result & 0x80));
			SetFlag(Flag::Z, !result);
			acc_ = result;

			assert(op.addrMode == AddressMode::kIMM);
			cycleLeft_ += 2;
			break;
		}
		case Instruction::kDCP: {
			bus_->Write(operand.addr, operand.val - 1);
			auto res = acc_ - operand.val + 1;
			SetFlag(Flag::N, res != 0 ? (res & 0x80) : 0);
			SetFlag(Flag::Z, (res & 0xFF) == 0);
			SetFlag(Flag::C, res >= 0);

			switch (op.addrMode) {
			    case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
			    case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
			    case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
			    case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
			    case AddressMode::kABY:
					cycleLeft_ += 7;
					break;
			    case AddressMode::kINX:
					cycleLeft_ += 8;
					break;
			    case AddressMode::kINY:
					cycleLeft_ += 8;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kISC: {
			uint8_t incRes = operand.val + 1;
			bus_->Write(operand.addr, incRes);

			const uint16_t sum = acc_ + ~incRes + (IsSet(Flag::C) ? 1 : 0);
			const uint8_t addRes = sum & 0xFF;
			SetFlag(Flag::C, !(sum >> 8));
			SetFlag(Flag::V, !!((~(acc_ ^ ~incRes)) & (acc_ ^ addRes) & 0x80));
			SetFlag(Flag::N, !!(addRes & 0x80));
			SetFlag(Flag::Z, !addRes);
			acc_ = addRes;

			switch (op.addrMode) {
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kABY:
					cycleLeft_ += 7;
					break;
				case AddressMode::kINX:
					cycleLeft_ += 8;
					break;
				case AddressMode::kINY:
					cycleLeft_ += 8;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kSLO: {
			uint8_t shiftRes = operand.val << 1;
			SetFlag(Flag::C, operand.val & 0x80);
			bus_->Write(operand.addr, shiftRes);

			acc_ = acc_ | shiftRes;
			SetFlag(Flag::N, acc_ & 0x80);
			SetFlag(Flag::Z, acc_ == 0);

			switch (op.addrMode) {
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kABY:
					cycleLeft_ += 7;
					break;
				case AddressMode::kINX:
					cycleLeft_ += 8;
					break;
				case AddressMode::kINY:
					cycleLeft_ += 8;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kRLA: {
			uint8_t shiftRes = operand.val << 1;
			shiftRes |= IsSet(Flag::C) ? 0x01 : 0x00;
			SetFlag(Flag::C, operand.val & 0x80);
			bus_->Write(operand.addr, shiftRes);

			acc_ = acc_ & shiftRes;
			SetFlag(Flag::N, acc_ & 0x80);
			SetFlag(Flag::Z, acc_ == 0);

			switch (op.addrMode) {
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kABY:
					cycleLeft_ += 7;
					break;
				case AddressMode::kINX:
					cycleLeft_ += 8;
					break;
				case AddressMode::kINY:
					cycleLeft_ += 8;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kSRE: {
			uint8_t shiftRes = operand.val >> 1;
			SetFlag(Flag::C, operand.val & 0x01);
			bus_->Write(operand.addr, shiftRes);

			acc_ ^= shiftRes;
			SetFlag(Flag::N, acc_ & 0x80);
			SetFlag(Flag::Z, acc_ == 0);

			switch (op.addrMode) {
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kABY:
					cycleLeft_ += 7;
					break;
				case AddressMode::kINX:
					cycleLeft_ += 8;
					break;
				case AddressMode::kINY:
					cycleLeft_ += 8;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
		case Instruction::kRRA: {
			uint8_t shiftRes = operand.val >> 1;
			shiftRes |= IsSet(Flag::C) ? 0x80 : 0x00;
			SetFlag(Flag::C, operand.val & 0x01);
			bus_->Write(operand.addr, shiftRes);

			const uint16_t sum = acc_ + shiftRes + (IsSet(Flag::C) ? 1 : 0);
			const uint8_t addRes = sum & 0xFF;
			SetFlag(Flag::C, sum >> 8);
			SetFlag(Flag::V, !!((acc_ ^ addRes) & (shiftRes ^ addRes) & 0x80));
			SetFlag(Flag::N, !!(addRes & 0x80));
			SetFlag(Flag::Z, !addRes);
			acc_ = addRes;

			switch (op.addrMode) {
				case AddressMode::kZP:
					cycleLeft_ += 5;
					break;
				case AddressMode::kZPX:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABS:
					cycleLeft_ += 6;
					break;
				case AddressMode::kABX:
					cycleLeft_ += 7;
					break;
				case AddressMode::kABY:
					cycleLeft_ += 7;
					break;
				case AddressMode::kINX:
					cycleLeft_ += 8;
					break;
				case AddressMode::kINY:
					cycleLeft_ += 8;
					break;
				default: {
					tfm::printf("Unexpected address mode %s\n", ToString(op.addrMode));
					assert(false);
				}
			}
			break;
		}
    }
}

const CpuState& Cpu6502::GetState() const {
	return cpuState_;
}

Cpu6502::Operand Cpu6502::FetchOperand(AddressMode m) {
	Cpu6502::Operand res;
	uint16_t addr = 0;
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
			addr = Join(LL, HH);

			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kABX: {
			auto LL = bus_->Read(pc_ + 1);
			auto HH = bus_->Read(pc_ + 2);
			addr = Join(LL, HH) + x_;

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
			addr = Join(LL, HH);
			LL = bus_->Read(addr);
			HH = bus_->Read((uint16_t)HH << 8 | ((addr + 1) & 0xFF));
			addr = Join(LL, HH);

			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kINX: {
			addr = bus_->Read(pc_ + 1) + x_;
			auto LL = bus_->Read(addr & 0xFF);
			auto HH = bus_->Read((addr + 1) & 0xFF);
			addr = Join(LL, HH);
			res.val = bus_->Read(addr);
			res.addr = addr;
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kINY: {
			addr = bus_->Read(pc_ + 1);
			auto LL = bus_->Read(addr & 0xFF);
			auto HH = bus_->Read((addr + 1) & 0xFF);
			addr = Join(LL, HH) + y_;

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

	return res;
}

bool Cpu6502::IsSet(Flag f) const {
	return status_ & f;
}

void Cpu6502::SetFlag(Flag f, bool active) {
	if (active) {
		status_ |= f;
	} else {
		status_ &= ~f;
	}
}

void Cpu6502::PushStack(uint8_t val) {
	bus_->Write(0x100 + stackPtr_--, val);
}

uint8_t Cpu6502::PopStack() {
	return bus_->Read(0x100 + ++stackPtr_);
}

void Cpu6502::UpdateState() {
	cpuState_.pc = pc_;
	cpuState_.acc = acc_;
	cpuState_.x = x_;
	cpuState_.y = y_;
	cpuState_.stackPtr = stackPtr_;
	cpuState_.status = status_;
	cpuState_.cycle = cycle_;
}

} // namespace nes
