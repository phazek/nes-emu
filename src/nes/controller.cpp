#include "nes/controller.h"

namespace nes {

void Controller::PressButton(Button b) {
	status_ |= b;
}

void Controller::ReleaseButton(Button b) {
	status_ &= ~b;
}

uint8_t Controller::Read() {
	if (readActive_) {
		uint8_t ret = !!(status_ & readIdx_) ? 1 : 0;
		readIdx_ <<= 1;
		if (readIdx_ == 0) {
			readIdx_ = Button::kA;
			readActive_ = false;
		}
		return ret;
	}
	return 1;
}

void Controller::Write(uint8_t val) {
	if (val % 2) {
		pollTriggered_ = true;
	} else {
		if (pollTriggered_) {
			readActive_ = true;
			pollTriggered_ = false;
		}
	}
}

} // namespace nes
