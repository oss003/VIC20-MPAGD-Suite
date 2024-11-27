;
; Code equates
;
L0006       = $0006
L0007       = $0007
L0012       = $0012
L0013       = $0013
L0016       = $0016
L0017       = $0017
L0018       = $0018
L0019       = $0019
L001A       = $001A
L0024       = $0024
L0030       = $0030
L0031       = $0031
L0032       = $0032
L0033       = $0033
L0038       = $0038
L003A       = $003A
L003B       = $003B
L003C       = $003C
L0044       = $0044
L0047       = $0047
L004B       = $004B
L0085       = $0085
L0093       = $0093
L0095       = $0095
L00A9       = $00A9
L00AE       = $00AE
L00AF       = $00AF
L00B7       = $00B7		; Nr chars in filename
L00B9       = $00B9		; Current secondary address
L00BA       = $00BA		; Current device
L00BB       = $00BB		; Get char filename

L00C3       = $00C3		; KERNAL setup pointer
L00C4       = $00C4

; User space

L00FB       = $00FB
L00FC       = $00FC
L00FD       = $00FD
L00FE       = $00FE
L00FF       = $00FF

L0100       = $0100

L0300       = $0300
L0301       = $0301
L0330       = $0330
L0331       = $0331

L050E       = $050E
L0562       = $0562
L0585       = $0585
L05BA       = $05BA

L063A       = $063A
L0640       = $0640
L067C       = $067C
L0691       = $0691

L1800       = $1800
L1805       = $1805

L1C00       = $1C00
L1C01       = $1C01
L1C0C       = $1C0C

L9111       = $9111

LC1C8       = $C1C8
LD042       = $D042
LD945       = $D945

LE742       = $E742		; Print character
LF549       = $F549
LF5E9       = $F5E9
LF8E0       = $F8E0
LF934       = $F934

LFF93       = $FF93		; SECOND send second address after listen
LFFA8       = $FFA8		; CIOUT output byte to serial
LFFAE       = $FFAE		; UNLSN unlisten
LFFB1       = $FFB1		; LISTEN serial
;
; Start of code
;
;           *= $A400
;

;-------------------------------------------------
; Entry fastloader
;-------------------------------------------------

start:
            clc
            bcc LA41B
	
; Reset LOAD vector

            lda #$49
            sta L0330
            lda #$F5
            sta L0331

; Print text TURBODISK DISABLED

            ldy #$00
LA40F:      lda LA429,Y
            beq LA41A
            jsr LE742		; Print character
            iny
            bne LA40F
LA41A:      rts

; Set fastload LOAD vector

LA41B:
            lda #<LA454
            sta L0330
            lda #>LA454
            sta L0331

; Print text TURBODISK ACTIVATED

            ldy #$15
            bne LA40F

; Message text

LA429:      .byte $0D,"TURBODISK DISABLED",$0D,$00
LA43F:      .byte $0D,"TURBODISK ACTIVATED",$0D,$00


;-------------------------------------------------
; Fastloader LOAD entry
;-------------------------------------------------

LA454:      sta L0093
            lda L0093
            bne LA478

; Check for LOAD"$"

            ldy #$00
            lda (L00BB),Y	; Get char filename
            cmp #$24
            beq LA478

            ldx #$10
            lda #$A0
LA466:      sta LA794,X
            dex
            bpl LA466

; Copy filename in buffer

LA46C:      lda (L00BB),Y	; Get char filename
            sta LA794,Y
            iny
            cpy L00B7		; Nr chars in filename
            bcc LA46C
;            bcs LA47D
	    bcs LA4A2

LA478:      lda L0093
            jmp LF549

; Print text TURBOLOADING...

LA47D:
            ldy #$00
LA47F:      lda LA48A,Y
            beq LA4A2
            jsr LE742		; Print character
            iny
            bne LA47F

LA48A:      .byte $0D,"TURBOLOADING...",$0D,$00

LA49C:      .byte "M-W"
LA49F:      .byte $00
LA4A0:      .byte $00,$20

