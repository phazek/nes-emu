#pragma once

#include <cstddef>
#include <cstdint>

namespace nes {
	struct RomDescriptor {
		enum class Mirroring {
			kHorizontal,
			kVertical
		};

		size_t prgRomStart = 0;
		size_t prgRomSize = 0;
		size_t chrRomStart = 0;
		size_t chrRomSize = 0;
		bool hasBatteryBackedRAM = false;
		bool hasTrainer = false;
		bool hasFourScreenVRAM = false;
		uint16_t mapperType = 0;

		Mirroring mirrorType;
	};
} // namespace nes
