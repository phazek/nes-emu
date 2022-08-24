#include "ppu.h"

#include "bus.h"
#include <tfm/tinyformat.h>

namespace nes {

namespace {

constexpr uint16_t kPPUCTRL = 0x2000;   // WRITE
constexpr uint16_t kPPUMASK = 0x2001;   // WRITE
constexpr uint16_t kPPUSTATUS = 0x2002; // READ
constexpr uint16_t kOAMADDR = 0x2003;   // WRITE
constexpr uint16_t kOAMDATA = 0x2004;   // READ/WRITE
constexpr uint16_t kPPUSCROLL = 0x2005; // WRITE x2
constexpr uint16_t kPPUADDR = 0x2006;   // WRITE x2
constexpr uint16_t kPPUDATA = 0x2007;   // READ/WRITE

std::string AddressToString(uint16_t addr) {
    switch (addr) {
	case kPPUCTRL:
	    return "PPUCTRL(W)";
	case kPPUMASK:
	    return "PPUMASK(W)";
	case kPPUSTATUS:
	    return "PPUSTATUS(R)";
	case kOAMADDR:
	    return "OAMADDR(W)";
	case kOAMDATA:
	    return "OAMDATA(R/W)";
	case kPPUSCROLL:
	    return "PPUSCROLL(Wx2)";
	case kPPUADDR:
	    return "PPUADDR(Wx2)";
	case kPPUDATA:
	    return "PPUDATA(R/W)";
	case kOAMDMA:
	    return "OAMDMA(W)";
	default:
	    return "<UNKNOWN>";
    }
}

} // namespace

Ppu2C02::Ppu2C02(Bus* bus)
: bus_(bus)
{
	bus_->AttachPPU(this);
}

uint8_t Ppu2C02::Read(uint16_t addr) {
	// TODO
	tfm::printf("PPU read %s (0x%04X)\n", AddressToString(addr), addr);
	return 0;
}

void Ppu2C02::Write(uint16_t addr, uint8_t val) {
	// TODO
	tfm::printf("PPU write %s (0x%04X) -> 0x%02X\n", AddressToString(addr), addr, val);
}

void Ppu2C02::Tick() {
	// TODO
}

} // namespace nes
