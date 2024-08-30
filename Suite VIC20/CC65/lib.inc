;============================================================
; PLATFORM VARIABLES
;============================================================
;
; Memory map:
;
;	$0000-$03ff	 1.0kB	CPU WS
;	$0400-$0fff	 3,0kB	RAM
;	$1000-$17ff	 1,0kB	VIDEO RAM
;	$1400-$1Bff	 2,0kB	CHAR RAM
;	$1C00-$7fff	25,0kB	RAM
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
;               ASCII   ??  ??  45  48  56  54  52  50
;                 VIC   07  06  05  04  03  02  01  00
;
; 	191-1011 1111 	F5       @   O   U   T   E   Q
;               ASCII   ??  ??  64  79  85  84  69  81
;                 VIC   17  16  15  14  13  12  11  10
;
; 	223-1101 1111 	F3   =   :   K   H   F   S  Com
;               ASCII   ??  61  58  75  72  70  83  ??
;                 VIC   27  26  25  24  23  22  21  20
;
; 	239-1110 1111 	F1  RSh  .   M   B   C   Z  Spc
;               ASCII   ??  ??  46  77  66  67  90  32
;                 VIC   37  36  35  34  33  32  31  30
;
; 	247-1111 0111	Cur  /   ,   N   V   X  LSh Run
;               ASCII   ??  47  44  78  86  88  ??  ??
;                 VIC   47  46  45  44  43  42  41  40
;
; 	251-1111 1011	Cur  ;   L   J   G   D   A  Ctr
;               ASCII   ??  59  76  74  71  68  65  ??
;                 VIC   57  56  55  54  53  52  51  50
;
; 	253-1111 1101	Ret  *   P   I   Y   R   W
;               ASCII   13  42  80  73  89  82  87
;                 VIC   67  66  65  64  63  62  61  60
;
; 	254-1111 1110	Del Pnd  +   9   7   5   3   1
;               ASCII   08  ??  43  57  55  53  51  49
;                 VIC   77  76  75  74  73  72  71  70
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

; VIC constants

	OBJCHAR 	= 178		; first char nr for objects

; VIC variables:

	fontcol:	.byte 0		; Frontcolour text

; Addresses

	MapAddr		= $400		; RAM
	ScreenAddr	= $1000		; VIDEO RAM
	CharAddress	= $1400		; CHAR RAM
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
	Timer1_Low  	= $9114		; Timer 1 low byte
	Timer1_High 	= $9115		; Timer 1 high byte
	Timer2_Low  	= $9118		; Timer 2 low byte
	Timer2_High 	= $9119		; Timer 2 high byte
	AUX		= $911b		; Auxiliary control register
	IFR		= $911d		; Interrupt Flag Register
	IER		= $911e		; Interrupt Enable Register
	ColorAttr	= $9400		; Color attributes

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

; max num of columns 24
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
	ora #%1101
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

; Init VIA timer for timeout 1/25 sec

	lda #$e0		; T1 continuous
	sta AUX
	lda #$7f		; No int
	sta IER
	lda #0
	sta IFR			; Clear interrupt flags
	lda #<timervalue	; Timer 1/25 sec
	sta Timer1_Low
	lda #>timervalue
	sta Timer1_High

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
	sbc #32			; Convert ASCII to MPAGD code
	sta fntaddr

; Set color attribute

	jsr gaadd		; Calculate color attribute address
	lda fontcol		; Get fontcolour
	ldy #0
	sta (bufaddr),y		; Colour character

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

	lda #$40		; Set Timer1 expired bit
vsync1:
	bit IFR			; Check Interrupt Flag Register
	beq vsync1
	lda Timer1_Low		; Clear Timer interrupt

	lda clock
	and #1
	bne:+
	jsr proshr		; handle shrapnel every even frame
:
	lda sndtyp
	beq sndskip
sndloop:
;	lda SpeakerBit		; handle sound
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
;  A = key to read, high nibble=row and low nibble=col
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

;---------------------------------------------------------------
; Getkey in column,row format
;
; Output:
;  A = high nibble=row and low nibble=col key in matrix
;---------------------------------------------------------------
kget:
	stx xtmp		; Save x-reg
