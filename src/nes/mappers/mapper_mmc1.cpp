#include "nes/mappers/mapper_mmc1.h"

#include "tfm/tinyformat.h"
#include "nes/utils.h"

namespace nes::mapper {

namespace {

const std::string kMapperName = "MMC1";
uint16_t kMapperId = 1;

} // namespace

Mapper_MMC1::Mapper_MMC1(uint8_t* buffer, size_t bufSize, RomDescriptor desc)
: MapperBase(buffer, bufSize, desc)
, prgBankCount_(desc.prgRomSize >> 14)
{
	memset(prgRAM_.data(), 0, 0x2000);
	memset(prgBankAddressOffsets_.data(), 0, prgBankAddressOffsets_.size() * sizeof(size_t));
	Reset();
}

const std::string& Mapper_MMC1::GetName() {
	return kMapperName;
}

uint16_t Mapper_MMC1::GetId() {
	return kMapperId;
}

uint8_t Mapper_MMC1::ReadPrg(uint16_t addr) {
	if (IsInRange(0x6000, 0x7FFF, addr) && ramEnabled_) { // PRG RAM
		return prgRAM_[addr - 0x6000];
	} else if (IsInRange(0x8000, 0xBFFF, addr)) {
		return buffer_[descriptor_.prgRomStart + prgBankAddressOffsets_[0] + (addr - 0x8000)];
	} else if (IsInRange(0xC000, 0xFFFF, addr)) {
		return buffer_[descriptor_.prgRomStart + prgBankAddressOffsets_[1] + (addr - 0xC000)];
	}

	tfm::printf("ERROR: Invalid PRG read address at 0x%04X!", addr);
	assert(false);
	return 0;
}

std::span<uint8_t> Mapper_MMC1::ReadPrgN(uint16_t addr, uint16_t count) {
	uint16_t endAddr = addr + count;
	if (IsInRange(0x6000, 0x7FFF, addr) && ramEnabled_) { // PRG RAM
		uint16_t effAddr = addr - 0x6000;
		return {prgRAM_.data() + effAddr, count};
	} else if (IsInRange(0x8000, 0xBFFF, addr)) {
		uint16_t effAddr = prgBankAddressOffsets_[0] + (addr - 0x8000);
		return {buffer_ + effAddr, count};
	} else if (IsInRange(0xC000, 0xFFFF, addr)) {
		uint16_t effAddr = prgBankAddressOffsets_[1] + (addr - 0xC000);
		return {buffer_ + effAddr, count};
	}

	tfm::printf("ERROR: Invalid PRG-N read address at 0x%04X!", addr);
	assert(false);
	return {};
}

void Mapper_MMC1::WritePrg(uint16_t addr, uint8_t val) {
	if (IsInRange(0x6000, 0x7FFF, addr) && ramEnabled_) { // PRG RAM
		prgRAM_[addr - 0x6000] = val;
	} else if (IsInRange(0x8000, 0xFFFF, addr)) {
		if (val & 0x80) {
			shiftRegister_ = 0;
			writeCount_ = 0;
			Reset();
		} else {
			writeCount_++;
			shiftRegister_ |= (val & 0x01) << (writeCount_ - 1);

			if (writeCount_ == 5) {
				HandleControlMsg(addr, shiftRegister_);

				shiftRegister_ = 0;
				writeCount_ = 0;
			}
		}
		return;
	}

	tfm::printf("ERROR: Invalid PRG write address at 0x%04X!", addr);
	assert(false);
}

uint8_t Mapper_MMC1::ReadChar(uint16_t addr) {
	return buffer_[descriptor_.chrRomStart + addr];
}

std::span<uint8_t> Mapper_MMC1::ReadChrN(uint16_t addr, uint16_t count) {
	return {buffer_ + descriptor_.chrRomStart + addr, count};
}

void Mapper_MMC1::WriteChar(uint16_t addr, uint8_t val) {
	tfm::printf("ERROR: Invalid CHR write address at 0x%04X!", addr);
	assert(false);
}

void Mapper_MMC1::HandleControlMsg(uint16_t addr, uint8_t msg) {
	if (IsInRange(0x8000, 0x9FFF, addr)) { // Control
		prgRomBankMode_ = (msg & 0x0A) >> 2;
		chrRomBankMode_ = (msg & 0x10) >> 4;
	} else if (IsInRange(0xA000, 0xBFFF, addr)) { // CHR bank 0
		auto bankNr = msg & 0x0F;
		if (chrRomBankMode_ == 0) { // 8KB CHR ROM
			bankNr &= 0xFE;
			chrBankAddressOffsets_[0] = bankNr * 0x1000;
			chrBankAddressOffsets_[1] = (bankNr + 1) * 0x1000;
		} else {
			chrBankAddressOffsets_[0] = bankNr * 0x1000;
		}
		tfm::printf("MMC1: CHR bank swapped:\n- Bank0 = 0x%06X\n- Bank1 = 0x%06X\n",
				chrBankAddressOffsets_[0], chrBankAddressOffsets_[1]);
	} else if (IsInRange(0xC000, 0xDFFF, addr)) { // CHR bank 1
		if (chrRomBankMode_ == 1) {
			auto bankNr = msg & 0x0F;
			chrBankAddressOffsets_[1] = bankNr * 0x1000;
			tfm::printf("MMC1: CHR bank swapped:\n- Bank0 = 0x%06X\n- Bank1 = 0x%06X\n",
					chrBankAddressOffsets_[0], chrBankAddressOffsets_[1]);
		}
	} else if (IsInRange(0xE000, 0xFFFF, addr)) { // PRG bank
		auto bankNr = msg & 0x0F;
		switch (prgRomBankMode_) {
			case 0:
			case 1: {
				bankNr &= 0xFE;
				prgBankAddressOffsets_[0] = bankNr * 0x4000;
				prgBankAddressOffsets_[1] = (bankNr + 1) * 0x4000;
				break;
			}
			case 2: {
				prgBankAddressOffsets_[0] = 0;
				prgBankAddressOffsets_[1] = bankNr * 0x4000;
				break;
			}
			case 3: {
				prgBankAddressOffsets_[0] = bankNr * 0x4000;
				prgBankAddressOffsets_[1] = (prgBankCount_ - 1) * 0x4000;
				break;
			}
		}

		ramEnabled_ = !(msg & 0x10);
		tfm::printf("MMC1: PRG bank swapped (total %d):\n- Bank0 = 0x%06X\n- Bank1 = 0x%06X\n",
				prgBankCount_, prgBankAddressOffsets_[0],prgBankAddressOffsets_[1]);
	}
}

void Mapper_MMC1::Reset() {
	prgBankAddressOffsets_[0] = 0x0000;
	prgBankAddressOffsets_[1] = (prgBankCount_ - 1) * 0x4000;
	tfm::printf("MMC1: PRG bank reset!\n- Bank0 = 0x%06X\n- Bank1 = 0x%06X\n",
			prgBankAddressOffsets_[0], prgBankAddressOffsets_[1]);
}

} // namespace nes::mapper
