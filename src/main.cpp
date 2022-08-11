#include "olc/olcPixelGameEngine.h"
#include "tfm/tinyformat.h"

#include "nesapp.h"
#include "nes/cartridge.h"

int main(int argc, char** argv) {
	NesApp app;
	std::string path;
	if (argc > 1) {
		path = argv[1];
	}

	Cartridge cart;
	if (!cart.LoadFile(path)) {
		tfm::printf("ERROR: Failed to load ROM from path: %s\n", path);
		return 1;
	}

	if (app.Construct(640, 480, 2, 2)) {
		app.InsertCartridge(&cart);
		app.Start();
	}

    return 0;
}
