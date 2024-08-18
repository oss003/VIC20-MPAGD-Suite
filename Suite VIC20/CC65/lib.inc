;============================================================
; PLATFORM VARIABLES
;============================================================
;
; Memory map:
;
;	$0000-$03ff	 1.0kB	CPU WS
;	$0400-$0fff	 3,0kB	RAM
;	$1000-$17ff	 2,0kB	VIDEO RAM
;	$1800-$1fff	 2,0kB	CHAR RAM
;	$2000-$7fff	24,0kB	RAM
;	$8000-$8fff	 4,0kB	CHAR ROM
;	$9000-$93ff	 1,0kB	VIC VIA's
;	$9400-$97ff	 1,0kB	COLOUR RAM
;	$9800-$9fff	 2,0kB	I/O
;	$a000-$bfff	 8,0kB	RAM
;	$c000-$dfff	 8,0kB	BASIC ROM
;	$e000-$ffff	 8,0kB	KERNAL ROM
;
; Keyboard matrix:
;                          	  $9121
;          $9120	b7  b6  b5  b4  b3  b2  b1  b0
; 	127-0111 1111 	F7  Hom  -   0   8   6   4   2
; 	191-1011 1111 	F5       @   O   U   T   E   Q
; 	223-1101 1111 	F3   =   :   K   H   F   S  Com
; 	239-1110 1111 	F1  RSh  .   M   B   C   Z  Spc
; 	247-1111 0111	Cur  /   ,   N   V   X  LSh Run
; 	251-1111 1011	Cur  ;   L   J   G   D   A  Ctr
; 	253-1111 1101	Ret  *   P   I   Y   R   W
; 	254-1111 1110	Del Pnd  +   9   7   5   3   1
;
;	 P =$65,O=$14,A=$51,Q=$10
;	SPC=$30,N=$44,M=$34
;	 1 =$70,2=$00,3=$71,4=$01
;
; Joysticks:
;
;               b7    b6    b5    b4    b3    b2    b1    b0
;	$9111	-     -     Fire  Left  Down  Up    -     -
;	$9120	Right -     -     -     -     -     -     -
;
;	$9122	DDRB = Data Direction Register port $9120
;               0 = input, 1 = output
;		Bit7 should be 0 to read joystick $9120
;
; Video registers:
;
;	$9000	horizontal centering
;		bits 0-6 horizontal centering
;		bit  7 sets interlace scan
;	$9001	vertical centering
;	$9002	set # of columns
;		bits 0-6 set # of columns
;		bit 7 is part of video matrix address
;	$9003	set # of rows
;		bit 0 sets 8x8 or 16x8 chars
;		bits 1-6 set # of rows
;	$9005	start of character memory
;		bits 0-3 start of character memory (default = 0)
;		bits 4-7 is rest of video address (default= F)
;		BITS 3,2,1,0 CM startinq address
;			     HEX   DEC
;		0000   ROM   8000  32768
;		0001	     8400  33792
;		0010	     8800  34816
;		0011	     8C00  35840
;		1000   RAM   0000  0000
;		1001  xxxx
;		1010  xxxx   unavail.
;		1011  xxxx
;		1100	     1000  4096
;		1101	     1400  5120
;		1110	     1800  6144
;		1111	     1C00  7168
;	$900f	Screen and border color register
;		bits 4-7 select background color
;		bits 0-2 select border color
;		bit 3 selects inverted or normal mode
;
;------------------------------------------------------------
; VIC variables:
;
	fontcol:		.byte 0

; Settings

	SrcWidth	= 24
	ScrHeight	= 24
	CentreX		= 2
	CentreY		= 24

; Addresses

	MapAddr		= $400		; RAM
	ScreenAddr	= $1000		; VIDEO RAM
	CharAddress	= $1800		; CHAR RAM
	Reg0		= $9000		; Horizontal centering
	Reg1		= $9001		; Vertical centering
	Reg2		= $9002		; Set # of columns
	Reg3		= $9003		; Set # of rows
	Reg5		= $9005		; Start of character memory
	RegF		= $900f		; Screen and border color register
	JoyAddr1	= $9111		; Joystick address 1
	KeyColAddr	= $9120		; Keyboard column (write)
	JoyAddr2	= $9120		; Joystick address 2
	KeyRowAddr	= $9121		; Keyboard row (read)
	DDRB		= $9122		; Bit7 must be 0 for joystick read
	ColorAttr	= $9400		; Color attributes

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; System constants Atom, needs to be updated for VIC usage 

