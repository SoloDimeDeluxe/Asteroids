#include "game/game.h"

namespace ReyesCuadra
{
	// extern "C" para que el linker encuentre main dentro del namespace
	extern "C" int main()
	{
		game::Run();
		return 0;
	}
}
