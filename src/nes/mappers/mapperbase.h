#pragma once

#include "../romdescriptor.h"

#include <cstdint>
#include <string>
#include <span>

namespace nes::mapper {

class MapperBase {
public:
	MapperBase(uint8_t* buffer, size_t bufSize, RomDescriptor desc);
	virtual ~MapperBase() = default;

	virtual const std::string& GetName() = 0;
	virtual uint16_t GetId() = 0;
	virtual uint8_t ReadPrg(uint16_t addr) = 0;
	virtual std::span<uint8_t> ReadPrgN(uint16_t addr, uint16_t count) = 0;
	virtual void WritePrg(uint16_t addr, uint8_t val) = 0;
	virtual uint8_t ReadChar(uint16_t addr) = 0;
	virtual void WriteChar(uint16_t addr, uint8_t val) = 0;

protected:
	uint8_t* buffer_ = nullptr;
	size_t bufSize_ = 0;
	RomDescriptor descriptor_;
};

} // namespace nes::mapper