;	ScreenAddr  	= $8000		; Startaddress video RAM
	ScrSelAddr  	= $b000		; Screen selection address
	SpeakerBit  	= $b002		; Speaker bit
	CmdReg      	= $b400		; Command register AtoMMC
	DatReg      	= $b401		; Data register AtoMMC 
	Timer2_Low  	= $b808		; Timer 2 low byte
	Timer2_High 	= $b809		; Timer 2 high byte
	Timer1_Low  	= $b804		; Timer 1 low byte
	Timer1_High 	= $b805		; Timer 1 high byte

	green		= $00		; Colour definition filters
	yellow		= $55
	blue		= $aa
	red		= $ff

; System calls Atom

	SCRSYNC	 	= $fe66		; wait for next CRT field flyback 60 Hz
	SCRSYNC1 	= $fe6b		; wait for CRT field flyback 60 Hz
	OSRDCH   	= $fe94		; get key subroutine
	OSWRCH	 	= $fff4		; write character
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

;============================================================
; PLATFORM ROUTINES
;============================================================

;------------------------------------------------------------
; INIT, Initialise system
;
; - Disable interrupts
; - Init NTSC screen
; - Copy MPAGD font (96 chars = 768 bytes) to CHAR RAM (CHARcode 0-95)
; - Copy MPAGD tiles to CHAR RAM starting at $1b00 (CHARcode 96-) 
; - Set up screen line offset address table.
;------------------------------------------------------------

Init:

; Disable interrupts

	lda #$7f
	sta $911e
	sta $912e

; Init screen for NTSC
; this sets the background and border color of the screen

; in this case border black and background black
	lda #%00011000			
	sta RegF

; max num of columns (24)
	lda Reg2
	and #%10000000
	ora #SrcWidth
	sta Reg2

; max num of lines (mult by 2 - 24) for 8x8 chars
	lda Reg3
	and #%01000001
	ora #SrcWidth*2		; number of rows multipled by two
	sta Reg3

; horizontal and vertical position must be set for 
; non standard screen resolution (standard 22x23)
; same games even allow the user to change them
; these values are for PAL for NTSC should be different

; move horizontal position in the screen
	lda Reg0
	and #%10000000
	ora #CentreX
	sta Reg0

; move vertical position in the screen
	lda #CentreY
	sta Reg1

; Set character data pointer to CHAR RAM at $1400
	lda Reg5
	and #%11110000
	ora #%1110
	sta Reg5

; Reset fontcolor
	lda #0		; Black
	sta fontcol

; Copy MPAGD font (96 chars = 768 bytes) to CHAR RAM (CHARcode 0-95)

	ldx #0
@loop:
	lda font,x
	sta CharAddress,x
	lda font + $100,x
	sta CharAddress +$100,x
	lda font + $200,x
	sta CharAddress + $200,x
	inx
	bne @loop

; Copy MPAGD tiles to CHAR RAM starting at $1b00 (CHARcode 96-) 

; Set source = MPAGD tileaddress

	lda #<chgfx
	sta tileaddr
	lda #>chgfx
	sta tileaddr+1

; Set destination = CHAR RAM address

	lda #<(CharAddress + $300)
	sta bufaddr
	lda #>(CharAddress + $300)
	sta bufaddr+1

; Copy all MPAGD tiles into CHAR RAM

	ldy #0
@bloop:
	lda (tileaddr),y
	sta (bufaddr),y
	inc tileaddr
	bne :+
	inc tileaddr+1
:	inc bufaddr
	bne :+
	inc bufaddr+1
:	lda tileaddr
	cmp #<bprop		; Check for last tileaddress
	bne @bloop
	lda tileaddr+1
	cmp #>bprop
	bne @bloop

; Set up screen line offset address table.
; Save lb and hb of 24 lines in a table for reference

setsat:
	lda #0			; start
	sta scraddr
	sta scraddr+1

	ldy #0			; vertical lines on screen.
setsa0:
	lda scraddr
	sta SCROFF_lb,y		; write low byte.
	lda scraddr+1
	sta SCROFF_hb,y		; write high byte.
	jsr nline		; next line down.
	iny			; next position in table.
	cpy #ScrHeight
	bne setsa0
	rts

;----------------------------------------------------------------------
; CLS, Clear screen routine.
;
; Fill screenmem and colourmem  with space
;----------------------------------------------------------------------

cls:
	ldx #0
