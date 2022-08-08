#include "instructions.h"

#include <string>

namespace nes {

const std::string ToString(AddressMode m) {
	switch (m) {
		case AddressMode::kACC: return "ACC";
		case AddressMode::kABS: return "ABS";
		case AddressMode::kABX: return "ABX";
		case AddressMode::kABY: return "ABY";
		case AddressMode::kIMM: return "IMM";
		case AddressMode::kIMP: return "IMP";
		case AddressMode::kIND: return "IND";
		case AddressMode::kINX: return "INX";
		case AddressMode::kINY: return "INY";
		case AddressMode::kREL: return "REL";
		case AddressMode::kZP:  return "ZP";
		case AddressMode::kZPX: return "ZPX";
		case AddressMode::kZPY: return "ZPY";
		default: return std::to_string(static_cast<int>(m));
	}
}

std::string ToString(Instruction ins) {
	switch (ins) {
		case Instruction::kADC: return "ADC";
		case Instruction::kAND: return "AND";
		case Instruction::kASL: return "ASL";
		case Instruction::kBCC: return "BCC";
		case Instruction::kBCS: return "BCS";
		case Instruction::kBEQ: return "BEQ";
		case Instruction::kBIT: return "BIT";
		case Instruction::kBMI: return "BMI";
		case Instruction::kBNE: return "BNE";
		case Instruction::kBPL: return "BPL";
		case Instruction::kBRK: return "BRK";
		case Instruction::kBVC: return "BVC";
		case Instruction::kBVS: return "BVS";
		case Instruction::kCLC: return "CLC";
		case Instruction::kCLD: return "CLD";
		case Instruction::kCLI: return "CLI";
		case Instruction::kCLV: return "CLV";
		case Instruction::kCMP: return "CMP";
		case Instruction::kCPX: return "CPX";
		case Instruction::kCPY: return "CPY";
		case Instruction::kDEC: return "DEC";
		case Instruction::kDEX: return "DEX";
		case Instruction::kDEY: return "DEY";
		case Instruction::kEOR: return "EOR";
		case Instruction::kINC: return "INC";
		case Instruction::kINX: return "INX";
		case Instruction::kINY: return "INY";
		case Instruction::kJMP: return "JMP";
		case Instruction::kJSR: return "JSR";
		case Instruction::kLDA: return "LDA";
		case Instruction::kLDX: return "LDX";
		case Instruction::kLDY: return "LDY";
		case Instruction::kLSR: return "LSR";
		case Instruction::kNOP: return "NOP";
		case Instruction::kORA: return "ORA";
		case Instruction::kPHA: return "PHA";
		case Instruction::kPHP: return "PHP";
		case Instruction::kPLA: return "PLA";
		case Instruction::kPLP: return "PLP";
		case Instruction::kROL: return "ROL";
		case Instruction::kROR: return "ROR";
		case Instruction::kRTI: return "RTI";
		case Instruction::kRTS: return "RTS";
		case Instruction::kSBC: return "SBC";
		case Instruction::kSEC: return "SEC";
		case Instruction::kSED: return "SED";
		case Instruction::kSEI: return "SEI";
		case Instruction::kSTA: return "STA";
		case Instruction::kSTX: return "STX";
		case Instruction::kSTY: return "STY";
		case Instruction::kTAX: return "TAX";
		case Instruction::kTAY: return "TAY";
		case Instruction::kTSX: return "TSX";
		case Instruction::kTXA: return "TXA";
		case Instruction::kTXS: return "TXS";
		case Instruction::kTYA: return "TYA";
		case Instruction::kLAX: return "LAX";
		case Instruction::kSAX: return "SAX";
		case Instruction::kUSBC: return "USBC";
		case Instruction::kDCP: return "DCP";
		case Instruction::kISC: return "ISC";
		case Instruction::kSLO: return "SLO";
		case Instruction::kRLA: return "RLA";
		case Instruction::kSRE: return "SRE";
		case Instruction::kRRA: return "RRA";
		default: return std::to_string(static_cast<int>(ins));
	}
}

int OpSizeByMode(AddressMode m) {
	switch (m) {
	    case AddressMode::kIMP:
	    case AddressMode::kACC:
			return 1;
	    case AddressMode::kIMM:
	    case AddressMode::kINX:
	    case AddressMode::kINY:
	    case AddressMode::kREL:
	    case AddressMode::kZP:
	    case AddressMode::kZPX:
	    case AddressMode::kZPY:
			return 2;
	    case AddressMode::kABS:
	    case AddressMode::kABX:
	    case AddressMode::kABY:
	    case AddressMode::kIND:
			return 3;
	}
}

} // namespace nes
