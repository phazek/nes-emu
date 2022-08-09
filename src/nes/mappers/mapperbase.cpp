#include "mapperbase.h"

namespace nes::mapper {

MapperBase::MapperBase(uint8_t* buffer, size_t bufSize, RomDescriptor desc)
: buffer_(buffer)
, bufSize_(bufSize)
, descriptor_(desc) {}

} // namespace nes::mapper
