#include "nesapp.h"
#include "tfm/tinyformat.h"

namespace {

constexpr uint64_t kClockFrequency = 21477272; // Hz
constexpr uint64_t kPPUFrequency = kClockFrequency / 4; // Hz
constexpr float kPPUTickDuration = 1.f / kPPUFrequency; // s

const olc::vi2d kChrBankDisplayPos{80, 80};

void DecodeTileData(const std::span<uint8_t> data,
			const Ppu2C02::Palette& palette, olc::Sprite& output) {
	Tile t;
	t.FromData(data);
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			int idx = y * 8 + x;
			output.SetPixel(
			    x, y,
			    kColorPalette[palette[t.data[idx]]].ToPixel());
		}
	}
}

}  // namespace

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
	if (GetKey(olc::Key::ESCAPE).bReleased) return false;
	if (GetKey(olc::Key::PGDN).bReleased)
		tickDuration_ *= 2;
	if (GetKey(olc::Key::PGUP).bReleased)
		tickDuration_ /= 2;

	if (GetKey(olc::Key::C).bReleased) displayChrBanks_ = !displayChrBanks_;

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

	Clear(olc::DARK_BLUE);

	DrawSprite(121, 0, &frameBufferSprite_, 2);
	RenderSidePanel();

	if (displayChrBanks_) {
		RenderChrBanks();
	}

	return true;
}

void NesApp::InsertCartridge(Cartridge* cart) {
	bus_.InsertCartridge(cart);
}

void NesApp::RenderChrBanks() {
	olc::Sprite tileSprite{8, 8};
	auto& palette = ppu_.GetFramePalette()[4];
	std::span<uint8_t> bank = bus_.ReadChrN(0, 0x2000);

	FillRect(75, 75, 512 + 32 + 10, 256 + 16 + 10,
		 olc::Pixel{255, 200, 200});

	for (int row = 0; row < 16; ++row) {
		for (int col = 0; col < 32; ++col) {
			int idx = row * 32 + col;
			DecodeTileData(bank.subspan(idx * 16, 16), palette,
					   tileSprite);
			DrawSprite(80 + col * 16 + (col > 0 ? col : 0),
				   80 + row * 16 + (row > 0 ? row : 0),
				   &tileSprite, 2);
		}
	}
}

void NesApp::RenderSidePanel() {
	auto state = cpu_.GetState();
	DrawLine(120, 0, 120, ScreenHeight(), olc::WHITE);
	int yPos = 1;
	DrawString(10, yPos++ * 10, tfm::format("PC:  0x%04X", state.pc));
	DrawString(10, yPos++ * 10, tfm::format("A:   0x%02X", state.acc));
	DrawString(10, yPos++ * 10, tfm::format("X:   0x%02X", state.x));
	DrawString(10, yPos++ * 10, tfm::format("Y:   0x%02X", state.y));
	DrawString(10, yPos++ * 10, tfm::format("SP:  0x%02X", state.stackPtr));
	DrawString(10, yPos++ * 10, tfm::format("P:   0x%02X", state.status));
	DrawString(10, yPos++ * 10, tfm::format("CYC: %06d", state.cycle));
	DrawLine(0, yPos * 10, 120, yPos * 10, olc::WHITE);
	++yPos;

	DrawString(10, yPos++ * 10, "Palettes");
	auto& framePal = ppu_.GetFramePalette();

	for (int palIdx = 0; palIdx < 8; ++palIdx) {
		auto& pal = framePal[palIdx];
		for (int colorIdx = 0; colorIdx < 4; ++colorIdx) {
			uint8_t colorId = pal[colorIdx];
			auto c = kColorPalette[colorId];
			FillRect(colorIdx * 30, yPos * 10, 30, 30,
				 {c.r, c.g, c.b, c.a});
			DrawString(
				colorIdx * 30 + 10, yPos * 10 + 10,
				tfm::format("%02X", colorId),
				olc::Pixel(255 - c.r, 255 - c.g, 255 - c.b, 255));
		}
		yPos += 3;
	}

	olc::Sprite spriteZero{8, 8};
	memcpy((void*)spriteZero.GetData(), (void*)ppu_.GetSpriteZero().data(),
		   8 * 8 * sizeof(RGBA));
	DrawSprite(0, yPos * 10, &spriteZero, 4);
}
