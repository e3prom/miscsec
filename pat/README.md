# Print Address Table (PAT)

## Description
`pat` is a simple cross-platform console utility that print in a human-readable format the import and export address tables of PE/COFF executables and dynamic-linked libraries (DLLs). It can also print out a ready-to-paste C++ source code to be imported in Visual Studio for the compilation of a `proxy DLL`, the latter could be used for DLL side-loading / redirection attacks.

## Usage
### Print Import Address Table
```
$ pat -A1 -f bcrypt.dll
library name: ntdll.dll
-----------------------
imported functions:
 - EtwEventRegister()
   * RVA: 87298
   * Ordinal: 58
 - EtwTraceMessage()
   * RVA: 87064
   * Ordinal: 83
 - NtTerminateProcess()
   * RVA: 87318
   * Ordinal: 622
[...]
```
### Print Export Address Table
```
$ pat -A2 -f bcrypt.dll
library name: bcrypt.dll
------------------------
exported function:
 - BCryptAddContextFunction()
   * RVA: 58064
   * Offset: 54992
 - BCryptAddContextFunctionProvider()
   * RVA: 59904
   * Offset: 56832
 - BCryptCloseAlgorithmProvider()
   * RVA: 17136
   * Offset: 14064
 - BCryptConfigureContext()
   * RVA: 57392
   * Offset: 54320
[...]
```
### Print Proxy DLL Source Code for Visual Studio (linker)
```
$ pat -A3 -d evil.dll -f bcrypt.dll
#include "stdafx.h"

HINSTANCE hDll = LoadLibraryA("evil.dll");

#pragma comment(linker, "/export:BCryptAddContextFunction=evil.BCryptAddContextFunction,@1")
#pragma comment(linker, "/export:BCryptAddContextFunctionProvider=evil.BCryptAddContextFunctionProvider,@2")
#pragma comment(linker, "/export:BCryptCloseAlgorithmProvider=evil.BCryptCloseAlgorithmProvider,@3")
#pragma comment(linker, "/export:BCryptConfigureContext=evil.BCryptConfigureContext,@4")
#pragma comment(linker, "/export:BCryptConfigureContextFunction=evil.BCryptConfigureContextFunction,@5")
#pragma comment(linker, "/export:BCryptCreateContext=evil.BCryptCreateContext,@6")
#pragma comment(linker, "/export:BCryptCreateHash=evil.BCryptCreateHash,@7")
#pragma comment(linker, "/export:BCryptCreateMultiHash=evil.BCryptCreateMultiHash,@8")
#pragma comment(linker, "/export:BCryptDecrypt=evil.BCryptDecrypt,@9")
#pragma comment(linker, "/export:BCryptDeleteContext=evil.BCryptDeleteContext,@10")
[...]
```

## Build
To build `pat` simply run `cargo build`:
```
$ cargo build --release
   Compiling proc-macro2 v0.4.27
   Compiling unicode-xid v0.1.0
   Compiling semver-parser v0.7.0
   Compiling syn v0.15.29
   Compiling cfg-if v0.1.7
   Compiling plain v0.2.3
   Compiling unicode-width v0.1.5
   Compiling log v0.4.6
   Compiling getopts v0.2.18
   Compiling semver v0.9.0
   Compiling rustc_version v0.2.3
   Compiling scroll v0.9.2
   Compiling quote v0.6.11
   Compiling scroll_derive v0.9.5
   Compiling goblin v0.0.20
   Compiling pat v0.1.0
    Finished release [optimized] target(s) in 17.09s

$ file target/release/pat
target/release/pat: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/l, for GNU/Linux 3.2.0, BuildID[sha1]=69abc58111c52c6b45d05c2797e337fda2ceb0ec, with debug_info, not stripped
```

## Disclaimer
The author(s) of this program cannot be held responsible for any damages resulting or caused by the direct or indirect usage of this program.