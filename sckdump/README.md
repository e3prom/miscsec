# sckdump
## Summary
sckdump allow you to hexdump data received from a listening socket.

## Features
 * Print out hexadecimal value of data received from a listening socket.
 * Print out printable ASCII characters if any.
 * Support both IPv4 and IPv6 listeners.
 * Support various transport-layer protocols:
   * Transmission Control Protocol (TCP)
   * Stream Control Transmission Protocol (SCTP)
 * Support multiple concurrent connections using fork().

## Build
```
$ git clone https://github.com/e3prom/sckdump
$ make
```

## Example
The below example shows a web browser connecting over IPv6 to the local port
tcp/4446:
```
$ ./sckdump -6 -p 4446 -w 16
[*] SERVER: listening and accepting IPv6 connections on local port tcp/4446
[+] TCP: new client connection from host ::1 (src port tcp/36386)
[+] RECV: received 432 bytes from ::1 (tcp/8846)
    -------------------------------------------------------------------
    47 45 54 20 2f 20 48 54 54 50 2f 31 2e 31 0d 0a  | GET / HTTP/1.1..
    48 6f 73 74 3a 20 5b 3a 3a 31 5d 3a 34 34 34 36  | Host: [::1]:4446
    0d 0a 43 6f 6e 6e 65 63 74 69 6f 6e 3a 20 6b 65  | ..Connection: ke
    65 70 2d 61 6c 69 76 65 0d 0a 55 70 67 72 61 64  | ep-alive..Upgrad
    65 2d 49 6e 73 65 63 75 72 65 2d 52 65 71 75 65  | e-Insecure-Reque
    73 74 73 3a 20 31 0d 0a 55 73 65 72 2d 41 67 65  | sts: 1..User-Age
    6e 74 3a 20 4d 6f 7a 69 6c 6c 61 2f 35 2e 30 20  | nt: Mozilla/5.0 
    28 58 31 31 3b 20 4c 69 6e 75 78 20 78 38 36 5f  | (X11; Linux x86_
    36 34 29 20 41 70 70 6c 65 57 65 62 4b 69 74 2f  | 64) AppleWebKit/
    35 33 37 2e 33 36 20 28 4b 48 54 4d 4c 2c 20 6c  | 537.36 (KHTML, l
    69 6b 65 20 47 65 63 6b 6f 29 20 55 62 75 6e 74  | ike Gecko) Ubunt
    75 20 43 68 72 6f 6d 69 75 6d 2f 36 35 2e 30 2e  | u Chromium/65.0.
    33 33 32 35 2e 31 38 31 20 43 68 72 6f 6d 65 2f  | 3325.181 Chrome/
    36 35 2e 30 2e 33 33 32 35 2e 31 38 31 20 53 61  | 65.0.3325.181 Sa
    66 61 72 69 2f 35 33 37 2e 33 36 0d 0a 41 63 63  | fari/537.36..Acc
    65 70 74 3a 20 74 65 78 74 2f 68 74 6d 6c 2c 61  | ept: text/html,a
    70 70 6c 69 63 61 74 69 6f 6e 2f 78 68 74 6d 6c  | pplication/xhtml
    2b 78 6d 6c 2c 61 70 70 6c 69 63 61 74 69 6f 6e  | +xml,application
    2f 78 6d 6c 3b 71 3d 30 2e 39 2c 69 6d 61 67 65  | /xml;q=0.9,image
    2f 77 65 62 70 2c 69 6d 61 67 65 2f 61 70 6e 67  | /webp,image/apng
    2c 2a 2f 2a 3b 71 3d 30 2e 38 0d 0a 44 4e 54 3a  | ,*/*;q=0.8..DNT:
    20 31 0d 0a 41 63 63 65 70 74 2d 45 6e 63 6f 64  |  1..Accept-Encod
    69 6e 67 3a 20 67 7a 69 70 2c 20 64 65 66 6c 61  | ing: gzip, defla
    74 65 2c 20 62 72 0d 0a 41 63 63 65 70 74 2d 4c  | te, br..Accept-L
    61 6e 67 75 61 67 65 3a 20 65 6e 2d 55 53 2c 65  | anguage: en-US,e
    6e 3b 71 3d 30 2e 39 2c 65 6e 2d 47 42 3b 71 3d  | n;q=0.9,en-GB;q=
    30 2e 38 2c 66 72 3b 71 3d 30 2e 37 0d 0a 0d 0a  | 0.8,fr;q=0.7....
    -------------------------------------------------------------------
[-] TCP: connection from host ::1 (src port tcp/36386) closed.
```

## Usage
```
$ ./sckdump --help
Usage:
	 ./sckdump -4 [options]
	 ./sckdump -6 [options]

Options:
	 -p PORT	 TCP port to listen on (default is tcp/8980).
	 -w NUM		 Hex dump NUM characters at every lines.
	 -t PROT	 Use transport-layer protocol PROT (see below for supported protocols).
	 --help		 Print this help information.

Supported Protocols:
	 TCP	 Transmission Control Protocol
	 SCTP	 Stream Control Transmission Protocol
```

## License
This project is licensed under the terms of the GNU General Public License v3.0.
