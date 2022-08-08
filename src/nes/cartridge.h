#pragma once

namespace nes {

class Cartridge {
public:
	bool LoadFile(const std::string& filePath);

	uint8_t ReadPrg(uint16_t addr);
	void WritePrg(uint16_t addr, uint8_t val);
	uint8_t ReadChar(uint16_t addr);
	void WriteChar(uint16_t addr, uint8_t val);
};

} // namespace nes
