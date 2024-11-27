;----------------------------------------------
; Turboloader VIC20
; CC65 conversion by Mauro 2024
;----------------------------------------------

;----------------------------------------------
; Header
;----------------------------------------------

.org $a400-2

	.word main
main:
	.include "turboload.inc"

eind_asm:

.out .sprintf("Musicplayer size = %d bytes, free space = %d bytes", (eind_asm - main),(8192-eind_asm + main))