LA4A2:      lda #$10
            sta L00FF

            lda #<LA600
            sta L00FB
            lda #>LA600
            sta L00FC

            lda #$00
            sta L00FD
            lda #$05
            sta L00FE

LA4B6:      lda L00BA		; Current device
            jsr LFFB1		; LISTEN serial

            lda #$6F		; Serial byte
            jsr LFF93		; SECOND send second address after listen
            lda L00FD
            ldy L00FE
            sta LA49F		; Set tens in M-Wxx
            sty LA4A0		; Set units in M-Wxx

; Print text M-Wxx

            ldy #$00
LA4CC:      lda LA49C,Y
            jsr LFFA8		; CIOUT output byte to serial
            iny
            cpy #$06
            bne LA4CC

            ldy #$00
LA4D9:      lda (L00FB),Y
            jsr LFFA8		; CIOUT output byte to serial
            iny
            cpy #$20
            bcc LA4D9

            lda L00FB
            adc #$1F
            sta L00FB
            lda L00FC
            adc #$00
            sta L00FC

            lda L00FD
            adc #$20
            sta L00FD
            lda L00FE
            adc #$00
            sta L00FE

            jsr LFFAE		; UNLSN unlisten
            dec L00FF
            bne LA4B6

            lda L00BA		; Current device
            jsr LFFB1		; LISTEN serial

            lda #$6F		; Serial byte
            jsr LFF93		; SECOND send second address after listen

            lda #$55
            jsr LFFA8		; CIOUT output byte to serial

            lda #$43
            jsr LFFA8		; CIOUT output byte to serial
            jsr LFFAE		; UNLSN unlisten

            sei
            jsr LA589
            bit LA800
            bmi LA567

            ldy L00C3		; KERNAL setup pointer
            ldx L00C4
            lda L00B9		; Current secondary address
            beq LA530

            ldy LA802
            ldx LA803
LA530:      sty L00AE
            stx L00AF

            ldx #$04
            jsr LA571
LA539:      jsr LA589
            lda LA800
            bmi LA56A
            beq LA549
            jsr LA56F
            clc
            bcc LA539

LA549:      ldx #$02
            ldy #$00
LA54D:      lda LA800,X
            sta (L00AE),Y
; 	jsr setborder
            iny
            inx
            beq LA55D
            cpx LA801
            bcc LA54D
            beq LA54D

LA55D:      jsr LA57C

            clc
LA561:      ldx L00AE
            ldy L00AF
            cli
            rts

LA567:      .byte $a9,$04,$2c

LA56A:      lda #$00
            sec
            bcs LA561
LA56F:      ldx #$02

LA571:      ldy #$00
LA573:      lda LA800,X
            sta (L00AE),Y
;	jsr setborder
            iny
            inx
            bne LA573

LA57C:      clc
            tya
            adc L00AE
            sta L00AE
            lda L00AF
            adc #$00
            sta L00AF
            rts

LA589:      ldy #$00

LA58B:      lda #$80
            sta L9111
LA590:      lda L9111
            and #$02
            beq LA590

            lda #$00
            sta L9111

; Wait loop

            ldx #$07
LA59E:      dex
            nop
            bne LA59E

; Wait loop

            ldx #$04
LA5A4:      lda L9111
            lsr A
            rol L0095
            lsr A
            rol L0095
            nop
            nop
            dex
            bne LA5A4

            lda L0095
            eor #$FF
            sta LA800,Y
            iny
            bne LA58B
            rts

;setborder:
;	sta (L00AE),Y
;	and #7
;	sta tmp
;	lda $900f
;	and #7
;	ora tmp
;	sta $900f
;	rts
;
;tmp:	.byte 0

            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00

	    .byte $00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00

LA600:
            jsr LD042

            sei
            lda #$12
            ldy #$01
            sta L0300
            sty L0301
            jsr L05BA
            lda #$03
            sta L003C
            ldx #$00
            stx L004B
            beq LA644
LA61B:      ldy #$00
            lda (L003B),Y
            cmp #$82
            bne LA63C
            iny
            iny
            iny
