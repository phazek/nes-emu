#include "utils.h"


namespace nes {

bool IsInRange(uint16_t beg, uint16_t end, uint16_t val) {
	return (beg <= val) && (val <= end);
}

} // namespace nes
