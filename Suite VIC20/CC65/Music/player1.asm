;----------------------------------------------
; Music player VIC20
; CC65 conversion by Mauro 2024
;----------------------------------------------

;----------------------------------------------
; Header
;----------------------------------------------

.segment "CODE"

	.word main
main:
	.include "player1.inc"
	.align $100
songpos:
	.incbin "songdata_pal.bin"

eind_asm:

.out .sprintf("Musicplayer size = %d bytes, free space = %d bytes", (eind_asm - main),(8192-eind_asm + main))
