#pragma once

#include "olc/olcPixelGameEngine.h"
#include "nes/cpu6502.h"
#include "nes/ppu.h"
#include "nes/cartridge.h"
#include "nes/controller.h"

using namespace nes;

class NesApp: public olc::PixelGameEngine {
public:
	NesApp();

	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;
	void InsertCartridge(Cartridge* cart);

private:
	void RenderSidePanel();
	void RenderChrBanks();

	Bus bus_;
	Cpu6502 cpu_;
	Ppu2C02 ppu_;
	Controller con1_;
	uint8_t tickIndex_ = 0;
	bool paused_ = false;
	double tickDuration_ = 0.0;
	float timeToRun_ = 0.f;
	bool displayChrBanks_ = false;

	olc::Sprite frameBufferSprite_;
};
