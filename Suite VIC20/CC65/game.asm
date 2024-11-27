;----------------------------------------------
; Common AGD engine
; VIC20 conversion by Kees van Oss 2024
;----------------------------------------------

.segment "ZEROPAGE"
	.include "game.cfg" 
	.include "engine-zp.inc"
	.include "z80-zp.inc"

;----------------------------------------------
; BASIC header
;----------------------------------------------

.segment "BASIC"

	.word load
load:   .word @end
	.word 2
	.byte $9e
	.byte .lobyte(main/1000 .mod 10) + $30
	.byte .lobyte(main/100 .mod 10) + $30
	.byte .lobyte(main/10 .mod 10) + $30
	.byte .lobyte(main/1 .mod 10) + $30
	.byte 0 
@end:   .word 0 

;----------------------------------------------

.segment "CODE"
main:
	.include "game.inc"
	.include "lib.inc"
;	.include "music.inc"
;colscr:	.incbin "..\pictures\slasher3c"
;datscr:	.incbin "..\pictures\slasher3d"

	.include "z80.asm"

eind_asm:
eop:					; End Of Program

.out .sprintf("Gamecode size = %d bytes, free space = %d bytes", (eind_asm - main),(24576-eind_asm + main))
