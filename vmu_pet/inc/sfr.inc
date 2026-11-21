;; Moemon VMU pet: the LC86K87's special function registers (Sega's VMU hardware manual; libevmu's
;; evmu_address_space.h has the same addresses).

acc    = $100
psw    = $101
b      = $102
c      = $103
trl    = $104
trh    = $105
sp     = $106
pcon   = $107
ie     = $108
ip     = $109
ext    = $10d
ocr    = $10e
t0con  = $110
t0prr  = $111
t0l    = $112
t0lr   = $113
t0h    = $114
t0hr   = $115
t1cnt  = $118
t1lc   = $11a
t1l    = $11b
t1lr   = $11b
t1hc   = $11c
t1h    = $11d
t1hr   = $11d
mcr    = $120
stad   = $122
cnr    = $123
tdr    = $124
xbnk   = $125
vccr   = $127
scon0  = $130
sbuf0  = $131
sbr    = $132
scon1  = $134
sbuf1  = $135
p1     = $144
p1ddr  = $145
p1fcr  = $146
p3     = $14c
p3ddr  = $14d
p3int  = $14e
fpr    = $154
p7     = $15c
i01cr  = $15d
i23cr  = $15e
isl    = $15f
btcr   = $17f

;; XRAM (the LCD) when xbnk is 0 or 1: $180-$1fb
xram   = $180

;; OCR values (Sega's samples)
OSC_RC   = $4d      ; internal RC, 1/12
OSC_RCFW = $cd      ; internal RC, 1/6: flash writes
OSC_XT   = $ef      ; the 32 kHz crystal

;; RAM bank 0 / bank 1 system addresses
LOW_BATT_CHK = $6e  ; bank 0: $ff = don't detect a low battery
FMFLAG  = $7c       ; bank 1: flash write end detection
FMBANK  = $7d       ; bank 1: flash bank of the page
FMADD_H = $7e       ; bank 1: flash address of the page
FMADD_L = $7f
FMBUFF  = $80       ; bank 1: the 128-byte page buffer, $80-$ff

;; P3 bits (0 = pressed)
BTN_UP    = 0
BTN_DOWN  = 1
BTN_LEFT  = 2
BTN_RIGHT = 3
BTN_A     = 4
BTN_B     = 5
BTN_MODE  = 6
BTN_SLEEP = 7
