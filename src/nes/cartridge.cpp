#include "cartridge.h"
#include "tfm/tinyformat.h"

#include <array>
#include <fstream>

namespace nes {

namespace {

constexpr size_t kHeaderSize = 16;
constexpr std::array<uint8_t, 4> kMagicNumber{'N', 'E', 'S', 0x1A};
} // namespace

bool Cartridge::LoadFile(const std::string& filePath) {
	std::ifstream input{filePath, std::ios::binary};

	if (!input.is_open()) {
		tfm::printf("ERROR: failed to open rom file: %s", filePath);
		return false;
	}

	auto fsize = input.tellg();
	input.seekg( 0, std::ios::end );
	fsize = input.tellg() - fsize;
	bufferSize_ = static_cast<size_t>(fsize);
	input.seekg( 0, std::ios::beg);

	if (bufferSize_ < kHeaderSize) {
		tfm::printf("ERROR: invalid rom file size: %d bytes", bufferSize_);
		return false;
	}

	buffer_ = std::make_unique<uint8_t[]>(fsize);
	if (!input.read(reinterpret_cast<char*>(buffer_.get()), bufferSize_)) {
		tfm::printf("ERROR: unable to read entire file (%d/%d read)", input.gcount(), fsize);
		buffer_.reset();
		return false;
	}

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
