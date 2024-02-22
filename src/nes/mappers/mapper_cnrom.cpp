#include "mapper_cnrom.h"

#include "tfm/tinyformat.h"
#include "../utils.h"

namespace nes::mapper {

namespace {

const std::string kMapperName = "CNROM(003)";
uint16_t kMapperId = 3;

} // namespace

Mapper_CNROM::Mapper_CNROM(uint8_t* buffer, size_t bufSize, RomDescriptor desc)
: MapperBase(buffer, bufSize, desc)
, prgBankCount_(desc.prgRomSize >> 14)
, chrBankCount_(desc.chrRomSize >> 14)
{
	memset(prgBankAddressOffsets_.data(), 0, prgBankAddressOffsets_.size() * sizeof(size_t));
	memset(chrBankAddressOffsets_.data(), 0, chrBankAddressOffsets_.size() * sizeof(size_t));
	Reset();
}

const std::string& Mapper_CNROM::GetName() {
	return kMapperName;
}

uint16_t Mapper_CNROM::GetId() {
	return kMapperId;
}

uint8_t Mapper_CNROM::ReadPrg(uint16_t addr) {
	if (IsInRange(0x8000, 0xBFFF, addr)) {
		return buffer_[descriptor_.prgRomStart + prgBankAddressOffsets_[0] + (addr - 0x8000)];
	} else if (IsInRange(0xC000, 0xFFFF, addr)) {
		return buffer_[descriptor_.prgRomStart + prgBankAddressOffsets_[1] + (addr - 0xC000)];
	}

	tfm::printf("ERROR: Invalid PRG read address at 0x%04X!\n", addr);
	assert(false);
	return 0;
}

std::span<uint8_t> Mapper_CNROM::ReadPrgN(uint16_t addr, uint16_t count) {
	uint16_t endAddr = addr + count;
	if (IsInRange(0x8000, 0xBFFF, addr)) {
		uint16_t effAddr = prgBankAddressOffsets_[0] + (addr - 0x8000);
		return {buffer_ + effAddr, count};
	} else if (IsInRange(0xC000, 0xFFFF, addr)) {
		uint16_t effAddr = prgBankAddressOffsets_[1] + (addr - 0xC000);
		return {buffer_ + effAddr, count};
	}

	tfm::printf("ERROR: Invalid PRG-N read address at 0x%04X!\n", addr);
	assert(false);
	return {};
}

void Mapper_CNROM::WritePrg(uint16_t addr, uint8_t val) {
	if (IsInRange(0x8000, 0xFFFF, addr)) {
		chrBankAddressOffsets_[0] = (addr & 0b00000011) * 0x2000;
		return;
	}

	tfm::printf("ERROR: Invalid PRG write address at 0x%04X!\n", addr);
	assert(false);
}

uint8_t Mapper_CNROM::ReadChar(uint16_t addr) {
	return buffer_[descriptor_.chrRomStart + addr];
}

std::span<uint8_t> Mapper_CNROM::ReadChrN(uint16_t addr, uint16_t count) {
	return {buffer_ + descriptor_.chrRomStart + addr, count};
}

void Mapper_CNROM::WriteChar(uint16_t addr, uint8_t val) {
	tfm::printf("ERROR: Invalid CHR write address at 0x%04X!\n", addr);
	assert(false);
}

void Mapper_CNROM::Reset() {
	prgBankAddressOffsets_[0] = 0x0000;
	prgBankAddressOffsets_[1] = 0x4000;

	chrBankAddressOffsets_[0] = 0x0000;
	chrBankAddressOffsets_[1] = (chrBankCount_ - 1) * 0x2000;
}

} // namespace nes::mapper
