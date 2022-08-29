#include "nesapp.h"
#include "tfm/tinyformat.h"

namespace {

constexpr uint64_t kClockFrequency = 21477272; // Hz
constexpr uint64_t kPPUFrequency = kClockFrequency / 4; // Hz
constexpr float kPPUTickDuration = 1.f / kPPUFrequency; // s

} // namespace

NesApp::NesApp()
: bus_()
, cpu_(&bus_)
, ppu_(&bus_)
, frameBufferSprite_(256, 240) {
	sAppName = "NesEmu";
}

bool NesApp::OnUserCreate() {
	cpu_.Reset();
	tickDuration_ = kPPUTickDuration;

	ppu_.SetFramebuffer(reinterpret_cast<RGBA*>(frameBufferSprite_.GetData()));

	bus_.AttachController(&con1_, true);
	return true;
}

bool NesApp::OnUserUpdate(float fElapsedTime) {
	if (GetKey(olc::Key::SPACE).bReleased) paused_ = !paused_;
	if (GetKey(olc::Key::Q).bReleased) return false;
	if (GetKey(olc::Key::PGDN).bReleased)
	    tickDuration_ *= 2;
	if (GetKey(olc::Key::PGUP).bReleased)
		tickDuration_ /= 2;


	if (GetKey(olc::Key::A).bPressed) con1_.PressButton(Controller::Button::kStart);
	if (GetKey(olc::Key::A).bReleased) con1_.ReleaseButton(Controller::Button::kStart);
	if (GetKey(olc::Key::S).bPressed) con1_.PressButton(Controller::Button::kSelect);
	if (GetKey(olc::Key::S).bReleased) con1_.ReleaseButton(Controller::Button::kSelect);
	if (GetKey(olc::Key::Z).bPressed) con1_.PressButton(Controller::Button::kA);
	if (GetKey(olc::Key::Z).bReleased) con1_.ReleaseButton(Controller::Button::kA);
	if (GetKey(olc::Key::X).bPressed) con1_.PressButton(Controller::Button::kB);
	if (GetKey(olc::Key::X).bReleased) con1_.ReleaseButton(Controller::Button::kB);
	if (GetKey(olc::Key::UP).bPressed) con1_.PressButton(Controller::Button::kUp);
	if (GetKey(olc::Key::UP).bReleased) con1_.ReleaseButton(Controller::Button::kUp);
	if (GetKey(olc::Key::DOWN).bPressed) con1_.PressButton(Controller::Button::kDown);
	if (GetKey(olc::Key::DOWN).bReleased) con1_.ReleaseButton(Controller::Button::kDown);
	if (GetKey(olc::Key::LEFT).bPressed) con1_.PressButton(Controller::Button::kLeft);
	if (GetKey(olc::Key::LEFT).bReleased) con1_.ReleaseButton(Controller::Button::kLeft);
	if (GetKey(olc::Key::RIGHT).bPressed) con1_.PressButton(Controller::Button::kRight);
	if (GetKey(olc::Key::RIGHT).bReleased) con1_.ReleaseButton(Controller::Button::kRight);

	timeToRun_ += paused_ ? 0.f : fElapsedTime;

	while (timeToRun_ > tickDuration_) {
		if (tickIndex_ == 0) {
			cpu_.Tick();
		}
		tickIndex_ %= 2; // CPU ticks on every 3rd PPU tick
		ppu_.Tick();

		timeToRun_ -= tickDuration_;
	}

	// Display CPU state
	auto state = cpu_.GetState();
	Clear(olc::DARK_BLUE);
	DrawLine(120, 0, 120, ScreenHeight(), olc::WHITE);
	int y = 1;
	DrawString(10, y++ * 10, tfm::format("PC:  0x%04X", state.pc));
	DrawString(10, y++ * 10, tfm::format("A:   0x%02X", state.acc));
	DrawString(10, y++ * 10, tfm::format("X:   0x%02X", state.x));
	DrawString(10, y++ * 10, tfm::format("Y:   0x%02X", state.y));
	DrawString(10, y++ * 10, tfm::format("SP:  0x%02X", state.stackPtr));
	DrawString(10, y++ * 10, tfm::format("P:   0x%02X", state.status));
	DrawString(10, y++ * 10, tfm::format("CYC: %06d", state.cycle));
	DrawLine(0, y * 10, 120, y * 10, olc::WHITE);
	++y;

	// Display palette
	DrawString(10, y++ * 10, "Palettes");
	auto& framePal = ppu_.GetFramePalette();

	for (int palIdx = 0; palIdx < 8; ++palIdx) {
		auto& pal = framePal[palIdx];
		for (int colorIdx = 0; colorIdx < 4; ++colorIdx) {
			uint8_t colorId = pal[colorIdx];
			auto c = kColorPalette[colorId];
			FillRect(colorIdx * 30, y * 10, 30, 30, {c.r, c.g, c.b, c.a});
			DrawString(colorIdx * 30 + 10, y * 10 + 10, tfm::format("%02X", colorId), olc::Pixel(255 - c.r, 255 - c.g, 255 - c.b, 255));
		}
		y += 3;
	}

	DrawSprite(121, 0, &frameBufferSprite_, 2);

	return true;
}

void NesApp::InsertCartridge(Cartridge* cart) {
	bus_.InsertCartridge(cart);
}