kget0:
	ldx #0			; reset row	
kget1:
	lda colmask,x		; Get mask
	sta KeyColAddr		; Set row
	lda KeyRowAddr		; Read columns
	sta tmp			; Save colums
	cmp #$ff		; Key pressed?
	bne kget2		; Yes, skip
	inx			; Next row
	cpx #8			; All rows scanned?
	bne kget1		; No, repeat
	jmp kget0		; No key pressed, a=$ff
kget2:
	ldy #0			; Reset column
kget3:
	lsr tmp			; Test bit
	bcc kget4		; If clear, key pressed
	iny			; Next column
	jmp kget3		; Repeat
kget4:
	stx tmp			; Get row
	asl tmp			; Make high nibble
	asl tmp
	asl tmp
	asl tmp
	tya			; Get column
	clc
	adc tmp			; Add row as low nibble
	sta tmp
kget5:
	lda KeyRowAddr		; Read columns
	cmp #$ff
	bne kget5

	lda tmp			; Return keycode
	ldx xtmp		; Restore X-reg
	rts			; Return

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
:
	lda keys+6	; Fire3
	jsr ktest
	bcs :+
	lda z80_e
	and #%10111111
	sta z80_e
:
	lda keys+5	; Fire2
	jsr ktest
	bcs :+
	lda z80_e
	and #%11011111
	sta z80_e
:

	rts

;-------------------------------------------------------------
; JOY2SCAN, scan joystick2 and return key in z80_e
;-------------------------------------------------------------

joy2scan:
	rts

;-------------------------------------------------------------
; READKEY, Wait for keypress and then wait for release
;-------------------------------------------------------------

READKEY:
	lda #$ff
	sta tmp
	ldx #7
scanrow:
	lda colmask,x
	sta KeyColAddr
	lda KeyRowAddr		; read column
	and tmp
	sta tmp
	dex
	bpl scanrow
	lda tmp
	tay
	rts

;----------------------------------------------------
; Delay routine 1/50 sec
;
; Wait 1/60 sec = 16666 usec
; Wait 208 x 16 =  3328 usec
;                 19994 usec
; rts           =     6 usec
; Total         = 20000 usec
;----------------------------------------------------

delay:
	sta xtmp
del_loop:
	lda $9004
	bne del_loop			; wait for flyback (1/60 sec)

	ldy #208			; wait 208 x 16 = 3328 usec
delay1:
	adc ($80,x)			;	 6 usec
	adc ($80,x)			;	 6 usec
	dey				;	 2 usec
	bne delay1			;	 2 usec
	nop				; 2 usec
	nop				; 2 usec
					; tot: 20000 usec
	dec xtmp
	bne del_loop
	rts

;-------------------------------------------------------------
; Object routines
;-------------------------------------------------------------

; Clear all 6 Object slots

clrobjslots:
	ldx #5
	lda #$ff			; Mark slot is free
clrobjloop:
	sta objslots,x
	dex
	bpl clrobjloop
	rts

; Find a free Object slot and save obj nr in it

findfreeslot:
	ldx #0
findobjloop:
	lda objslots,x			; Get slot vale
	bmi slotfound			; Jump if free
	inx				; Point to next slot
	cpx #6
	bne findobjloop			; Repeat
	sec				; No slot free, set carry
	rts
slotfound:
	lda objnr			; get object nr
	sta objslots,x			; store in slot
	stx curobjslot
	clc				; Slot set, clear carry
	rts

; Copy object image data to char RAM
; z80_hl = object data address

copyobj2chr:
	ldx curobjslot
	lda objaddr_lb,x		; Calculate char RAM address	
	sta bufaddr
	lda objaddr_hb,x
	sta bufaddr+1

	ldy #31
copyloop:
	lda (z80_hl),y			; get object data
	sta (bufaddr),y			; store in char RAM
	dey
	bpl copyloop			; repeat for 32 bytes
	rts

; Display object
; curobjslot = slot pointer

