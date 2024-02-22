#pragma once

#include "mapperbase.h"
#include <array>

namespace nes::mapper {

class Mapper_CNROM: public MapperBase {
public:
	Mapper_CNROM(uint8_t* buffer, size_t bufSize, RomDescriptor desc);
	virtual const std::string& GetName() override;
	virtual uint16_t GetId() override;
	virtual uint8_t ReadPrg(uint16_t addr) override;
	virtual std::span<uint8_t> ReadPrgN(uint16_t addr, uint16_t count) override;
	virtual void WritePrg(uint16_t addr, uint8_t val) override;
	virtual uint8_t ReadChar(uint16_t addr) override;
	virtual std::span<uint8_t> ReadChrN(uint16_t addr, uint16_t count) override;
	virtual void WriteChar(uint16_t addr, uint8_t val) override;
private:
	uint8_t prgRomBankMode_ = 0;
	uint8_t chrRomBankMode_ = 0;

	const int prgBankCount_;
	const int chrBankCount_;
	std::array<size_t, 2> prgBankAddressOffsets_ = {0x0000, 0x4000};
	std::array<size_t, 2> chrBankAddressOffsets_ = {0x0000, 0x1000};

	void Reset();
};

} // namespace nes::mapper
