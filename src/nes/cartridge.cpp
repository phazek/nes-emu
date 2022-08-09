#include "cartridge.h"
#include "tfm/tinyformat.h"
#include "mappers/mapperfactory.h"

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
		tfm::printf("ERROR: failed to open rom file: %s\n", filePath);
		return false;
	}

	auto fsize = input.tellg();
	input.seekg( 0, std::ios::end );
	fsize = input.tellg() - fsize;
	bufferSize_ = static_cast<size_t>(fsize);
	input.seekg( 0, std::ios::beg);

	if (bufferSize_ < kHeaderSize) {
		tfm::printf("ERROR: invalid rom file size: %d bytes\n", bufferSize_);
		return false;
	}

	buffer_ = std::make_unique<uint8_t[]>(fsize);
	if (!input.read(reinterpret_cast<char*>(buffer_.get()), bufferSize_)) {
		tfm::printf("ERROR: unable to read entire file (%d/%d read)\n", input.gcount(), fsize);
		buffer_.reset();
		return false;
	}

	if (!Init()) {
		buffer_.reset();
		return false;
	}

	return true;
}

uint8_t Cartridge::ReadPrg(uint16_t addr) {
	return mapper_->ReadPrg(addr);
}

void Cartridge::WritePrg(uint16_t addr, uint8_t val) {
	mapper_->WritePrg(addr, val);
}

uint8_t Cartridge::ReadChar(uint16_t addr) {
	return mapper_->ReadChar(addr);
}

void Cartridge::WriteChar(uint16_t addr, uint8_t val) {
	mapper_->WriteChar(addr, val);
}

bool Cartridge::Init() {
	// Check magic number
	if (memcmp(buffer_.get(), kMagicNumber.data(), kMagicNumber.size()) != 0) {
		tfm::printf("ERROR: magic number not found\n");
		return false;
	}

	descriptor_.prgRomSize = buffer_[4] * 0x4000;
	descriptor_.prgRomStart = kHeaderSize;
	descriptor_.chrRomSize = buffer_[5] * 0x2000;
	descriptor_.chrRomStart = descriptor_.prgRomStart + descriptor_.prgRomSize;

	auto flag6 = buffer_[6];
	descriptor_.mirrorType = flag6 & 0x01
					 ? RomDescriptor::Mirroring::kVertical
					 : RomDescriptor::Mirroring::kHorizontal;
	descriptor_.hasBatteryBackedRAM = flag6 & 0x02;
	descriptor_.hasTrainer = flag6 & 0x04;
	if (descriptor_.hasTrainer) {
		descriptor_.prgRomStart += 512;
		descriptor_.chrRomStart += 512;
	}

	descriptor_.hasFourScreenVRAM = flag6 & 0x08;
	descriptor_.mapperType = (flag6 & 0xF0) >> 4;

	auto flag7 = buffer_[7];
	descriptor_.mapperType |= flag7 & 0xF0;

	auto totalRomSize = descriptor_.prgRomStart + descriptor_.prgRomSize + descriptor_.chrRomSize;
	if (totalRomSize > bufferSize_)
	{
		tfm::printf("ERROR: header-content size mismatch (%d <> %d)\n", totalRomSize, bufferSize_);
		return false;
	}

	mapper_.reset(mapper::MapperFactory::CreateMapper(buffer_.get(), bufferSize_, descriptor_));
	if (!mapper_) {
		tfm::printf("ERROR: failed to create mapper\n");
		return false;
	}

	return true;
}

} // namespace nes
