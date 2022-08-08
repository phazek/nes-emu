#pragma once

#include "mappers/mapperbase.h"

#include <string>
#include <memory>
#include <cstdint>

namespace nes {

class Cartridge {
public:
	bool LoadFile(const std::string& filePath);

	uint8_t ReadPrg(uint16_t addr);
	void WritePrg(uint16_t addr, uint8_t val);
	uint8_t ReadChar(uint16_t addr);
	void WriteChar(uint16_t addr, uint8_t val);
private:

	std::unique_ptr<uint8_t[]> buffer_;
	size_t bufferSize_ = 0;

	RomDescriptor descriptor_;
	std::unique_ptr<mapper::MapperBase> mapper_;
};

} // namespace nes
