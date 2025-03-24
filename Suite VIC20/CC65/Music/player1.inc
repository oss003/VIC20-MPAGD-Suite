;---------------------------------------------------------------
; Music player routine
;---------------------------------------------------------------
; - Music player + data should be loaded at $a900
; - $a900 = jmp music_init
; - $a903 = jmp music_on
; - $a906 = jmp music_off
; - $a909 = jmp music_play
; - Zeropage variables from $90 upwards can be used
;---------------------------------------------------------------


;---------------------------------------------------------------
; Zeropage variables
;---------------------------------------------------------------

	SONG_POS          = $d0
	PATTERN_POS       = $d1
	FRAME_COUNT       = $d2
	FRAME_LENGTH      = $d3
	SONG_NEXT_TRUE    = $d4
	SONG_END_TRUE     = $d5

; 2 byte address of pattern data

	CH1_PATTERN_POS   = $d6
	CH2_PATTERN_POS   = $d8
	CH3_PATTERN_POS   = $da
	CH4_PATTERN_POS   = $dc

; #$00 == inactive mode

	CH1_ACTIVE        = $e0
	CH2_ACTIVE        = $e1
	CH3_ACTIVE        = $e2
	CH4_ACTIVE        = $e3

	TEMP_GUY          = $e4
	CHAN_NOTE_DATA    = $e5
	CHANS_INACTIVE    = $e6
	FAST_COUNTER      = $e7
	MEGA_COUNTER      = $e8

; Flag for enable/disable channel for music

	CHANNEL1_ENABLED  = $ea
	CHANNEL2_ENABLED  = $eb
	CHANNEL3_ENABLED  = $ec
	CHANNEL4_ENABLED  = $ed
	MUSIC_ON	      = $ee

;---------------------------------------------------------------
; Constants
;---------------------------------------------------------------

	TEXT_TITLE	= songpos
	TEXT_ARTIST	= songpos+$10
	TEXT_COPY	= songpos+$20
	TABLE_SPEED	= songpos+$30
	TABLE_VOLUME	= songpos+$40
	PATTERNS	= songpos+$50
	SONG_PAGE_1	= songpos+$850
	SONG_PAGE_2	= songpos+$950

; VIC-I chip audio registers
	VIC_CHAN_1	= $900a
	VIC_CHAN_2	= $900b
	VIC_CHAN_3	= $900c
	VIC_CHAN_4	= $900d
	VIC_VOLUME	= $900e


; bit masks for pattern data / effects
	NOTE_IS		= %10000000
	NOTE_OFF	= %00000001
	NOTE_NEXT	= %00000010
	NOTE_END	= %00000011
	NOTE_NOTHING	= %00000000

; VIC VIA constants

	Timer21_Low  	= $9124		; Timer 1 low byte
	Timer21_High 	= $9125		; Timer 1 high byte
	AUX2		= $912b		; Auxiliary control register
	IFR2		= $912d		; Interrupt Flag Register
	IER2		= $912e		; Interrupt Enable Register
	timerint	= 20000		; Interrupt timer 1/50 sec

;---------------------------------------------------------------
; API calls, entry with A = API call
;---------------------------------------------------------------

	jmp music_init
	jmp music_on
	jmp music_off
	jmp music_play

;---------------------------------------------------------------
; Initialisation routine
;---------------------------------------------------------------

music_init:

; Init VIA2 timer21 for IRQ 1/50 sec

	sei			; disable interrupt
	lda #$40		; T1 continuous, no square wave on PB7
	sta AUX2
	lda #$c0		; int timer1
	sta IER2
	lda #<timerint		; timer 1/50 sec
	sta Timer21_Low
	lda #>timerint
	sta Timer21_High
	lda #<int_service	; set new IRQ vector
	sta $0314
	lda #>int_service
	sta $0315

; Init music player

	lda #$00
	sta SONG_POS
	sta PATTERN_POS
	sta SONG_NEXT_TRUE
	sta SONG_END_TRUE
	sta FRAME_COUNT
	sta MEGA_COUNTER

	lda #$01		; load first frame length
	sta FRAME_LENGTH

	rts	


