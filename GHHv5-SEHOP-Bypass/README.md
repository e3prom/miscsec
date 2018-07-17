## Description
The file 'fool.cpp' is a modified version of a Proof of Concept code as written
in the great book 'Gray Hat Hacking The Ethical Hackers Handbook' Fourth
Edition (ISBN-10: 0071832386). The book doesn't mention any author or include
any copyright statement so I include here the modified version. The buffer
sizes are different from the original so the SEHOP bypass techniques are
applicable. In fact, the third-byte of the controllable stack memory addresses
must be between 0x80 and 0xFB.

This directory holds the source files of both the main program and the .DLL.
The latter includes the sequence of instructions (XOR, POP, POP, RET) necessary
to bypass the SEHOP protection using the technique described by 'Stéfan Le
Berre' and 'Damien Cauquil' of 'sysdream'. You can find the original paper at
https://sysdream.com/news/lab/2011-03-14-article-outrepasser-sehop/

For more information about SEHOP please have a look at:
https://blogs.technet.microsoft.com/srd/2009/02/02/preventing-the-exploitation-of-structured-exception-handler-seh-overwrites-with-sehop/

## Exploit
The python file is a python-based exploit I wrote to bypass SEHOP on both
Windows 7 and Windows 10. The exploit has been sucessfully tested on these OS
versions however keep in mind that the memory alignement may be different on
your host, especially if you choose to compile the PoC.

## Structured Exception Handler Overwrite Protection (SEHOP)
The .reg file is a quick way to activate SEHOP for 'fool.exe' on Windows 7.
SEHOP is activated by default on Windows Server 2008 and Vista SP1. However,
it's desactivated by default on Windows 7 and require setting registry keys to
activate it. Setting the value to '0' enable the mitigation technique; a value
of '1' disable it. You may want to disable SEHOP to debug the exception handler
on a debugger such as Immunity Debugger.

## Proof of Concept Binaries
The PE/COFF executable and the .DLL have been compiled with /GS enabled but
/SafeSEH, /DYNAMICBASE and /NXCompat disabled.

Here's the SHA256 checksums for the executable and the DLL:
```
c6cb0eb3a522021c5692e440df28ed4ea3e9ef7fc2e193d83581c2607b459a81  fool.exe
be6a97fdbe3f56d64bda00b97258595295a334caa3b31da047eb83e9edfc0462  foolDLL.dll
```

## Additional Notes
On Windows 10, the third address of the default SE Handler is somewhat
randomized. In reality it looks like the entropy is quite small and only
20-40 executions may be necessary to hit the handler correct address and to
allow for shellcode execution.
