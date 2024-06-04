#include "nes/cpu6502.h"
#include "nes/instructions.h"

#include <tfm/tinyformat.h>
#include <optional>

namespace nes {

namespace {

uint16_t Join(uint8_t LL, uint8_t HH) {
	return (uint16_t)HH << 8 | LL;
}

constexpr uint16_t kStackBase = 0x0100;
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
    // compensate for multi-cycle instructions
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

    auto opCode = bus_->Read(pc_);
    auto op = kOpDecoder.at(opCode);
    auto operand = FetchOperand(op.addrMode);
	pc_ += OpSizeByMode(op.addrMode);

    switch (op.instr) {
		case Instruction::kADC: {
		    ADC(op, operand);
		    break;
		}
		case Instruction::kAND: {
			AND(op, operand);
			break;
		}
		case Instruction::kASL: {
			ASL(op, operand);
		    break;
		}
		case Instruction::kBCC: {
			BCC(op, operand);
			break;
		}
		case Instruction::kBCS: {
			BCS(op, operand);
			break;
		}
		case Instruction::kBEQ: {
			BEQ(op, operand);
			break;
		}
		case Instruction::kBIT: {
			BIT(op, operand);
			break;
		}
		case Instruction::kBMI: {
			BMI(op, operand);
			break;
		}
		case Instruction::kBNE: {
			BNE(op, operand);
			break;
		}
		case Instruction::kBPL: {
			BPL(op, operand);
			break;
		}
		case Instruction::kBRK: {
			BRK(op, operand);
			break;
		}
		case Instruction::kBVC: {
			BVC(op, operand);
			break;
		}
		case Instruction::kBVS: {
			BVS(op, operand);
			break;
		}
		case Instruction::kCLC: {
			CLC(op, operand);
			break;
		}
		case Instruction::kCLD: {
			CLD(op, operand);
			break;
		}
		case Instruction::kCLI: {
			CLI(op, operand);
			break;
		}
		case Instruction::kCLV: {
			CLV(op, operand);
			break;
		}
		case Instruction::kCMP: {
			CMP(op, operand);
			break;
		}
		case Instruction::kCPX: {
			CPX(op, operand);
			break;
		}
		case Instruction::kCPY: {
			CPY(op, operand);
			break;
		}
		case Instruction::kDEC: {
			DEC(op, operand);
			break;
		}
		case Instruction::kDEX: {
			DEX(op, operand);
			break;
		}
		case Instruction::kDEY: {
			DEY(op, operand);
			break;
		}
		case Instruction::kEOR: {
			EOR(op, operand);
			break;
		}
		case Instruction::kINC: {
			INC(op, operand);
			break;
		}
		case Instruction::kINX: {
			INX(op, operand);
			break;
		}
		case Instruction::kINY: {
			INY(op, operand);
			break;
		}
		case Instruction::kJMP: {
			JMP(op, operand);
			break;
		}
		case Instruction::kJSR: {
			JSR(op, operand);
			break;
		}
		case Instruction::kLDA: {
			LDA(op, operand);
			break;
		}
		case Instruction::kLDX: {
			LDX(op, operand);
			break;
		}
		case Instruction::kLDY: {
			LDY(op, operand);
			break;
		}
		case Instruction::kLSR: {
			LSR(op, operand);
		    break;
		}
		case Instruction::kNOP: {
			NOP(op, operand);
			break;
		}
		case Instruction::kORA: {
			ORA(op, operand);
			break;
		}
		case Instruction::kPHA: {
			PHA(op, operand);
			break;
		}
		case Instruction::kPHP: {
			PHP(op, operand);
			break;
		}
		case Instruction::kPLA: {
			PLA(op, operand);
			break;
		}
		case Instruction::kPLP: {
			PLP(op, operand);
		    break;
		}
		case Instruction::kROL: {
			ROL(op, operand);
			break;
		}
		case Instruction::kROR: {
			ROR(op, operand);
			break;
		}
		case Instruction::kRTI: {
			RTI(op, operand);
			break;
		}
		case Instruction::kRTS: {
			RTS(op, operand);
			break;
		}
		case Instruction::kSBC: {
			SBC(op, operand);
			break;
		}
		case Instruction::kSEC: {
			SEC(op, operand);
			break;
		}
		case Instruction::kSED: {
			SED(op, operand);
			break;
		}
		case Instruction::kSEI: {
			SEI(op, operand);
			break;
		}
		case Instruction::kSTA: {
			STA(op, operand);
			break;
		}
		case Instruction::kSTX: {
			STX(op, operand);
			break;
		}
		case Instruction::kSTY: {
			STY(op, operand);
			break;
		}
		case Instruction::kTAX: {
			TAX(op, operand);
			break;
		}
		case Instruction::kTAY: {
			TAY(op, operand);
			break;
		}
		case Instruction::kTSX: {
			TSX(op, operand);
			break;
		}
		case Instruction::kTXA: {
			TXA(op, operand);
			break;
		}
		case Instruction::kTXS: {
			TXS(op, operand);
			break;
		}
		case Instruction::kTYA: {
			TYA(op, operand);
			break;
		}
		// "Illegal" Opcodes and Undocumented Instructions
		case Instruction::kLAX: {
			LAX(op, operand);
			break;
		}
		case Instruction::kSAX: {
			SAX(op, operand);
			break;
		}
		case Instruction::kUSBC: {
			USBC(op, operand);
			break;
		}
		case Instruction::kDCP: {
			DCP(op, operand);
			break;
		}
		case Instruction::kISC: {
			ISC(op, operand);
			break;
		}
		case Instruction::kSLO: {
			SLO(op, operand);
			break;
		}
		case Instruction::kRLA: {
			RLA(op, operand);
			break;
		}
		case Instruction::kSRE: {
			SRE(op, operand);
			break;
		}
		case Instruction::kRRA: {
			RRA(op, operand);
			break;
		}
    }

