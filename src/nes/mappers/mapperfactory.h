
#pragma once

#include "mapperbase.h"

namespace nes::mapper {

class MapperFactory {
public:
	static MapperBase* CreateMapper(uint8_t* buffer, size_t bufSize, RomDescriptor desc);
};

} // namespace nes::mapper
