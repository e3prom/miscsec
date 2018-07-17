/* foolDLL.cpp: Defines the exported functions for the DLL application.
 * This DLL simply include the XOR, POP, POP, RETN sequence.
 * They may be found in the wild with functions that resturn a Zero or NULL.
 *
 * Compile Instructions:
 * cl /LD /GS foolDLL.cpp /link /SafeSEH:no /DYNAMICBASE:no /NXCompat:no
 */
//#include "stdafx.h"

extern "C" int __declspec(dllexport) test(){
  __asm
        {
            xor eax, eax
            pop esi
            pop ebp
            retn
        }
}