;---------------------------------------------------------------
; INTERRUPT SERVICE ROUTINE
;---------------------------------------------------------------

int_service:
	lda Timer21_Low		; Clear timer1 flag

; Insert interrupt code here

	jsr music_play		; Cycle color topleft char

; End of interrupt code

	pla			; Restore resgisters
	tay
	pla
	tax
	pla

	rti			; Return from interrupt

;---------------------------------------------------------------
; Music_on, enable interrupt and play music
;---------------------------------------------------------------

music_on:
	lda #$c0		; int timer1
	sta IER2
	lda $900e
	ora #$0f
	sta $900e

	lda #$1
	sta MUSIC_ON
	sta CHANNEL1_ENABLED
	sta CHANNEL2_ENABLED
	sta CHANNEL3_ENABLED
	sta CHANNEL4_ENABLED

	cli			; enable interrupts
	rts

;---------------------------------------------------------------
; Music_off, disable interrrupt and stop music
;---------------------------------------------------------------

music_off:
	; sei			; enable interrupts
	; lda #$7f		; int timer1
	; sta IFR2
	; sta IER2
	; lda #$00
	; sta $900e

	lda #$0
	sta MUSIC_ON
	sta CHANNEL1_ENABLED
	sta CHANNEL2_ENABLED
	sta CHANNEL3_ENABLED
	sta CHANNEL4_ENABLED
	sta VIC_CHAN_1
	sta VIC_CHAN_2
	sta VIC_CHAN_3
	sta VIC_CHAN_4

	rts

;---------------------------------------------------------------
; Mainloop
;---------------------------------------------------------------

music_play:
main_loop_skip_raster:

; load them patterns
; resets song position if 4 empty patterns
	jsr SONG_POS_UPDATE

; update song stuff
	jsr AUDIO_UPDATE

; check if NeXT was called
	lda SONG_NEXT_TRUE
	cmp #$00
	beq not_next_effect_called
	lda #$00
	sta PATTERN_POS
	sta SONG_NEXT_TRUE
	inc SONG_POS
	jsr SONG_POS_UPDATE
	jsr AUDIO_PROCESS_CHANNEL
	jmp main_loop_skip_raster

; ready for next music frame?
not_next_effect_called:
	inc FRAME_COUNT
	lda FRAME_COUNT
	cmp FRAME_LENGTH
	bne endmusicplayfunc

; reset frame counter
	lda #$00
	sta FRAME_COUNT

; increase pattern position
	inc PATTERN_POS
	lda PATTERN_POS
	cmp #$10
	bne not_next_pattern

next_pattern:
	lda #$00
	sta PATTERN_POS
	inc SONG_POS
	lda SONG_POS

not_next_pattern:

; done
	inc MEGA_COUNTER
	lda MEGA_COUNTER

endmusicplayfunc:
	rts

;---------------------------------------------------------------
;---------------------------------------------------------------

; grab current playback data and push to VIC

AUDIO_UPDATE:

; y = pattern position
	ldy PATTERN_POS
	lda TABLE_SPEED,y
	sta FRAME_LENGTH
	lda TABLE_VOLUME,y
	and #$f0
	ora VIC_VOLUME
	sta VIC_VOLUME

; x = channel counter
	ldx #$00

; CHANNEL 1
	lda CH1_ACTIVE
	cmp #$00
	bne channel1_active

	lda #$20
	jmp channel1_done

channel1_active:
	lda (CH1_PATTERN_POS),y
	jsr AUDIO_PROCESS_CHANNEL
	adc #$20

channel1_done:

; CHANNEL 2
	inx
	lda CH2_ACTIVE
	cmp #$00
	bne channel2_active

	lda #$20
	jmp channel2_done

channel2_active:
	lda (CH2_PATTERN_POS),y
	jsr AUDIO_PROCESS_CHANNEL
	adc #$20

