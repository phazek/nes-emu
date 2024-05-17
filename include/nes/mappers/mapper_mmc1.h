#pragma once

#include "nes/mappers/mapperbase.h"
#include <array>

namespace nes::mapper {

class Mapper_MMC1: public MapperBase {
public:
	Mapper_MMC1(uint8_t* buffer, size_t bufSize, RomDescriptor desc);
	virtual const std::string& GetName() override;
	virtual uint16_t GetId() override;
	virtual uint8_t ReadPrg(uint16_t addr) override;
	virtual std::span<uint8_t> ReadPrgN(uint16_t addr, uint16_t count) override;
	virtual void WritePrg(uint16_t addr, uint8_t val) override;
	virtual uint8_t ReadChar(uint16_t addr) override;
	virtual std::span<uint8_t> ReadChrN(uint16_t addr, uint16_t count) override;
	virtual void WriteChar(uint16_t addr, uint8_t val) override;
private:
	bool ramEnabled_ = true;
	std::array<uint8_t, 0x2000> prgRAM_;
	uint8_t shiftRegister_ = 0;
	uint8_t writeCount_ = 0;

	uint8_t prgRomBankMode_ = 0;
	uint8_t chrRomBankMode_ = 0;

	const int prgBankCount_;
	std::array<size_t, 2> prgBankAddressOffsets_;
	std::array<size_t, 2> chrBankAddressOffsets_ = {0x0000, 0x1000};

	void HandleControlMsg(uint16_t addr, uint8_t msg);
	void Reset();
};

} // namespace nes::mapper