displayobj:
	lda curobjslot
	asl a				; calculate object char nr
	asl a
	clc
	adc #OBJCHAR
	sta charnr			; charnr = slotptr*4 + 178

	jsr getoffset			; Calculate screen address
	lda bufaddr
	sta scraddr
	clc
	lda bufaddr+1
	adc #>ScreenAddr
	sta scraddr+1

; Save background char and display object on screen

	ldx curobjslot			; Set buffer offset pointer

	ldy #0
	lda (scraddr),y			; get screen char top left
	sta bufchar1,x			; save background char
	lda charnr			; get object char
	sta (scraddr),y			; display on screen

	iny
	inc charnr
	lda (scraddr),y			; get screen char 
	sta bufchar2,x			; save background char
	lda charnr			; get object char
	sta (scraddr),y			; display on screen

	ldy #24
	inc charnr
	lda (scraddr),y			; get screen char 
	sta bufchar3,x			; save background char
	lda charnr			; get object char
	sta (scraddr),y			; display on screen

	iny
	inc charnr
	lda (scraddr),y			; get screen char 
	sta bufchar4,x			; save background char
	lda charnr			; get object char
	sta (scraddr),y			; display on screen

; Save background colours and set object colour on screen

	clc				; Point to Color RAM
	lda scraddr+1
	adc #>(ColorAttr-ScreenAddr)
	sta scraddr+1

	ldy #0
	lda (scraddr),y			; get colour top left
	sta bufcol1,x			; save background colour
	lda objcol
	sta (scraddr),y			; set object colour

	iny
	lda (scraddr),y			; get colour top right
	sta bufcol2,x			; save background colour
	lda objcol
	sta (scraddr),y			; set object colour 

	ldy #24
	lda (scraddr),y			; get colour bottom left
	sta bufcol3,x			; save background colour
	lda objcol
	sta (scraddr),y			; set object colour

	iny
	lda (scraddr),y			; get colour bottom right
	sta bufcol4,x			; save background colour
	lda objcol
	sta (scraddr),y			; set object colour

	rts

; Erase sprite
; TODO -> RESTORE BACKGROUND

eraseobj:
	jsr getoffset			; Calculate screen address
	lda bufaddr
	sta scraddr
	clc
	lda bufaddr+1
	adc #>ScreenAddr
	sta scraddr+1

; Find slotnr

	ldy #0
	lda (scraddr),y			; erase top left
	sec
	sbc #OBJCHAR
	lsr a
	lsr a
	tax

; Erase object from screen and restore background 

	lda bufchar1,x
	sta (scraddr),y			; erase top left
	iny
	lda bufchar2,x
	sta (scraddr),y			; erase top right
	ldy #24
	lda bufchar3,x
	sta (scraddr),y			; erase bottom left
	iny
	lda bufchar4,x
	sta (scraddr),y			; erase bottom right

; Restore background colour

	clc
	lda scraddr+1
	adc #>(ColorAttr-ScreenAddr)
	sta scraddr+1

	ldy #0
	lda bufcol1,x
	sta (scraddr),y			; erase top left
	iny
	lda bufcol2,x
	sta (scraddr),y			; erase top right
	ldy #24
	lda bufcol3,x
	sta (scraddr),y			; erase bottom left
	iny
	lda bufcol4,x
	sta (scraddr),y			; erase bottom right

	rts

; Object slots for objects on screen

objnr:		.byte 0
charnr:		.byte 0

curobjslot:	.byte 0
objslots:	.byte 0,0,0,0,0,0

bufchar1:	.byte 0,0,0,0,0,0
bufchar2:	.byte 0,0,0,0,0,0
bufchar3:	.byte 0,0,0,0,0,0
bufchar4:	.byte 0,0,0,0,0,0

bufcol1:	.byte 0,0,0,0,0,0
bufcol2:	.byte 0,0,0,0,0,0
bufcol3:	.byte 0,0,0,0,0,0
bufcol4:	.byte 0,0,0,0,0,0

; Calculate object char locations for data storage

objaddr_lb:	
	.repeat 6,i
		.byte <(CharAddress + (OBJCHAR*8 + i*32))
	.endrepeat
objaddr_hb:	
	.repeat 6,i
		.byte >(CharAddress + (OBJCHAR*8 + i*32))
	.endrepeat

objcol:	.byte 0