	if (bus_->CheckDMA()) {
		cycleLeft_ += 513 + (pc_ % 2);
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
			res.boundaryCrossed = false;
			break;
		}
		case AddressMode::kABS: {
			auto LL = bus_->Read(pc_ + 1);
			auto HH = bus_->Read(pc_ + 2);
			addr = Join(LL, HH);

			res.val = bus_->Read(addr, true);
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
	bus_->Write(kStackBase + stackPtr_--, val);
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

// Official op implementations

void Cpu6502::ADC(Operation op, Cpu6502::Operand operand) {
	uint16_t sum = acc_ + operand.val + (IsSet(Flag::C) ? 1 : 0);
	uint8_t result = sum & 0xFF;
	SetFlag(Flag::C, sum >> 8);
	SetFlag(Flag::V, !((acc_ ^ operand.val) & 0x80) && ((acc_ ^ result) & 0x80));
	SetFlag(Flag::N, result & 0x80);
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::AND(Operation op, Cpu6502::Operand operand) {
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::ASL(Operation op, Cpu6502::Operand operand) {
	uint8_t res = operand.val << 1;

	SetFlag(Flag::C, operand.val & 0x80);
	SetFlag(Flag::N, res & 0x80);
	SetFlag(Flag::Z, res == 0);

	if (operand.addr) {
		bus_->Write(operand.addr.value(), res);
	} else {
		acc_ = res;
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::BCC(Operation op, Cpu6502::Operand operand) {
	if (!IsSet(Flag::C)) {
		pc_ += (int8_t)operand.val;
		cycleLeft_ += 3 + (operand.boundaryCrossed ? 1 : 0);
	} else {
		cycleLeft_ += 2;
	}
}

void Cpu6502::BCS(Operation op, Cpu6502::Operand operand) {
	if (IsSet(Flag::C)) {
		pc_ += (int8_t)operand.val;
		cycleLeft_ += 3 + (operand.boundaryCrossed ? 1 : 0);
	} else {
		cycleLeft_ += 2;
	}
}

void Cpu6502::BEQ(Operation op, Cpu6502::Operand operand) {
	if (IsSet(Flag::Z)) {
		pc_ += (int8_t)operand.val;
		cycleLeft_ += 3 + (operand.boundaryCrossed ? 1 : 0);
	} else {
		cycleLeft_ += 2;
	}
}

void Cpu6502::BIT(Operation op, Cpu6502::Operand operand) {
	SetFlag(Flag::N, operand.val & 0x80);
	SetFlag(Flag::V, operand.val & 0x40);
	SetFlag(Flag::Z, !(acc_ & operand.val));

	switch (op.addrMode) {
		case AddressMode::kABS:
			cycleLeft_ += 4;
			break;
		case AddressMode::kZP:
			cycleLeft_ += 3;
			break;
		default: {
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::BMI(Operation op, Cpu6502::Operand operand) {
	if (IsSet(Flag::N)) {
		pc_ += (int8_t)operand.val;
		cycleLeft_ += 3 + (operand.boundaryCrossed ? 1 : 0);
	} else {
		cycleLeft_ += 2;
	}
}

void Cpu6502::BNE(Operation op, Cpu6502::Operand operand) {
	if (!IsSet(Flag::Z)) {
		pc_ += (int8_t)operand.val;
		cycleLeft_ += 3 + (operand.boundaryCrossed ? 1 : 0);
	} else {
		cycleLeft_ += 2;
	}
}

void Cpu6502::BPL(Operation op, Cpu6502::Operand operand) {
	if (!IsSet(Flag::N)) {
		pc_ += (int8_t)operand.val;
		cycleLeft_ += 3 + (operand.boundaryCrossed ? 1 : 0);
	} else {
		cycleLeft_ += 2;
	}
}

void Cpu6502::BRK(Operation op, Cpu6502::Operand operand) {
	auto addr = pc_ + 1;
	PushStack((addr >> 8) & 0xFF);
	PushStack(addr & 0xFF);
	PushStack(status_ | Flag::B | Flag::X);

	pc_ = bus_->Read(0xFFFE) | (bus_->Read(0xFFFF) << 8);
	SetFlag(Flag::I, true);

	cycleLeft_ += 7;
}

void Cpu6502::BVC(Operation op, Cpu6502::Operand operand) {
	if (!IsSet(Flag::V)) {
		pc_ += (int8_t)operand.val;
		cycleLeft_ += 3 + (operand.boundaryCrossed ? 1 : 0);
	} else {
		cycleLeft_ += 2;
	}
}

void Cpu6502::BVS(Operation op, Cpu6502::Operand operand) {
	if (IsSet(Flag::V)) {
		pc_ += (int8_t)operand.val;
		cycleLeft_ += 3 + (operand.boundaryCrossed ? 1 : 0);
	} else {
		cycleLeft_ += 2;
	}
}

void Cpu6502::CLC(Operation op, Cpu6502::Operand operand) {
	SetFlag(Flag::C, false);
	cycleLeft_ += 2;
}

void Cpu6502::CLD(Operation op, Cpu6502::Operand operand) {
	SetFlag(Flag::D, false);
	cycleLeft_ += 2;
}

void Cpu6502::CLI(Operation op, Cpu6502::Operand operand) {
	SetFlag(Flag::I, false);
	cycleLeft_ += 2;
}

void Cpu6502::CLV(Operation op, Cpu6502::Operand operand) {
	SetFlag(Flag::V, false);
	cycleLeft_ += 2;
}

void Cpu6502::CMP(Operation op, Cpu6502::Operand operand) {
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
}

void Cpu6502::CPX(Operation op, Cpu6502::Operand operand) {
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::CPY(Operation op, Cpu6502::Operand operand) {
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
}

void Cpu6502::DEC(Operation op, Cpu6502::Operand operand) {
	uint8_t res = operand.val - 1;
	SetFlag(Flag::N, res & 0x80);
	SetFlag(Flag::Z, res == 0);
	bus_->Write(operand.addr.value(), res);

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
}

void Cpu6502::DEX(Operation op, Cpu6502::Operand operand) {
	x_--;
	SetFlag(Flag::N, x_ & 0x80);
	SetFlag(Flag::Z, x_ == 0);

	cycleLeft_ += 2;
}

void Cpu6502::DEY(Operation op, Cpu6502::Operand operand) {
	y_--;
	SetFlag(Flag::N, y_ & 0x80);
	SetFlag(Flag::Z, y_ == 0);

	cycleLeft_ += 2;
}

void Cpu6502::EOR(Operation op, Cpu6502::Operand operand) {
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
}

void Cpu6502::INC(Operation op, Cpu6502::Operand operand) {
	uint8_t res = operand.val + 1;
	SetFlag(Flag::N, res & 0x80);
	SetFlag(Flag::Z, res == 0);
	bus_->Write(operand.addr.value(), res);

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
}

void Cpu6502::INX(Operation op, Cpu6502::Operand operand) {
	x_++;
	SetFlag(Flag::N, x_ & 0x80);
	SetFlag(Flag::Z, x_ == 0);

	cycleLeft_ += 2;
}

void Cpu6502::INY(Operation op, Cpu6502::Operand operand) {
	y_++;
	SetFlag(Flag::N, y_ & 0x80);
	SetFlag(Flag::Z, y_ == 0);

	cycleLeft_ += 2;
}

void Cpu6502::JMP(Operation op, Cpu6502::Operand operand) {
	pc_ = operand.addr.value();

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
}

void Cpu6502::JSR(Operation op, Cpu6502::Operand operand) {
	auto addr = pc_ - 1;
	PushStack(addr >> 8); // HH
	PushStack(addr & 0xFF); // LL
	pc_ = operand.addr.value();

	cycleLeft_ += 6;
}

void Cpu6502::LDA(Operation op, Cpu6502::Operand operand) {
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::LDX(Operation op, Cpu6502::Operand operand) {
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::LDY(Operation op, Cpu6502::Operand operand) {
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::LSR(Operation op, Cpu6502::Operand operand) {
	uint8_t res = operand.val >> 1;

	SetFlag(Flag::C, operand.val & 0x01);
	SetFlag(Flag::N, false);
	SetFlag(Flag::Z, res == 0);

	if (operand.addr) {
		bus_->Write(operand.addr.value(), res);
	} else {
		acc_ = res;
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::NOP(Operation op, Cpu6502::Operand operand) {
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::ORA(Operation op, Cpu6502::Operand operand) {
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::PHA(Operation op, Cpu6502::Operand operand) {
	PushStack(acc_);
	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 3;
}

void Cpu6502::PHP(Operation op, Cpu6502::Operand operand) {
	PushStack(status_ | Flag::X | Flag::B);
	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 3;
}

void Cpu6502::PLA(Operation op, Cpu6502::Operand operand) {
	acc_ = PopStack();
	SetFlag(Flag::N, acc_ & 0x80);
	SetFlag(Flag::Z, acc_ == 0);

	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 4;
}

void Cpu6502::PLP(Operation op, Cpu6502::Operand operand) {
	status_ = (PopStack() & ~Flag::B) | Flag::X;
	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 4;
}

void Cpu6502::ROL(Operation op, Cpu6502::Operand operand) {
	uint8_t res = operand.val << 1;
	res |= IsSet(Flag::C) ? 0x01 : 0x00;

	SetFlag(Flag::C, operand.val & 0x80);
	SetFlag(Flag::N, res & 0x80);
	SetFlag(Flag::Z, res == 0);

	if (operand.addr) {
		bus_->Write(operand.addr.value(), res);
	} else {
		acc_ = res;
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::ROR(Operation op, Cpu6502::Operand operand) {
	uint8_t res = operand.val >> 1;
	res |= IsSet(Flag::C) ? 0x80 : 0x00;

	SetFlag(Flag::C, operand.val & 0x01);
	SetFlag(Flag::N, res & 0x80);
	SetFlag(Flag::Z, res == 0);

	if (operand.addr) {
		bus_->Write(operand.addr.value(), res);
	} else {
		acc_ = res;
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::RTI(Operation op, Cpu6502::Operand operand) {
	status_ = (PopStack() & ~Flag::B) | Flag::X;
	uint16_t addr = PopStack();  // LL
	addr |= PopStack() << 8;     // HH
	pc_ = addr;

	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 6;
}

void Cpu6502::RTS(Operation op, Cpu6502::Operand operand) {
	uint16_t addr = PopStack();  // LL
	addr |= PopStack() << 8;     // HH
	pc_ = addr + 1;

	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 6;
}

void Cpu6502::SBC(Operation op, Cpu6502::Operand operand) {
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::SEC(Operation op, Cpu6502::Operand operand) {
	SetFlag(Flag::C, true);
	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 2;
}

void Cpu6502::SED(Operation op, Cpu6502::Operand operand) {
	SetFlag(Flag::D, true);
	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 2;
}

void Cpu6502::SEI(Operation op, Cpu6502::Operand operand) {
	SetFlag(Flag::I, true);
	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 2;
}

void Cpu6502::STA(Operation op, Cpu6502::Operand operand) {
	bus_->Write(operand.addr.value(), acc_);

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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::STX(Operation op, Cpu6502::Operand operand) {
	bus_->Write(operand.addr.value(), x_);

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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::STY(Operation op, Cpu6502::Operand operand) {
	bus_->Write(operand.addr.value(), y_);

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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::TAX(Operation op, Cpu6502::Operand operand) {
	x_ = acc_;
	SetFlag(Flag::N, acc_ & 0x80);
	SetFlag(Flag::Z, acc_ == 0);

	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 2;
}

void Cpu6502::TAY(Operation op, Cpu6502::Operand operand) {
	y_ = acc_;
	SetFlag(Flag::N, acc_ & 0x80);
	SetFlag(Flag::Z, acc_ == 0);

	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 2;
}

void Cpu6502::TSX(Operation op, Cpu6502::Operand operand) {
	x_ = stackPtr_;
	SetFlag(Flag::N, stackPtr_ & 0x80);
	SetFlag(Flag::Z, stackPtr_ == 0);

	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 2;
}

void Cpu6502::TXA(Operation op, Cpu6502::Operand operand) {
	acc_ = x_;
	SetFlag(Flag::N, x_ & 0x80);
	SetFlag(Flag::Z, x_ == 0);

	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 2;
}

void Cpu6502::TXS(Operation op, Cpu6502::Operand operand) {
	stackPtr_ = x_;

	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 2;
}

void Cpu6502::TYA(Operation op, Cpu6502::Operand operand) {
	acc_ = y_;
	SetFlag(Flag::N, y_ & 0x80);
	SetFlag(Flag::Z, y_ == 0);

	assert(op.addrMode == AddressMode::kIMP);
	cycleLeft_ += 2;
}

// Unoficcial op implementations

void Cpu6502::LAX(Operation op, Cpu6502::Operand operand) {
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::SAX(Operation op, Cpu6502::Operand operand) {
	bus_->Write(operand.addr.value(), acc_ & x_);

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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::USBC(Operation op, Cpu6502::Operand operand) {
	const uint16_t sum = acc_ + ~operand.val + (IsSet(Flag::C) ? 1 : 0);
	const uint8_t result = sum & 0xFF;
	SetFlag(Flag::C, !(sum >> 8));
	SetFlag(Flag::V, !!((~(acc_ ^ ~operand.val)) & (acc_ ^ result) & 0x80));
	SetFlag(Flag::N, !!(result & 0x80));
	SetFlag(Flag::Z, !result);
	acc_ = result;

	assert(op.addrMode == AddressMode::kIMM);
	cycleLeft_ += 2;
}

void Cpu6502::DCP(Operation op, Cpu6502::Operand operand) {
	bus_->Write(operand.addr.value(), operand.val - 1);
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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::ISC(Operation op, Cpu6502::Operand operand) {
	uint8_t incRes = operand.val + 1;
	bus_->Write(operand.addr.value(), incRes);

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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::SLO(Operation op, Cpu6502::Operand operand) {
	uint8_t shiftRes = operand.val << 1;
	SetFlag(Flag::C, operand.val & 0x80);
	bus_->Write(operand.addr.value(), shiftRes);

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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::RLA(Operation op, Cpu6502::Operand operand) {
	uint8_t shiftRes = operand.val << 1;
	shiftRes |= IsSet(Flag::C) ? 0x01 : 0x00;
	SetFlag(Flag::C, operand.val & 0x80);
	bus_->Write(operand.addr.value(), shiftRes);

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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::SRE(Operation op, Cpu6502::Operand operand) {
	uint8_t shiftRes = operand.val >> 1;
	SetFlag(Flag::C, operand.val & 0x01);
	bus_->Write(operand.addr.value(), shiftRes);

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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}

void Cpu6502::RRA(Operation op, Cpu6502::Operand operand) {
	uint8_t shiftRes = operand.val >> 1;
	shiftRes |= IsSet(Flag::C) ? 0x80 : 0x00;
	SetFlag(Flag::C, operand.val & 0x01);
	bus_->Write(operand.addr.value(), shiftRes);

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
			tfm::printf("Unexpected address mode %s\n",
				    ToString(op.addrMode));
			assert(false);
		}
	}
}
} // namespace nes
