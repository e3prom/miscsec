## Description
The file 'fool.cpp' is a modified version of a Proof of Concept code as written
in the great book 'Gray Hat Hacking The Ethical Hackers Handbook' Fourth
Edition (ISBN-10: 0071832386). The book doesn't mention any author or include
any copyright statement so I include here the modified version. The buffers
sizes are different from the original so that the SEHOP bypass techniques are
applicable. In fact, the third-byte of the controllable stack memory addresses
must be between 0x80 and 0xFB. These addresses may changes from one build
environment to another, this is why I recommend using the binares (see below).

This directory holds the source files of both the main program and its .dll.
The latter includes the sequence of instructions (XOR, POP, POP, RET) necessary
to bypass the SEHOP protection using the technique described by 'Stéfan Le
Berre' and 'Damien Cauquil'. You can find their original paper at
https://sysdream.com/news/lab/2011-03-14-article-outrepasser-sehop

For more information about SEHOP please have a look at
https://blogs.technet.microsoft.com/srd/2009/02/02/preventing-the-exploitation-of-structured-exception-handler-seh-overwrites-with-sehop

## Exploit
The python file is a python-based exploit I wrote to bypass SEHOP on both
Windows 7 and Windows 10. The exploit has been successfully tested on these OS
versions, however keep in mind that the memory alignment may be different on
your host, especially if you chose to compile the Proof of Concept codes.

## SEHOP Opt-in Control
The .reg file is a quick way to activate SEHOP for 'fool.exe' on Windows 7.
SEHOP is activated by default on Windows Server 2008 and Vista SP1. However,
it's disabled by default on Windows 7 and require setting registry keys to
activate it. Setting the value to '0' enables the mitigation technique; a value
of '1' disables it. You may want to disable SEHOP to debug the exception handler
on a debugger such as Immunity Debugger.

## PoC Binaries
The PE/COFF executable and the accompanying .dll have both been compiled with the
/GS flag to add security cookies onto the stack and with /DYNAMICBASE and
/NXCompat disabled. The main executable has been compiled with /SafeSEH on.

Here's the SHA256 checksums for the executable and the DLL:
```
c6cb0eb3a522021c5692e440df28ed4ea3e9ef7fc2e193d83581c2607b459a81  fool.exe
be6a97fdbe3f56d64bda00b97258595295a334caa3b31da047eb83e9edfc0462  foolDLL.dll
```

## PoC Sources
You can compile the provided Proof of Concept sources using Microsoft Visual
Studio 2017 or higher. Please refer to the source code's comment section for
the recommended compilation flags.

## Additional Notes
* On Windows 10, the third address of the default SE Handler is somewhat
  randomized. It looks like the entropy is weak and only 20-40 executions
  may be necessary to hit the correct handler address and to trigger shellcode
  execution.
* It may be necessary to disable Data Execution Prevention (DEP) by adding an
  exception for fool.exe. In fact, to successfully bypass SEHOP, the NSEH
  pointer's last byte should be equal to 0x74 (jmp opcode). These instructions
  including the shellcode reside on the stack, on memory pages not marked for
  executions.
* You can automate the exploitation of fool.exe on Windows 10 using a simple
  for loop:
  ```
  > for /L %I in (1,1,256) DO fool.exe attack.bin || echo exploit attempt %I failed!
  ```
