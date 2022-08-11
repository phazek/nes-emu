#include "ppu.h"

#include "bus.h"
namespace nes {

uint8_t Ppu2C02::Read(uint16_t addr) {
	// TODO
	return 0;
}

void Ppu2C02::Write(uint16_t addr, uint8_t val) {
	// TODO
}

void Ppu2C02::AttachBus(Bus* bus) {
	bus_ = bus;
}

} // namespace nes
