#include "mapperfactory.h"

#include "tfm/tinyformat.h"
#include "mapper_nrom.h"
#include "mapper_mmc1.h"

namespace nes::mapper {

MapperBase* MapperFactory::CreateMapper(uint8_t* buffer,
					size_t bufSize, RomDescriptor desc)
{
    switch (desc.mapperType) {
		case 0: return new Mapper_NROM(buffer, bufSize, desc);
		case 1: return new Mapper_MMC1(buffer, bufSize, desc);
    }

    tfm::printf("ERROR: unsupported mapper id %d", desc.mapperType);
    return nullptr;
}

} // namespace nes::mapper
