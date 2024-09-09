;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macros
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SLOT_NUM = $80
POS_X_CHAR = $81
POS_Y_CHAR = $82
OFFSET_X = $83
OFFSET_Y = $84
TEMP_PTR = $85
;TEMP_PTR+1 = $86
SPRITE_PTR = $87
;SPRITE_PTR+1 = $88
CHAR_RAM_PTR = $89
;CHAR_RAM_PTR+1 = $8a
TABLE_BKG_PTR = $8b
;TABLE_BKG_PTR+1 = $8c
BACKGROUND_RAM_TO_MERGE = $8d
;BACKGROUND_RAM_TO_MERGE+1 = $8e


.macro  AddTo16bitPtrValue toAdd, pointer
    clc
    lda pointer
    adc toAdd
    sta pointer
    bcc :+
    inc pointer+1
:
.endmacro

.macro  AddTo16bitPtrValueDest toAdd, sourcePtr, destPtr
    clc
    lda sourcePtr
    adc toAdd
    sta destPtr
    lda sourcePtr+1
    adc #$00
    sta destPtr+1
.endmacro

.macro  AddTo16bitPtrDest AddValuePtr, SourcePtr, DestPtr
    clc
    lda SourcePtr
    adc AddValuePtr
    sta DestPtr
    lda SourcePtr+1
    adc AddValuePtr+1
    sta DestPtr+1
.endmacro


.macro  SubFrom16bitPtrValue toSub, pointer
    sec
    lda pointer
    sbc toSub
    sta pointer
    bcs :+
    dec pointer+1
:
.endmacro

.macro MS_SET_CHAR_RAM_PTR
    ; Set char ram to be used by the current sprite
    lda SLOT_NUM
    asl ; multiply by two
    tay ; move to y
    lda SpritesPosCharRam,y ; Points to the ram for each slot
    sta CHAR_RAM_PTR
    lda SpritesPosCharRam+1,y
    sta CHAR_RAM_PTR+1
.endmacro

.macro MS_POS_CHAR_ON_SCREEN
    ; Check position of the sprite on screen
    lda dispx
    lsr
    lsr
    lsr ; Divide by 8
    sta POS_X_CHAR

    lda dispx
    lsr
    and #%00000011
    sta OFFSET_X

    lda dispy
    lsr
    lsr
    lsr ; Divide by 8
    sta POS_Y_CHAR

    lda dispy
    and #%00000110
    sta OFFSET_Y
.endmacro

.macro MS_WRITE_BACKGROUND_SPRITE_CHAR
    tay
    lda MemoryCharLow,y
    sta BACKGROUND_RAM_TO_MERGE
    lda MemoryCharHigh,y
    sta BACKGROUND_RAM_TO_MERGE+1

    ; set background to use in sprite in char ram
    ldy #$07
:
    lda (BACKGROUND_RAM_TO_MERGE),y
    sta (CHAR_RAM_PTR),y
    dey
    bpl :-

    AddTo16bitPtrValue #8, CHAR_RAM_PTR
.endmacro

.macro MS_CLEAN_SPRITE_RAM
    ;Find background element
    ; lda POS_X_CHAR
    ; sta dispx
    ; lda POS_Y_CHAR
    ; sta dispy
    ; jsr getoffset

    ; lda #<scraddr
    ; sta TABLE_BKG_PTR
    ; lda #>scraddr
    ; sta TABLE_BKG_PTR+1

    ; lda BackgroundTableLow,y
    ; clc
    ; adc TABLE_BKG_PTR
    ; sta TABLE_BKG_PTR
    ; lda BackgroundTableHigh,y
    ; adc TABLE_BKG_PTR+1
    ; sta TABLE_BKG_PTR+1

    ; clc
    ; txa
    ; adc TABLE_BKG_PTR
    ; sta TABLE_BKG_PTR
    ; lda TABLE_BKG_PTR+1
    ; adc #0
    ; sta TABLE_BKG_PTR+1
    ; ldy #0
    ; lda (TABLE_BKG_PTR),y

    ; MS_WRITE_BACKGROUND_SPRITE_CHAR

    ; ldy #24
    ; lda (TABLE_BKG_PTR),y

    ; MS_WRITE_BACKGROUND_SPRITE_CHAR

    ; ldy #48
    ; lda (TABLE_BKG_PTR),y

    ; MS_WRITE_BACKGROUND_SPRITE_CHAR

    ; ldy #1
    ; lda (TABLE_BKG_PTR),y

    ; MS_WRITE_BACKGROUND_SPRITE_CHAR

    ; ldy #25
    ; lda (TABLE_BKG_PTR),y

    ; MS_WRITE_BACKGROUND_SPRITE_CHAR

    ; ldy #49
    ; lda (TABLE_BKG_PTR),y

    ; MS_WRITE_BACKGROUND_SPRITE_CHAR

    ; ldy #2
    ; lda (TABLE_BKG_PTR),y

    ; MS_WRITE_BACKGROUND_SPRITE_CHAR

    ; ldy #26
    ; lda (TABLE_BKG_PTR),y

    ; MS_WRITE_BACKGROUND_SPRITE_CHAR

    ; ldy #50
    ; lda (TABLE_BKG_PTR),y

    ; MS_WRITE_BACKGROUND_SPRITE_CHAR

    ; SubFrom16bitPtrValue #72, CHAR_RAM_PTR

	ldy #0
	lda #0
