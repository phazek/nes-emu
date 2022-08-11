#include "nesapp.h"
#include "tfm/tinyformat.h"

namespace {

constexpr uint64_t kClockFrequency = 21477272; // Hz
constexpr uint64_t kPPUFrequency = kClockFrequency / 4; // Hz
constexpr float kPPUTickDuration = 1.f / kPPUFrequency; // s

} // namespace

NesApp::NesApp()
: bus_()
, cpu_(&bus_) {
	sAppName = "NesEmu";
}

bool NesApp::OnUserCreate() {
	cpu_.Reset();
	return true;
}

bool NesApp::OnUserUpdate(float fElapsedTime) {
	if (GetKey(olc::Key::SPACE).bReleased) paused_ = !paused_;
	if (GetKey(olc::Key::Q).bReleased) return false;
	if (GetKey(olc::Key::PGDN).bReleased)
	    tickDuration_ *= 2;
	if (GetKey(olc::Key::PGUP).bReleased)
		tickDuration_ /= 2;

	timeUntilTick_ -= paused_ ? 0.f : fElapsedTime;

	if (timeUntilTick_ < 0.f) {
		if (tickIndex_ == 0) {
			cpu_.Tick();

			auto state = cpu_.GetState();
			Clear(olc::DARK_BLUE);
			DrawLine(110, 0, 110, ScreenHeight(), olc::WHITE);
			int y = 1;
			DrawString(10, y++ * 10, tfm::format("PC:  0x%04X", state.pc));
			DrawString(10, y++ * 10, tfm::format("A:   0x%02X", state.acc));
			DrawString(10, y++ * 10, tfm::format("X:   0x%02X", state.x));
			DrawString(10, y++ * 10, tfm::format("Y:   0x%02X", state.y));
			DrawString(10, y++ * 10, tfm::format("SP:  0x%02X", state.stackPtr));
			DrawString(10, y++ * 10, tfm::format("P:   0x%02X", state.status));
			DrawString(10, y++ * 10, tfm::format("CYC: %06d", state.cycle));
			DrawLine(0, y * 10, 110, y * 10, olc::WHITE);
		}
		//ppu_.Tick();

		tickIndex_ %= 2; // CPU ticks on every 3rd PPU tick
		timeUntilTick_ = tickDuration_ * 0.1f;
	}

	return true;
}

void NesApp::InsertCartridge(Cartridge* cart) {
	bus_.InsertCartridge(cart);
}
