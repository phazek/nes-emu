#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "tfm/tinyformat.h"

namespace nes {
struct RomDescriptor {
    enum class Mirroring { kHorizontal, kVertical };

    size_t prgRomStart = 0;
    size_t prgRomSize = 0;
    size_t chrRomStart = 0;
    size_t chrRomSize = 0;
    bool hasBatteryBackedRAM = false;
    bool hasTrainer = false;
    bool hasFourScreenVRAM = false;
    uint16_t mapperType = 0;

    Mirroring mirrorType;

    const std::string ToString() const {
	const char* format =
R"(PRG: 0x%x - 0x%x (0x%x total, %d bank(s))
CHR: 0x%x - 0x%x (0x%x total, %d bank(s))
Battery backed RAM: %s
Trainer: %s
4-screen VRAM: %s
MapperId: %d
)";
	return tfm::format(format, prgRomStart, prgRomStart + prgRomSize,
			   prgRomSize, prgRomSize / 0x4000, chrRomStart, chrRomStart + chrRomSize,
			   chrRomSize, chrRomSize / 0x2000, hasBatteryBackedRAM, hasTrainer,
			   hasFourScreenVRAM, mapperType);
    }
};
}  // namespace nes
