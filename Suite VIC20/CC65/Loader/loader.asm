;----------------------------------------------
; Basic loader VIC20
; CC65 conversion by KC 2024
;----------------------------------------------

;----------------------------------------------
; Header
;----------------------------------------------

;.org $1200-2

	.word main
main:
	.include "loader.inc"

eind_asm:

;.out .sprintf("Loader size = %d bytes", (eind_asm - main))
