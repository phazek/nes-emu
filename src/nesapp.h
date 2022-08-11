#pragma once

#include "olc/olcPixelGameEngine.h"
#include "nes/cpu6502.h"
#include "nes/cartridge.h"

using namespace nes;

class NesApp: public olc::PixelGameEngine {
public:
	NesApp();

	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;
	void InsertCartridge(Cartridge* cart);

private:
	Bus bus_;
	Cpu6502 cpu_;
	uint8_t tickIndex_ = 0;
	bool paused_ = false;
	float tickDuration_ = 0.5f;
	float timeUntilTick_ = 0.5f;
};