channel2_done:

; CHANNEL 3
	inx
	lda CH3_ACTIVE
	cmp #$00
	bne channel3_active
	lda #$20
	jmp channel3_done

channel3_active:
	lda (CH3_PATTERN_POS),y
	jsr AUDIO_PROCESS_CHANNEL
	adc #$20

channel3_done:

; CHANNEL 4
	inx
	lda CH4_ACTIVE
	cmp #$00
	bne channel4_active

	lda #$20
	jmp channel4_done

channel4_active:
	lda (CH4_PATTERN_POS),y
	jsr AUDIO_PROCESS_CHANNEL
	adc #$20

channel4_done:
	rts

;---------------------------------------------------------------
;---------------------------------------------------------------

AUDIO_PROCESS_CHANNEL:
	sta CHAN_NOTE_DATA
	and #%10000000
	cmp #%10000000
	bne not_note

is_note:
	lda CHANNEL1_ENABLED,x
	beq channel_disabled_is_note
	lda CHAN_NOTE_DATA
	sta VIC_CHAN_1,x
channel_disabled_is_note:	
	rts

not_note:
	lda CHAN_NOTE_DATA
	cmp #%00000001
	bne not_note_off

	lda CHANNEL1_ENABLED,x
	beq channel_disabled_not_note
	lda #$00
	sta VIC_CHAN_1,x
channel_disabled_not_note:
	rts

not_note_off:
	lda CHAN_NOTE_DATA
	cmp #%00000010
	bne not_note_next

; move song to next song row
	inc SONG_NEXT_TRUE

; XXX do what here?
; will update correctly on next frame
; if we move where this subroutine gets called

;	lda #$00
	rts

not_note_next:
	lda CHAN_NOTE_DATA
	cmp #NOTE_END
	bne not_end_of_song

; turn the sound off cheaply :D/
	lda VIC_VOLUME
	and #$f0
	sta VIC_VOLUME

;-----BUG----------
; soft reset the machine!
; solution from https://www.c64-wiki.com/wiki/Reset_(Process)
;	jmp $fd22
;-----BUG----------

not_end_of_song:
	rts

;---------------------------------------------------------------
;---------------------------------------------------------------

; put pattern addresses in zero page
; SONG_POS should be set before calling

SONG_POS_UPDATE:

; x = channel pattern position offset
	ldx #$00
	stx CHANS_INACTIVE
	lda #$ff
	sta CH1_ACTIVE
	sta CH2_ACTIVE
	sta CH3_ACTIVE
	sta CH4_ACTIVE

song_pos_loop:
	lda SONG_POS
	sec
	sbc #$40
	bpl song_page_2

song_page_1:
	asl
	asl
	sta TEMP_GUY
	txa
	lsr
	clc
	adc TEMP_GUY
	tay
	lda SONG_PAGE_1,y
	jmp pattern_found

song_page_2:
	asl
	asl
	sta TEMP_GUY
	txa
	lsr
	clc
	adc TEMP_GUY
	tay
	lda SONG_PAGE_2,y

pattern_found:
	sta TEMP_GUY

; lets make sure its not an empty pattern
	cmp #$ff
	bne dont_reset

; set channel to inactive mode
	txa
	lsr
	tay
	lda #$00
	sta CH1_ACTIVE,y

; reset song if there are 4 empty patterns
	inc CHANS_INACTIVE
	lda CHANS_INACTIVE
	cmp #$04
	bne dont_reset

song_reset:
	lda #$00
	sta SONG_POS
	jmp SONG_POS_UPDATE

dont_reset:
	lda TEMP_GUY

; LSB
	asl
	asl
	asl
	asl
	clc
	adc #<PATTERNS	;$50
	sta CH1_PATTERN_POS,x
	inx

; MSB11
	lda TEMP_GUY
	lsr
	lsr
	lsr
	lsr
	clc
	adc #>PATTERNS
	sta CH1_PATTERN_POS,x
	inx
	cpx #$08
	bne song_pos_loop
	rts

