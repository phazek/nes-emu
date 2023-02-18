#include "mapper_nrom.h"

#include "tfm/tinyformat.h"
#include "../utils.h"

namespace nes::mapper {

namespace {

const std::string kMapperName = "NROM";
uint16_t kMapperId = 0;

} // namespace

Mapper_NROM::Mapper_NROM(uint8_t* buffer, size_t bufSize, RomDescriptor desc):
	MapperBase(buffer, bufSize, desc) {}

const std::string& Mapper_NROM::GetName() {
	return kMapperName;
}

uint16_t Mapper_NROM::GetId() {
	return kMapperId;
}

uint8_t Mapper_NROM::ReadPrg(uint16_t addr) {
	if (IsInRange(0x8000, 0xBFFF, addr)) {
		return buffer_[descriptor_.prgRomStart + (addr - 0x8000)];
	}
	if (IsInRange(0xC000, 0xFFFF, addr)) {
		if (descriptor_.prgRomSize > 0x4000) {
			return buffer_[descriptor_.prgRomStart + (addr - 0x8000)];
		} else {
			return buffer_[descriptor_.prgRomStart + (addr - 0xC000)];
		}
	}

	tfm::printf("ERROR: Invalid PRG read address at 0x%04X!", addr);
	assert(false);
	return 0;
}

std::span<uint8_t> Mapper_NROM::ReadPrgN(uint16_t addr, uint16_t count) {
	if (IsInRange(0x8000, 0xBFFF, addr)) {
		auto effAddr = descriptor_.prgRomStart + addr - 0x8000;
		return {buffer_ + effAddr, count};
	}
	if (IsInRange(0xC000, 0xFFFF, addr)) {
		if (descriptor_.prgRomSize > 0x4000) {
			auto effAddr = descriptor_.prgRomStart + (addr - 0x8000);
			return {buffer_ + effAddr, count};
		} else {
			auto effAddr = descriptor_.prgRomStart + (addr - 0xC000);
			return {buffer_ + effAddr, count};
		}
	}

	assert(false);
	return {};
}

void Mapper_NROM::WritePrg(uint16_t addr, uint8_t val) {
	tfm::printf("ERROR: Invalid PRG write address at 0x%04X!", addr);
	assert(false);
}

uint8_t Mapper_NROM::ReadChar(uint16_t addr) {
	return buffer_[descriptor_.chrRomStart + addr];
}

void Mapper_NROM::WriteChar(uint16_t addr, uint8_t val) {
	tfm::printf("ERROR: Invalid CHR write address at 0x%04X!", addr);
	assert(false);
}

} // namespace nes::mapper
