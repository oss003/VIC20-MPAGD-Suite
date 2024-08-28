# VIC20-MPAGD-Suite

Usage VIC20 Suite with CMD window:
- Open CMD Window
- Type CD "SUITE VIC20"
- Type BUILD TEST to compile \AGDSOURCE\TEST.AGD


MinGW sources:
- I use MinGW C-compiler to compile file


CC65:
- GAME.ASM is start assembly
- VIC20.CFG is configuration file CC65
- LIB.INC is library with specific VIC20 routines
- \AGD\ENGINE.INC is MPAGD engine

I inserted an endless loop just after the mloop label for testing in the engine.
This code is relocated if I want to test a part of the code.
Screenbuilding is working now.


VIC20 memory:
- $0400-$06ff		= MAP with tile properties
- $0700-$0717		= Screen Line offset table low-byte
- $0718-$072f		= Screen Line offset table low-byte
- $0730-$0fff   = Free
- $1000-$1240   = Screen
- $1400-$16ff   = Font storage                  (char   0- 95)
- $1700-$198f   = Tiles copied from MPAGD       (char  96-177)
- $1990-$1a4f   = 6 objects, 4 chars per object (char 178-201)
- $1a50-$1bff   = 6 sprites, 9 chars per sprite (char 202-255)
- $1c00-$7fff   = Gamecode
- $8000-$8fff   = CHAR ROM
- $9000-$93ff   = VIC VIA's
- $9400-$960f   = To be used for tile colours (24x22)
- $9610-$97ff   = Free
- $9800-$9fff   = I/O
- $a000-$bfff   = Free
- $c000-$dfff   = BASIC ROM
- $e000-$ffff   = KERNAL ROM
