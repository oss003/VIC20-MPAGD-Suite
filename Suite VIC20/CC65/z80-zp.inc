;ws	 = $60

z80_f: .res 1;	 = ws+$00
z80_a: .res 1;	 = ws+$01
z80_af	 = z80_f

z80_c: .res 1;	 = ws+$02
z80_b: .res 1;	 = ws+$03
z80_bc	 = z80_c

z80_e: .res 1;	 = ws+$04
z80_d: .res 1;	 = ws+$05
z80_de	 = z80_e

z80_l: .res 1;	 = ws+$06
z80_h: .res 1;	 = ws+$07
z80_hl	 = z80_l

z80_x: .res 1;    = ws+$08
z80_i: .res 1;    = ws+$09
z80_ix	 = z80_x

z80_iy: .res 2;	 = ws+$0a

z80_fp: .res 1;	 = ws+$0c
z80_ap: .res 1;	 = ws+$0d

z80_cp: .res 1;	 = ws+$0e
z80_bp: .res 1;	 = ws+$0f
z80_bcp	 = z80_cp

z80_ep: .res 1;	 = ws+$10
z80_dp: .res 1;	 = ws+$11
z80_dep	 = z80_ep

z80_lp: .res 1;	 = ws+$12
z80_hp: .res 1;	 = ws+$13
z80_hlp	 = z80_lp

z80_sp: .res 2;   = ws+$14

z80_reg0: .res 1; = ws+$16
z80_reg1: .res 1; = ws+$17
z80_reg2: .res 1; = ws+$18
z80_reg3: .res 1; = ws+$19

;z80_r: .res 1;	 = ws+$1a