LA626:      lda L0691,Y
            cmp #$2A
            beq LA66A
            cmp #$3F
            beq LA635
            cmp (L003B),Y
            bne LA63C
LA635:      iny
            cpy #$12
            beq LA66A
            bne LA626
LA63C:      inc L004B
            ldx L004B
            cpx #$08
            beq LA64B
LA644:      lda L0562,X
            sta L003B
            bne LA61B
LA64B:      lda L0300
            beq LA656
            ldy L0301
            jmp L050E
LA656:      lda #$FF
            sta L0300
            jsr L0585
            cli
            jmp LD945

            .byte $02,$22,$42,$62,$82,$A2,$C2,$E2

LA66A:      inc L003B
            ldy #$00
            lda (L003B),Y
            sta L0300
            iny
            lda (L003B),Y
            sta L0301
LA679:      jsr L05BA
            jsr L0585
            lda L0300
            bne LA679
            rts

            ldy #$00
LA687:      lda L0300,Y
            sta L0085
LA68C:      bit L1800
            bpl LA68C
            lda #$10
            sta L1800
LA696:      bit L1800
            bmi LA696
            ldx #$04
LA69D:      lda #$00
            asl L0085
            rol A
            asl A
            asl L0085
            rol A
            asl A
            sta L1800
            dex
            bne LA69D
            pha
            pla
            pha
            pla
            lda #$0F
            sta L1800
            iny
            bne LA687
            rts

            ldy L0301
            sty L0007
            lda L0300
            cmp L0006
            php
            sta L0006
            plp
            beq LA6DA
            lda #$B0
            sta $00
            cli
LA6CF:      bit $00
            bmi LA6CF
            sei
            lda $00
            cmp #$01
            bne LA728
LA6DA:      lda #$EE
            sta L1C0C
            lda #$06
            sta L0032
            lda #$00
            sta L0033
            sta L0030
            lda #$03
            sta L0031
            jsr L063A
LA6F0:      bvc LA6F0
            clv
            lda L1C01
            sta L0300,Y
            iny
            bne LA6F0

            ldy #$BA
LA6FE:      bvc LA6FE
            clv
            lda L1C01
            sta L0100,Y
            iny
            bne LA6FE
            jsr LF8E0
            lda L0038
            cmp L0047
            beq LA717
            lda #$22
            bne LA72B
LA717:      jsr LF5E9
            cmp L003A
            beq LA722
            lda #$23
            bne LA72B
LA722:      lda #$EC
            sta L1C0C
            rts

LA728:      clc
            adc #$18
LA72B:      sta L0044
            lda #$FF
            sta L0300
            jsr L0585
            lda L0044
            jmp LC1C8
            jsr L0640
            jmp L067C
            lda L0012
            sta L0016
            lda L0013
            sta L0017
            lda L0006
            sta L0018
            lda L0007
            sta L0019
            lda #$00
            eor L0016
            eor L0017
            eor L0018
            eor L0019
            sta L001A
            jsr LF934
            ldx #$5A
LA761:      jsr L067C
LA764:      bvc LA764
            clv
            lda L1C01
            cmp L0024,Y
            bne LA775
            iny
            cpy #$08
            bne LA764
            rts

LA775:      dex
            bne LA761
            lda #$20
            bne LA72B
            lda #$D0
            sta L1805
            lda #$21
LA783:      bit L1805
            bpl LA72B
            bit L1C00
            bmi LA783
            lda L1C01
            clv
            ldy #$00
            rts

; Load data

LA794:      .byte $A0,$A0,$A0,$A0,$A0,$A0,$A0,$A0
            .byte $A0,$A0,$A0,$A0,$A0,$A0,$A0,$A0
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00

;-------------------------------------------------
; 256 bytes buffer
;-------------------------------------------------

LA800:      .byte $00
LA801:      .byte $00
LA802:      .byte $00
LA803:      .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00,$00,$00,$00
            .byte $00,$00,$00,$00,$00
