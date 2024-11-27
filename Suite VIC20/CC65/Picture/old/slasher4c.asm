;----------------------------------------------
;Tents puzzle game
; Atom version by Kees van Oss  2023
;----------------------------------------------
	.DEFINE asm_code $9600-2
	.DEFINE header   0		; Header Wouter Ras emulator
	.DEFINE filenaam "TENTS"

.org asm_code-22*header

.IF header
;********************************************************************
; ATM Header for Atom emulator Wouter Ras

name_start:
	.byte filenaam			; Filename
name_end:
	.repeat 16-name_end+name_start	; Fill with 0 till 16 chars
	  .byte $0
	.endrep

	.word asm_code			; 2 bytes startaddress
	.word exec			; 2 bytes linkaddress
	.word eind_asm-start_asm	; 2 bytes filelength

;********************************************************************
.ENDIF


exec  = start_asm
start_asm:
	.include "slasher4c.inc"
eind_asm:
