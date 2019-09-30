#include "plugin.h"
#include "..\injector\assembly.hpp"

using namespace std;
using namespace injector;
using namespace plugin;

fstream lg;

uintptr_t ivfasi;

uintptr_t Fix_202_GetDriver_NoDriverReturn, Fix_202_GetDriver_WithDriverReturn;

void __declspec(naked) Fix_202_GetDriver()
{
	__asm {

		test    dword ptr[ebx + 470h], 4000000h
		jz noDriver

		mov     ecx, [esp + 1Ch]
		mov     ecx, [ecx + 460h]
		test    ecx, ecx
		jz noDriver

		push Fix_202_GetDriver_WithDriverReturn
		ret

		noDriver:
		push Fix_202_GetDriver_NoDriverReturn
		ret
	}
}

class ImVehFtFix {
public:

	static void Fix_211(uintptr_t ivfasi)
	{
		// Fix a common random crash when opening the game. License plate related.
		// This will disable some license plate changes, no cons because SilentPatch already changes all license plates
		MakeNOP((ivfasi+0x3F6D), 5, true);
	}

	static void Fix_202(uintptr_t ivfasi)
	{
		// Fix a random crash related to window shot check, crashes if no driver (idkw, but it happens)
		// This will add a new condition to check if there is driver
		Fix_202_GetDriver_NoDriverReturn = ivfasi + 0x5B05;
		Fix_202_GetDriver_WithDriverReturn = ivfasi + 0x5A49;
		MakeNOP((ivfasi + 0x5A39), 16);
		MakeJMP((ivfasi + 0x5A39), Fix_202_GetDriver);
	}

	////////////////////////////////////////////////

    ImVehFtFix() {

		lg.open("ImVehFtFix.log", fstream::out | fstream::trunc);
        
		ivfasi = (uintptr_t)GetModuleHandle("ImVehFt.asi");

		if (ivfasi) // IVF is installed
		{
			// Find version
			int version = 0;

			int value2 = ReadMemory<uint32_t>((ivfasi+0x16AB2), true);
			if (value2 == 0x0A322E30) // '0.2'
			{
				version = 1;
				lg << "IVF v2.0.2 detected." << "\n";
				lg.flush();
			}
			else
			{
				int value = ReadMemory<uint32_t>((ivfasi+0x24517), true);
				if (value == 0x0A312E31) // '1.1'
				{
					version = 2;
					lg << "IVF v2.1.1 detected." << "\n";
					lg.flush();
				}
				else
				{
					lg << "This IVF version isn't supported by this fix." << "\n";
					lg << "Supported versions are: v2.1.1 and v2.0.2" << "\n";
					lg.flush();
				}
			}

			// Call fix
			switch (version)
			{
			case 1:
				Fix_202(ivfasi);
				break;
			case 2:
				Fix_211(ivfasi);
				break;
			default:
				break;
			}
		}
		else
		{
			lg << "IVF isn't installed." << "\n";
			lg.flush();
		}

    }
} imVehFtFix;
