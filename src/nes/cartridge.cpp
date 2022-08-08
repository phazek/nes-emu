#include "cartridge.h"

namespace nes {

bool Cartridge::LoadFile(const std::string& filePath) {
	return true;
}

uint8_t Cartridge::ReadPrg(uint16_t addr) {
}

void Cartridge::WritePrg(uint16_t addr, uint8_t val) {

}

uint8_t Cartridge::ReadChar(uint16_t addr) {
}

void Cartridge::WriteChar(uint16_t addr, uint8_t val) {

}
} // namespace nes
