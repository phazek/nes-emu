#pragma once

#include "mapperbase.h"

namespace nes::mapper {

class Mapper_NROM: public MapperBase {
public:
	Mapper_NROM(uint8_t* buffer, size_t bufSize, RomDescriptor desc);
	virtual const std::string& GetName() override;
	virtual uint16_t GetId() override;
	virtual uint8_t ReadPrg(uint16_t addr) override;
	virtual std::span<uint8_t> ReadPrgN(uint16_t addr, uint16_t count) override;
	virtual void WritePrg(uint16_t addr, uint8_t val) override;
	virtual uint8_t ReadChar(uint16_t addr) override;
	virtual void WriteChar(uint16_t addr, uint8_t val) override;
private:
};

} // namespace nes::mapper
