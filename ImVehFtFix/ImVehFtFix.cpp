#include "plugin.h"
#include "CCustomCarPlateMgr.h"
#include "..\injector\assembly.hpp"
 
using namespace std;
using namespace injector;
using namespace plugin;

fstream lg;

bool gameInitialized;

uintptr_t ivfasi;

uintptr_t Fix_202_GetDriver_NoDriverReturn, Fix_202_GetDriver_WithDriverReturn;
uintptr_t Fix_211_7B37_TrueReturn, Fix_211_7B37_FalseReturn;
uintptr_t Fix_211_3EF8_Return;

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

void __declspec(naked) Fix_211_7B37()
{
	__asm {
		cmp    eax, 10h
		jz falsereturn

		mov     cl, [eax]
		mov[eax + edx], cl
		push Fix_211_7B37_TrueReturn
		ret

		falsereturn:
		push Fix_211_7B37_FalseReturn
		ret
	}
}

void __declspec(naked) Fix_211_3EF8()
{
	__asm {
		//add esp, 12
		push Fix_211_3EF8_Return
		ret
	}
}

char __cdecl NewRenderLicenseplateTextToRaster(char *text, RwRaster *charsRaster, int a3, RwRaster *resultRaster)
{
	char *v4; // eax
	signed int numLetter; // ebp
	char *newTex; // ebx
	unsigned char *oldTex; // eax
	char *_newTex; // esi
	signed int j; // edi
	signed int n; // ecx
	unsigned int line; // [esp+0h] [ebp-10h]
	unsigned int row; // [esp+4h] [ebp-Ch]
	int charsStride; // [esp+8h] [ebp-8h]
	int stride; // [esp+Ch] [ebp-4h]

	if (!gameInitialized)
		return 0;
	v4 = (char*)RwRasterLock(resultRaster, 0, 5);
	if (!v4)
		return 0;
	if (!CharsetLockedData)
		return 0;
	charsStride = charsRaster->stride;
	if (!charsStride)
		return 0;
	stride = resultRaster->stride;
	if (!stride)
		return 0;
	numLetter = 0;
	newTex = v4;
	do
	{
		row = 1;
		line = 1;
		unsigned int *ptrRow = &row;
		unsigned int *ptrLine = &line;
		GetCharacterPositionInCharSet(text[numLetter], *ptrRow, *ptrLine);

		oldTex = &CharsetLockedData[128 * (row + (line << 8))];

		_newTex = newTex;
		j = 64;
		do
		{
			n = 0;
			do
			{
				_newTex[n] = oldTex[n];
				++n;
			} while (n < 128);
			oldTex += charsStride;
			_newTex += stride;
			--j;
		} while (j);
		++numLetter;
		newTex += 128;
	} while (numLetter < 8);
	RwRasterUnlock(resultRaster);
	return 1;
}

/////////////////////////////////////////////////

class ImVehFtFix {
public:

	static void Fix_211(uintptr_t ivfasi)
	{
		// Fix a common random crash when opening the game. License plate text related.
		// TODO: Redirect to the original function, this new one looks useless (it was for testing proposes)
		patch::RedirectCall((ivfasi + 0x3F6D), NewRenderLicenseplateTextToRaster, true);

		// Fix a common random crash during night.
		Fix_211_7B37_TrueReturn = ivfasi + 0x7B3C;
		Fix_211_7B37_FalseReturn = ivfasi + 0x7B41;
		MakeJMP((ivfasi + 0x7B37), Fix_211_7B37);

		// Disable window message about shader not compiled (the mod works without it).
		MakeJMP((ivfasi + 0x1F1E), (ivfasi + 0x1F3E));
		MakeJMP((ivfasi + 0x1F79), (ivfasi + 0x1F90));
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
		gameInitialized = false;

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

		Events::processScriptsEvent += [] {
			gameInitialized = true;
		};

    }
} imVehFtFix;