:
	sta (CHAR_RAM_PTR),y
	iny
	cpy #72
	bne :-
.endmacro

.macro MS_COPY_SPRITE_CHAR_RAM
    ; Copy selected sprite from table to char ram
    ; moved to the right position in the 3x3 sprite char
    ; and merged with background

    ; First column (uses the left side of the sprite)
    AddTo16bitPtrValueDest OFFSET_Y, CHAR_RAM_PTR, TEMP_PTR
    MS_COPY_8x24_SPRITE_CHAR_RAM z80_c

    ; Third column (uses the left side of the sprite - different filter)
    AddTo16bitPtrValue #48, TEMP_PTR
    MS_COPY_8x24_SPRITE_CHAR_RAM z80_b      

    ; Second column (uses the right side of the sprite no filter)
    AddTo16bitPtrValue #16, SPRITE_PTR
    SubFrom16bitPtrValue #24, TEMP_PTR
    MS_COPY_8x24_SPRITE_CHAR_RAM_NO_FILTER  
.endmacro

.macro MS_COPY_8x24_SPRITE_CHAR_RAM FILTER_SPRITE
    ldy #$0f
:  
    lda (SPRITE_PTR),y
    and FILTER_SPRITE
    eor (TEMP_PTR),y    
    sta (TEMP_PTR),y
    dey
    bpl :-
.endmacro

.macro MS_SET_INITIAL_SPRITE_PTR
	lda z80_de
	sta SPRITE_PTR
	lda z80_de+1
	sta SPRITE_PTR+1
.endmacro

.macro MS_FIND_SPRITE_SLOT
	lda #0			; Set slotnum to 0 by default
	sta SLOT_NUM

	; Find the slot in the SpriteNumTable
	ldy #0
	ldx #0
:
	lda z80_ix
	cmp SpriteNumTable,x
	bne :+
	lda z80_ix+1
	inx	
	cmp SpriteNumTable,x
	bne :+
	; if the address is the same the slot is in y
	sty SLOT_NUM
	jmp :++
:
	inx
	iny
	cpy #6 	; loop for all the sprites on screen
			; TBD - constant with the number of sprites
	bne :--
:
.endmacro

.macro MS_COPY_8x24_SPRITE_CHAR_RAM_NO_FILTER
    ldy #$0f
:  
    lda (SPRITE_PTR),y
    eor (TEMP_PTR),y
    sta (TEMP_PTR),y
    dey
    bpl :-
.endmacro

.macro MS_DRAW_SPRITE_ON_SCREEN
    ; Draw the entire sprite on screen

    lda SLOT_NUM    ; Load the number into the accumulator
    asl         ; Shift left (multiply by 2)
    asl         ; Shift left (multiply by 4)
    asl         ; Shift left (multiply by 8)
    clc         ; Clear the carry flag
    adc SLOT_NUM    ; Add the original number
    tax         ; Store the result in x

    lda SpriteSlotPositionCharRam,x
    inx
    ldy #0
    sta (scraddr),y
    lda SpriteSlotPositionCharRam,x
    inx
    iny
    sta (scraddr),y
    lda SpriteSlotPositionCharRam,x
    inx
    iny
    sta (scraddr),y
    lda SpriteSlotPositionCharRam,x
    inx
    ldy #24
    sta (scraddr),y
    lda SpriteSlotPositionCharRam,x
    inx
    iny
    sta (scraddr),y
    lda SpriteSlotPositionCharRam,x
    inx
    iny
    sta (scraddr),y
    lda SpriteSlotPositionCharRam,x
    inx
    ldy #48
    sta (scraddr),y
    lda SpriteSlotPositionCharRam,x
    inx
    iny
    sta (scraddr),y
    lda SpriteSlotPositionCharRam,x
    inx
    iny
    sta (scraddr),y
.endmacro