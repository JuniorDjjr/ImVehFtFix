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
uintptr_t Fix_211new_7BF4_TrueReturn, Fix_211new_7BF4_FalseReturn;
uintptr_t Fix_211new_8B86_TrueReturn, Fix_211new_8B86_FalseReturn;
uintptr_t Fix_211_8BA6_TrueReturn, Fix_211_8BA6_FalseReturn;
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

/////////
void __declspec(naked) Fix_211new_8B86()
{
	__asm {
		test    ecx, ecx
		jz falsereturn

		cmp     byte ptr[ecx + 30Ch], 0
		push Fix_211new_8B86_TrueReturn
		ret

		falsereturn:
		push Fix_211new_8B86_FalseReturn
		ret
	}
}
void __declspec(naked) Fix_211_8BA6()
{
	__asm {
		test    ecx, ecx
		jz falsereturn

		cmp     byte ptr[ecx + 30Ch], 0
		push Fix_211_8BA6_TrueReturn
		ret

		falsereturn :
		push Fix_211_8BA6_FalseReturn
		ret
	}
}

/////////
void __declspec(naked) Fix_211new_7BF4()
{
	__asm {
		cmp    eax, 10h
		jz Fix_211new_7BF4_falsereturn

		mov     cl, [eax]
		mov[eax + edi], cl
		push Fix_211new_7BF4_TrueReturn
		ret

		Fix_211new_7BF4_falsereturn :
		push Fix_211new_7BF4_FalseReturn
		ret
	}
}
void __declspec(naked) Fix_211_7B37()
{
	__asm {
		cmp    eax, 10h
		jz Fix_211_7B37_falsereturn

		mov     cl, [eax]
		mov[eax + edx], cl
		push Fix_211_7B37_TrueReturn
		ret

		Fix_211_7B37_falsereturn :
		push Fix_211_7B37_FalseReturn
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

	static void Fix_211new(uintptr_t ivfasi)
	{
		// Fix a common random crash when opening the game. License plate text related.
		// TODO: Redirect to the original function, this new one looks useless (it was for testing proposes)
		patch::RedirectCall((ivfasi + 0x3F6D), NewRenderLicenseplateTextToRaster, true);
		
		// Fix a common random crash during night.
		Fix_211new_7BF4_TrueReturn = ivfasi + 0x7BF9;
		Fix_211new_7BF4_FalseReturn = ivfasi + 0x7BFE;
		MakeJMP((ivfasi + 0x7BF4), Fix_211new_7BF4);

		// Fix a rare random crash.
		Fix_211new_8B86_TrueReturn = ivfasi + 0x8B8D;
		Fix_211new_8B86_FalseReturn = ivfasi + 0x8C86;
		MakeJMP((ivfasi + 0x8B86), Fix_211new_8B86);
		
		// Disable window message about shader not compiled (the mod works without it).
		MakeJMP((ivfasi + 0x1F1E), (ivfasi + 0x1F3E));
		MakeJMP((ivfasi + 0x1F79), (ivfasi + 0x1F90));

		// Remove IVF logo on menu
		MakeNOP((ivfasi + 0x8104), 10, true);
	}

	static void Fix_211(uintptr_t ivfasi)
	{
		// Fix a common random crash when opening the game. License plate text related.
		// TODO: Redirect to the original function, this new one looks useless (it was for testing proposes)
		patch::RedirectCall((ivfasi + 0x3F6D), NewRenderLicenseplateTextToRaster, true);

		// Fix a common random crash during night.
		Fix_211_7B37_TrueReturn = ivfasi + 0x7B3C;
		Fix_211_7B37_FalseReturn = ivfasi + 0x7B41;
		MakeJMP((ivfasi + 0x7B37), Fix_211_7B37);

		// Fix a rare random crash.
		Fix_211_8BA6_TrueReturn = ivfasi + 0x8BAD;
		Fix_211_8BA6_FalseReturn = ivfasi + 0x8CA6;
		MakeJMP((ivfasi + 0x8BA6), Fix_211_8BA6);

		// Disable window message about shader not compiled (the mod works without it).
		MakeJMP((ivfasi + 0x1F1E), (ivfasi + 0x1F3E));
		MakeJMP((ivfasi + 0x1F79), (ivfasi + 0x1F90));

		// Remove IVF logo on menu
		MakeNOP((ivfasi + 0x8124), 10, true);
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
		lg << "Build 6" << "\n";
        
		ivfasi = (uintptr_t)GetModuleHandle("ImVehFt.asi");

		if (ivfasi) // IVF is installed
		{
			// Find version
			int version = 0;

			int value = ReadMemory<uint16_t>((ivfasi + 0x24CF), true);
			if (value == 0x7501) // '0.2' 01 75
			{
				version = 1;
				lg << "IVF v2.0.2 detected." << "\n";
				lg.flush();
			}
			else
			{
				if (value == 0xF0BF) // '1.1' BF F0
				{
					version = 2;
					lg << "IVF v2.1.1 (old official) detected." << "\n";
					lg.flush();
				}
				else
				{
					if (value == 0xD0BF) // '1.1' D0 BF
					{
						version = 3;
						lg << "IVF v2.1.1 (new unofficial) detected." << "\n";
						lg.flush();
					}
					else
					{
						lg << "This IVF version isn't supported by this fix: " << value << "\n";
						lg << "Supported versions are: v2.1.1 and v2.0.2" << "\n";
						lg.flush();
					}
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
			case 3:
				Fix_211new(ivfasi);
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
