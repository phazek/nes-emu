#pragma once

#include "nes/bus.h"
#include "nes/instructions.h"

namespace nes {

enum class AddressMode;

struct CpuState {
	uint16_t pc = 0;
	uint8_t acc = 0;
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t stackPtr = 0;
	uint8_t status = 0;
	uint64_t cycle = 0;
};

class Cpu6502 {
public:
	Cpu6502(Bus* bus);

	void Reset();
	void Tick();

	const CpuState& GetState() const;

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

	CpuState cpuState_;

	Operand FetchOperand(AddressMode m);
	bool IsSet(Flag f) const;
	void SetFlag(Flag f, bool active);
	void PushStack(uint8_t val);
	uint8_t PopStack();

	void UpdateState();

	void ADC(Operation op, Cpu6502::Operand operand);
	void AND(Operation op, Cpu6502::Operand operand);
	void ASL(Operation op, Cpu6502::Operand operand);
	void BCC(Operation op, Cpu6502::Operand operand);
	void BCS(Operation op, Cpu6502::Operand operand);
	void BEQ(Operation op, Cpu6502::Operand operand);
	void BIT(Operation op, Cpu6502::Operand operand);
	void BMI(Operation op, Cpu6502::Operand operand);
	void BNE(Operation op, Cpu6502::Operand operand);
	void BPL(Operation op, Cpu6502::Operand operand);
	void BRK(Operation op, Cpu6502::Operand operand);
	void BVC(Operation op, Cpu6502::Operand operand);
	void BVS(Operation op, Cpu6502::Operand operand);
	void CLC(Operation op, Cpu6502::Operand operand);
	void CLD(Operation op, Cpu6502::Operand operand);
	void CLI(Operation op, Cpu6502::Operand operand);
	void CLV(Operation op, Cpu6502::Operand operand);
	void CMP(Operation op, Cpu6502::Operand operand);
	void CPX(Operation op, Cpu6502::Operand operand);
	void CPY(Operation op, Cpu6502::Operand operand);
	void DEC(Operation op, Cpu6502::Operand operand);
	void DEX(Operation op, Cpu6502::Operand operand);
	void DEY(Operation op, Cpu6502::Operand operand);
	void EOR(Operation op, Cpu6502::Operand operand);
	void INC(Operation op, Cpu6502::Operand operand);
	void INX(Operation op, Cpu6502::Operand operand);
	void INY(Operation op, Cpu6502::Operand operand);
	void JMP(Operation op, Cpu6502::Operand operand);
	void JSR(Operation op, Cpu6502::Operand operand);
	void LDA(Operation op, Cpu6502::Operand operand);
	void LDX(Operation op, Cpu6502::Operand operand);
	void LDY(Operation op, Cpu6502::Operand operand);
	void LSR(Operation op, Cpu6502::Operand operand);
	void NOP(Operation op, Cpu6502::Operand operand);
	void ORA(Operation op, Cpu6502::Operand operand);
	void PHA(Operation op, Cpu6502::Operand operand);
	void PHP(Operation op, Cpu6502::Operand operand);
	void PLA(Operation op, Cpu6502::Operand operand);
	void PLP(Operation op, Cpu6502::Operand operand);
	void ROL(Operation op, Cpu6502::Operand operand);
	void ROR(Operation op, Cpu6502::Operand operand);
	void RTI(Operation op, Cpu6502::Operand operand);
	void RTS(Operation op, Cpu6502::Operand operand);
	void SBC(Operation op, Cpu6502::Operand operand);
	void SEC(Operation op, Cpu6502::Operand operand);
	void SED(Operation op, Cpu6502::Operand operand);
	void SEI(Operation op, Cpu6502::Operand operand);
	void STA(Operation op, Cpu6502::Operand operand);
	void STX(Operation op, Cpu6502::Operand operand);
	void STY(Operation op, Cpu6502::Operand operand);
	void TAX(Operation op, Cpu6502::Operand operand);
	void TAY(Operation op, Cpu6502::Operand operand);
	void TSX(Operation op, Cpu6502::Operand operand);
	void TXA(Operation op, Cpu6502::Operand operand);
	void TXS(Operation op, Cpu6502::Operand operand);
	void TYA(Operation op, Cpu6502::Operand operand);
	void LAX(Operation op, Cpu6502::Operand operand);
	void SAX(Operation op, Cpu6502::Operand operand);
	void USBC(Operation op, Cpu6502::Operand operand);
	void DCP(Operation op, Cpu6502::Operand operand);
	void ISC(Operation op, Cpu6502::Operand operand);
	void SLO(Operation op, Cpu6502::Operand operand);
	void RLA(Operation op, Cpu6502::Operand operand);
	void SRE(Operation op, Cpu6502::Operand operand);
	void RRA(Operation op, Cpu6502::Operand operand);


};
} // namespace nes
