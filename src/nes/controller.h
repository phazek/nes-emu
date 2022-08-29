
#pragma once

#include <cstdint>

namespace nes {

class Controller {
public:
	enum Button {
		kA = 0x01,
		kB = 0x02,
		kSelect = 0x04,
		kStart = 0x08,
		kUp = 0x10,
		kDown = 0x20,
		kLeft = 0x40,
		kRight = 0x80,
	};

	void PressButton(Button b);
	void ReleaseButton(Button b);
	uint8_t Read();
	void Write(uint8_t val);

private:
	uint8_t status_ = 0x00;
	bool readActive_ = false;
	bool pollTriggered_ = false;
	uint8_t readIdx_ = 1;
};

} // namespace nes