clsloop:
	lda #0
	sta ScreenAddr,x	; Screen RAM
	sta ScreenAddr+$100,x
	sta ScreenAddr+$200,x
	sta ColorAttr,x		; Colour RAM
	sta ColorAttr+$100,x
	sta ColorAttr+$200,x
	inx
	bne clsloop
	rts

;----------------------------------------------------------------------
; SETPAL, Set colour palette
;
; Not used at the moment 
;----------------------------------------------------------------------

setpal:
	rts

;----------------------------------------------------------------------
; PCHAR, Display character on screen
;----------------------------------------------------------------------

pchar:
	sec
	sbc #32
	sta fntaddr

; Set color attribute

	jsr gaadd
	lda fontcol
	ldy #0
	sta (bufaddr),y

; Display character

	jsr gprad		; get screen address.
pchar0:
	lda fntaddr
	ldy #0
	sta (scraddr),y 	; copy to screen.
	rts

;----------------------------------------------------------------------
; GETOFFSET, Calculate bufaddr = Y*screen width + X 
;----------------------------------------------------------------------

getoffset:
	ldy dispy
	lda SCROFF_hb,y
	sta bufaddr+1
	lda SCROFF_lb,y
	clc
	adc dispx
	sta bufaddr
	bcc :+
	inc bufaddr+1
:	rts

;-------------------------------------------------------------
; VSYNC, synchronize to 25 frames/sec
;
;  - read joystick/keyboard
;  - handle sound
;  - sync framerate with timer 25Hz
;  - handle shrapnel every even frame
;-------------------------------------------------------------

vsync:
	pha
	tya
	pha
	txa
	pha
	jsr joykey		; read joystick/keyboard.

	ldx $a2			; Set Timer1 expired bit
vsync1:
	cpx $a2			; Check Interrupt Flag Register
;	beq vsync1

	lda clock
	and #1
	bne:+
	jsr proshr		; handle shrapnel every even frame
:
	lda sndtyp
	beq sndskip
sndloop:
	lda SpeakerBit		; handle sound
	ldy sndtyp
sndwait:
	dey
	bne sndwait
	eor #4
;	sta SpeakerBit
	dec sndtyp
	bne sndloop
sndskip: 
	pla
	tax
	pla
	tay
	pla
	rts

;-------------------------------------------------------------
; KTEST, scan key and return in carry if key pressed
;
; Keyboard test routine.
;
; Input:
;  A = key to read, high nibble=col and low nibble=row
;
; Output:
;  carry clr = key pressed
;  carry set = key not pressed
;--------------------------------------------------------

ktest:
	sta z80_a		; save key

	lsr a			; set row
	lsr a
	lsr a
	lsr a
	tax
	lda colmask,x
	sta KeyColAddr

	lda z80_a
	and #$0f
	tax

	lda KeyRowAddr		; read column
	and keymask,x		; check key pressed
	beq pressed
	sec			; key not pressed
	rts
pressed:
	clc			; key pressed
	rts

colmask:	.byte 127,191,223,239,247,251,253,254
keymask:	.byte $01,$02,$04,$08,$10,$20,$40,$80

;-------------------------------------------------------------
; JOY1SCAN, scan joystick1 and return key in z80_e
;-------------------------------------------------------------

joy1scan:
	lda #$ff
	sta z80_e

	lda JoyAddr1
	lsr a
	lsr a

	lsr a
	rol z80_e	; Up
	lsr a
	rol z80_e	; Down
	lsr a
	rol z80_e	; Left

	lda #%01111111
	sta DDRB	; Bit7 = input joystick b7
	lda JoyAddr2
	asl a
	rol z80_e	; Right
	lda #%11111111
	sta DDRB	; Reset  

	lda JoyAddr1
	and #%00100000
	bne :+
	lda z80_e
	and #%11101111	; Fire
	sta z80_e

	lda keys+5	; Fire2
	jsr ktest
	bcs :+
	lda z80_e
	and #%11011111
	sta z80_e
:
	lda keys+6	; Fire3
	jsr ktest
	bcs :+
	lda z80_e
	and #%10111111
	sta z80_e
:
	rts

;-------------------------------------------------------------
; JOY2SCAN, scan joystick2 and return key in z80_e
;-------------------------------------------------------------

joy2scan:
	rts

;-------------------------------------------------------------
; READKEY, Wait for keypress
;-------------------------------------------------------------

READKEY:
	lda #0
	sta KeyColAddr
	lda KeyRowAddr
	cmp #$ff
	beq READKEY
	rts

