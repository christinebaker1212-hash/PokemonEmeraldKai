[Remap]
x = x
y = y
z = z
a = a
b = b
c = c
s = s

;-------------------------------------------------------------------
[Command]
name = "AI_1"
command = D, D, D, D, D, D, D, D, D, D, D, D, D, D, D
time = 1

[Command]
name = "AI_2"
command = F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
time = 1

[Command]
name = "AI_3"
command = U, U, U, U, U, U, U, U, U, U, U, U, U, U, U
time = 1

[Command]
name = "AI_4"
command = B, B, B, B, B, B, B, B, B, B, B, B, B, B, B
time = 1
 
[Command]
name = "AI_5"
command = a, a, a, a, a, a, a, a, a, a, a, a, a, a, a
time = 1

[Command]
name = "AI_6"
command = b, b, b, b, b, b, b, b, b, b, b, b, b, b, b
time = 1

[Command]
name = "AI_7"
command = c, c, c, c, c, c, c, c, c, c, c, c, c, c, c
time = 1

[Command]
name = "AI_8"
command = x, x, x, x, x, x, x, x, x, x, x, x, x, x, x
time = 1

[Command]
name = "AI_9"
command = y, y, y, y, y, y, y, y, y, y, y, y, y, y, y
time = 1

[Command]
name = "AI_10"
command = z, z, z, z, z, z, z, z, z, z, z, z, z, z, z
time = 1

;-| Default Values |-------------------------------------------------------
[Defaults]
; Default value for the "time" parameter of a Command. Minimum 1.
command.time = 30

; Default value for the "buffer.time" parameter of a Command. Minimum 1,
; maximum 30.
command.buffer.time = 1

[Command]
name = "DIZZY_RECOVERY"
command = F,B
time = 18

[Command]
name = "Throw_Prolonge"
command = F,B,F,B,F
time = 20

;-| Special |-----------------------------------------------------------

[Command]
Name = "RisingH_L"
Command = F, ~D, DF, x
Time = 22

[Command]
Name = "RisingH_M"
Command = F, ~D, DF, y
Time = 22

[Command]
Name = "RisingH_S"
Command = F, ~D, DF, z
Time = 22

[Command]
Name = "RisingH_L"
Command = F, ~D, DF, ~x
Time = 22

[Command]
Name = "RisingH_M"
Command = F, ~D, DF, ~y
Time = 22

[Command]
Name = "RisingH_S"
Command = F, ~D, DF, ~z
Time = 22

[Command]
Name = "Condor_D"
Command = x+y+z
Time = 2

[Command]
Name = "Condor_x"
Command = ~x,~y,~z
Time = 3

[Command]
Name = "Condor_y"
Command = ~y,~z,~x
Time = 3

[Command]
Name = "Condor_z"
Command = ~z,~x,~y
Time = 3

[Command]
name = "Mex-Typhoon_L"
command = ~F, D, B, U, x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~F, D, B, U, ~x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~D, B, U, F, x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~D, B, U, F, ~x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~B, U, F, D, x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~B, U, F, D, ~x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~U, F, D, B, x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~U, F, D, B, ~x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~F, U, B, D, x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~F, U, B, D, ~x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~D, F, U, B, x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~D, F, U, B, ~x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~B, D, F, U, x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~B, D, F, U, ~x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~U, B, D, F, x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~U, B, D, F, ~x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~B, D, F, B, x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~B, D, F, B, ~x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~F, D, B, F, x
time = 50

[Command]
name = "Mex-Typhoon_L"
command = ~F, D, B, F, ~x
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~F, D, B, U, F, y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~F, D, B, U, F, ~y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~D, B, U, F, y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~D, B, U, F, ~y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~B, U, F, D, y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~B, U, F, D, ~y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~U, F, D, B, y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~U, F, D, B, ~y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~F, U, B, D, y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~F, U, B, D, ~y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~D, F, U, B, y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~D, F, U, B, ~y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~B, D, F, U, y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~B, D, F, U, ~y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~U, B, D, F, y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~U, B, D, F,  ~y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~B, D, F, B, y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~B, D, F, B, ~y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~F, D, B, F, y
time = 50

[Command]
name = "Mex-Typhoon_M"
command = ~F, D, B, F, ~y
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~F, D, B, U, z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~F, D, B, U, ~z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~D, B, U, F, z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~D, B, U, F, ~z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~B, U, F, D, z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~B, U, F, D, ~z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~U, F, D, B, z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~U, F, D, B, ~z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~F, U, B, D, z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~F, U, B, D, ~z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~D, F, U, B, z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~D, F, U, B, ~z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~B, D, F, U, z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~B, D, F, U, ~z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~U, B, D, F, z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~U, B, D, F, ~z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~B, D, F, B, z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~B, D, F, B, ~z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~F, D, B, F, z
time = 50

[Command]
name = "Mex-Typhoon_S"
command = ~F, D, B, F, ~z
time = 50

[Command]
name = "R-Typhoon"
command = ~F, D, B, U, F, D, B, U, x
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, U, F, D, B, U, ~x
time = 90

[Command]
name = "R-Typhoon"
command = ~D, B, U, F, D, B, U, F, x
time = 90

[Command]
name = "R-Typhoon"
command = ~D, B, U, F, D, B, U, F, ~x
time = 90

[Command]
name = "R-Typhoon"
command = ~B, U, F, D, B, U, F, D, x
time = 90

[Command]
name = "R-Typhoon"
command = ~B, U, F, D, B, U, F, D, ~x
time = 90

[Command]
name = "R-Typhoon"
command = ~U, F, D, B, U, F, D, B, x
time = 90

[Command]
name = "R-Typhoon"
command = ~U, F, D, B, U, F, D, B, ~x
time = 90

[Command]
name = "R-Typhoon"
command = ~F, U, B, D, F, U, B, D, x
time = 90

[Command]
name = "R-Typhoon"
command = ~F, U, B, D, F, U, B, D, ~x
time = 90

[Command]
name = "R-Typhoon"
command = ~D, F, U, B, D, F, U, B, x
time = 90

[Command]
name = "R-Typhoon"
command = ~D, F, U, B, D, F, U, B, ~x
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, U, B, D, F, U, x
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, U, B, D, F, U, ~x
time = 90

[Command]
name = "R-Typhoon"
command = ~U, B, D, F, U, B, D, F, x
time = 90

[Command]
name = "R-Typhoon"
command = ~U, B, D, F, U, B, D, F, ~x
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, B, D, F, B, x
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, B, D, F, B, ~x
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, F, D, B, F, x
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, F, D, B, F, ~x
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, U, F, D, B, U, y
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, U, F, D, B, U, ~y
time = 90

[Command]
name = "R-Typhoon"
command = ~D, B, U, F, D, B, U, F, y
time = 90

[Command]
name = "R-Typhoon"
command = ~D, B, U, F, D, B, U, F, ~y
time = 90

[Command]
name = "R-Typhoon"
command = ~B, U, F, D, B, U, F, D, y
time = 90

[Command]
name = "R-Typhoon"
command = ~B, U, F, D, B, U, F, D, ~y
time = 90

[Command]
name = "R-Typhoon"
command = ~U, F, D, B, U, F, D, B, y
time = 90

[Command]
name = "R-Typhoon"
command = ~U, F, D, B, U, F, D, B, ~y
time = 90

[Command]
name = "R-Typhoon"
command = ~F, U, B, D, F, U, B, D, y
time = 90

[Command]
name = "R-Typhoon"
command = ~F, U, B, D, F, U, B, D, ~y
time = 90

[Command]
name = "R-Typhoon"
command = ~D, F, U, B, D, F, U, B, y
time = 90

[Command]
name = "R-Typhoon"
command = ~D, F, U, B, D, F, U, B, ~y
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, U, B, D, F, U, y
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, U, B, D, F, U, ~y
time = 90

[Command]
name = "R-Typhoon"
command = ~U, B, D, F, U, B, D, F, y
time = 90

[Command]
name = "R-Typhoon"
command = ~U, B, D, F, U, B, D, F, ~y
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, B, D, F, B, y
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, B, D, F, B, ~y
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, F, D, B, F, y
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, F, D, B, F, ~y
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, U, F, D, B, U, z
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, U, F, D, B, U, ~z
time = 90

[Command]
name = "R-Typhoon"
command = ~D, B, U, F, D, B, U, F, z
time = 90

[Command]
name = "R-Typhoon"
command = ~D, B, U, F, D, B, U, F, ~z
time = 90

[Command]
name = "R-Typhoon"
command = ~B, U, F, D, B, U, F, D, z
time = 90

[Command]
name = "R-Typhoon"
command = ~B, U, F, D, B, U, F, D, ~z
time = 90

[Command]
name = "R-Typhoon"
command = ~U, F, D, B, U, F, D, B, z
time = 90

[Command]
name = "R-Typhoon"
command = ~U, F, D, B, U, F, D, B, ~z
time = 90

[Command]
name = "R-Typhoon"
command = ~F, U, B, D, F, U, B, D, z
time = 90

[Command]
name = "R-Typhoon"
command = ~F, U, B, D, F, U, B, D, ~z
time = 90

[Command]
name = "R-Typhoon"
command = ~D, F, U, B, D, F, U, B, z
time = 90

[Command]
name = "R-Typhoon"
command = ~D, F, U, B, D, F, U, B, ~z
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, U, B, D, F, U, z
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, U, B, D, F, U, ~z
time = 90

[Command]
name = "R-Typhoon"
command = ~U, B, D, F, U, B, D, F, z
time = 90

[Command]
name = "R-Typhoon"
command = ~U, B, D, F, U, B, D, F, ~z
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, B, D, F, B, z
time = 90

[Command]
name = "R-Typhoon"
command = ~B, D, F, B, D, F, B, ~z
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, F, D, B, F, z
time = 90

[Command]
name = "R-Typhoon"
command = ~F, D, B, F, D, B, F, ~z
time = 90

;-| Double Tap |-----------------------------------------------------------
[Command]
name = "FF"     ;Required (do not remove)
command = F, F
time = 10

[Command]
name = "BB"     ;Required (do not remove)
command = B, B
time = 10

;-| 2/3 Button Combination |-----------------------------------------------
[Command]
name = "recovery" ;Required (do not remove)
command = x+a
time = 1

[Command]
name = "AI_11"
command = s, s, s, s, s, s, s, s, s, s, s, s, s, s, s
time = 1

[Command]
name = "AI_12"
command = D, D, D, D, D, D, D, D, D, D, D, D, D, D, a
time = 1

[Command]
name = "AI_13"
command = F, F, F, F, F, F, F, F, F, F, F, F, F, F, a
time = 1

[Command]
name = "AI_14"
command = U, U, U, U, U, U, U, U, U, U, U, U, U, U, a
time = 1

[Command]
name = "AI_15"
command = B, B, B, B, B, B, B, B, B, B, B, B, B, B, a
time = 1
 
[Command]
name = "AI_16"
command = a, a, a, a, a, a, a, a, a, a, a, a, a, a, b
time = 1

[Command]
name = "AI_17"
command = b, b, b, b, b, b, b, b, b, b, b, b, b, b, a
time = 1

[Command]
name = "AI_18"
command = c, c, c, c, c, c, c, c, c, c, c, c, c, c, a
time = 1

[Command]
name = "AI_19"
command = x, x, x, x, x, x, x, x, x, x, x, x, x, x, a
time = 1

[Command]
name = "AI_20"
command = y, y, y, y, y, y, y, y, y, y, y, y, y, y, a
time = 1

;-| Single Button |---------------------------------------------------------
[Command]
name = "a"
command = a
time = 1

[Command]
name = "b"
command = b
time = 1

[Command]
name = "c"
command = c
time = 1

[Command]
name = "x"
command = x
time = 1

[Command]
name = "y"
command = y
time = 1

[Command]
name = "z"
command = z
time = 1

[Command]
name = "s"
command = s
time = 1

[Command]
name = "start"
command = s
time = 1

;-| Single Dir |------------------------------------------------------------
[Command]
name = "fwd" ;Required (do not remove)
command = $F
time = 1

[Command]
name = "downfwd"
command = $DF
time = 1

[Command]
name = "down" ;Required (do not remove)
command = $D
time = 1

[Command]
name = "downback"
command = $DB
time = 1

[Command]
name = "back" ;Required (do not remove)
command = $B
time = 1

[Command]
name = "upback"
command = $UB
time = 1

[Command]
name = "up" ;Required (do not remove)
command = $U
time = 1

[Command]
name = "upfwd"
command = $UF
time = 1

;-| Hold Button |---------------------------------------------------------
[Command]
name = "hold_x"
command = /x
time = 1

[Command]
name = "hold_y"
command = /y
time = 1

[Command]
name = "hold_z"
command = /z
time = 1

[Command]
name = "hold_a"
command = /a
time = 1

[Command]
name = "hold_b"
command = /b
time = 1

[Command]
name = "hold_c"
command = /c
time = 1

[Command]
name = "hold_s"
command = /s
time = 1

;-| Hold Dir |------------------------------------------------------------
[Command]
name = "holdfwd" ;Required (do not remove)
command = /$F
time = 1

[Command]
name = "holddownfwd"
command = /$DF
time = 1

[Command]
name = "holddown" ;Required (do not remove)
command = /$D
time = 1

[Command]
name = "holddownback"
command = /$DB
time = 1

[Command]
name = "holdback" ;Required (do not remove)
command = /$B
time = 1

[Command]
name = "holdupback"
command = /$UB
time = 1

[Command]
name = "holdup" ;Required (do not remove)
command = /$U
time = 1

[Command]
name = "holdupfwd"
command = /$UF
time = 1

[Command]
name = "AI_21"
command = z, z, z, z, z, z, z, z, z, z, z, z, z, z, a
time = 1

[Command]
name = "AI_22"
command = s, s, s, s, s, s, s, s, s, s, s, s, s, s, a
time = 1

[Command]
name = "AI_23"
command = D, D, D, D, D, D, D, D, D, D, D, D, D, D, b
time = 1

[Command]
name = "AI_24"
command = F, F, F, F, F, F, F, F, F, F, F, F, F, F, b
time = 1

[Command]
name = "AI_25"
command = U, U, U, U, U, U, U, U, U, U, U, U, U, U, b
time = 1

[Command]
name = "AI_26"
command = B, B, B, B, B, B, B, B, B, B, B, B, B, B, b
time = 1
 
[Command]
name = "AI_27"
command = a, a, a, a, a, a, a, a, a, a, a, a, a, a, c
time = 1

[Command]
name = "AI_28"
command = b, b, b, b, b, b, b, b, b, b, b, b, b, b, c
time = 1

[Command]
name = "AI_29"
command = c, c, c, c, c, c, c, c, c, c, c, c, c, c, b
time = 1

[Command]
name = "AI_30"
command = x, x, x, x, x, x, x, x, x, x, x, x, x, x, b
time = 1

;---------------------------------------------------------------------------
; Don't remove the following line. It's required by the CMD standard.
[Statedef -1]

;---------------------------------------------
[State -1, AI]
type = Varset
triggerall = var(50) = 0
trigger1 = command = "AI_1"
trigger2 = command = "AI_2"
trigger3 = command = "AI_3"
trigger4 = command = "AI_4"
trigger5 = command = "AI_5"
trigger6 = command = "AI_6"
trigger7 = command = "AI_7"
trigger8 = command = "AI_8"
trigger9 = command = "AI_9"
trigger10 = command = "AI_10"
trigger11 = command = "AI_11"
trigger12 = command = "AI_12"
trigger13 = command = "AI_13"
trigger14 = command = "AI_14"
trigger15 = command = "AI_15"
trigger16 = command = "AI_16"
trigger17 = command = "AI_17"
trigger18 = command = "AI_18"
trigger19 = command = "AI_19"
trigger20 = command = "AI_20"
trigger21 = command = "AI_21"
trigger22 = command = "AI_22"
trigger23 = command = "AI_23"
trigger24 = command = "AI_24"
trigger25 = command = "AI_25"
trigger26 = command = "AI_26"
trigger27 = command = "AI_27"
trigger28 = command = "AI_28"
trigger29 = command = "AI_29"
trigger30 = command = "AI_30"
var(50) = 1

;---------------------------------------------------------------------------
;Stand Light Punch
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "x"
triggerall = Command != "RisingH_L"
triggerall = Command != "Mex-Typhoon_L"
triggerall = Command != "Mex-Typhoon_M"
triggerall = Command != "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
triggerall = StateType != A
triggerall = Command != "holddown"
triggerall = Command != "holdfwd"
triggerall = Command != "holdback"
trigger1 = stateno = 300
trigger1 = animelemtime(3) >=1
trigger2 = ctrl
ignorehitpause = 1
value = 200

;Stand Light Punch FWD-BACK
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = Command != "holddown"
triggerall = Command != "RisingH_L"
triggerall = Command != "Mex-Typhoon_L"
triggerall = Command != "Mex-Typhoon_M"
triggerall = Command != "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
triggerall = Command = "x" && (Command = "holdfwd" || Command = "holdback")
trigger1 = stateno = 200
trigger1 = animelemtime(2) >=3
trigger2 = stateno = 300
trigger2= animelemtime(3) >=1
trigger3 = ctrl
ignorehitpause = 1
value = 201

;Stand Medium Punch
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "y"
triggerall = Command != "RisingH_M"
triggerall = Command != "Mex-Typhoon_L"
triggerall = Command != "Mex-Typhoon_M"
triggerall = Command != "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
triggerall = StateType != A
triggerall = P2Bodydist X>=47
triggerall = Command != "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = Ifelse(stateno=20,1700,210)

;Stand Medium Punch Near
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "y"
triggerall = Command != "RisingH_M"
triggerall = Command != "Mex-Typhoon_L"
triggerall = Command != "Mex-Typhoon_M"
triggerall = Command != "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
triggerall = StateType != A
triggerall = Command != "holddown"
triggerall = P2Bodydist X<47
trigger1 = ctrl
ignorehitpause = 1
value = Ifelse(stateno=20,1700,211)

;Stand Strong Punch
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "z"
triggerall = Command != "Mex-Typhoon_L"
triggerall = Command != "Mex-Typhoon_M"
triggerall = Command != "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
triggerall = Command != "RisingH_S"
triggerall = StateType != A
triggerall = P2Bodydist X>=47
triggerall = Command != "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = Ifelse(stateno=20,1500,220)

;Stand Strong Punch Near
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "z"
triggerall = Command != "RisingH_S"
triggerall = Command != "Mex-Typhoon_L"
triggerall = Command != "Mex-Typhoon_M"
triggerall = Command != "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
triggerall = StateType != A
triggerall = Command != "holddown"
triggerall = P2Bodydist X<47
trigger1 = ctrl
ignorehitpause = 1
value = Ifelse(stateno=20,1500,221)

;Stand Light Kick
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "a"
triggerall = StateType != A
triggerall = P2Bodydist X>=47
triggerall = Command != "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = 230

;Stand Light Kick Near
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "a"
triggerall = StateType != A
triggerall = Command != "holddown"
triggerall = P2Bodydist X<47
trigger1 = ctrl
ignorehitpause = 1
value = 231

;Stand Medium Kick
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "b"
triggerall = StateType != A
triggerall = P2Bodydist X>=45
triggerall = Command != "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = 240

;Stand Medium Kick Near
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "b"
triggerall = StateType != A
triggerall = Command != "holddown"
triggerall = P2Bodydist X<45
trigger1 = ctrl
ignorehitpause = 1
value = 241

;Stand Strong Kick
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "c"
triggerall = StateType != A
triggerall = P2Bodydist X>=41
triggerall = Command != "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = Ifelse(stateno=20,1600,250)

;Stand Strong Kick Near
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "c"
triggerall = StateType != A
triggerall = Command != "holddown"
triggerall = P2Bodydist X<41
trigger1 = ctrl
ignorehitpause = 1
value = Ifelse(stateno=20,1600,251)

;Crouch Light Punch
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "x"
triggerall = Command != "Mex-Typhoon_L"
triggerall = Command != "Mex-Typhoon_M"
triggerall = Command != "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
triggerall = Command != "RisingH_L"
triggerall = StateType != A
triggerall = Command = "holddown"
trigger1 = ctrl
trigger2 = stateno = 300
trigger2 = animelemtime(3) >=2
ignorehitpause = 1
value = 300

;Crouch Medium Punch
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "y"
triggerall = Command != "Mex-Typhoon_L"
triggerall = Command != "Mex-Typhoon_M"
triggerall = Command != "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
triggerall = Command != "RisingH_M"
triggerall = StateType != A
triggerall = Command = "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = 310

;Crouch Strong Punch
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "z"
triggerall = Command != "Mex-Typhoon_L"
triggerall = Command != "Mex-Typhoon_M"
triggerall = Command != "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
triggerall = Command != "RisingH_S"
triggerall = StateType != A
triggerall = Command = "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = 320

;Crouch Light Kick
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "a"
triggerall = StateType != A
triggerall = Command = "holddown"
trigger1 = ctrl
trigger2 = stateno = 300
trigger2 = animelemtime(3) >=2
ignorehitpause = 1
value = 330

;Crouch Medium Kick
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "b"
triggerall = StateType != A
triggerall = Command = "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = 340

;Crouch Strong Kick
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "c"
triggerall = StateType != A
triggerall = Command = "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = 350

;Jump Light Punch
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = Command = "x"
triggerall = Command != "Condor_D"
triggerall = Command != "Condor_x"
triggerall = Command != "Condor_y"
triggerall = Command != "Condor_z"
trigger1 = ctrl
ignorehitpause = 1
value = 400

;Jump Medium Punch
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = Command != "holddown"
triggerall = Command != "Condor_D"
triggerall = Command != "Condor_x"
triggerall = Command != "Condor_y"
triggerall = Command != "Condor_z"
triggerall = Command = "y"
trigger1 = ctrl
ignorehitpause = 1
value = 410

;Jump Medium Punch Down
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = Command = "holddown"
triggerall = Command != "Condor_D"
triggerall = Command != "Condor_x"
triggerall = Command != "Condor_y"
triggerall = Command != "Condor_z"
triggerall = Command = "y"
trigger1 = ctrl
ignorehitpause = 1
value = ifelse(Vel x = 0,410,411)

;Jump Strong Punch Neutral
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = Command = "z"
triggerall = Command != "Condor_D"
triggerall = Command != "Condor_x"
triggerall = Command != "Condor_y"
triggerall = Command != "Condor_z"
triggerall = Vel x = 0
trigger1 = ctrl
ignorehitpause = 1
value = 420

;Jump Strong Punch
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = Command != "holddown"
triggerall = Command = "z"
triggerall = Command != "Condor_D"
triggerall = Command != "Condor_x"
triggerall = Command != "Condor_y"
triggerall = Command != "Condor_z"
triggerall = Vel x != 0
trigger1 = ctrl
ignorehitpause = 1
value = 421

;Jump Strong Punch Down
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = Command = "holddown"
triggerall = Command = "z"
triggerall = Command != "Condor_D"
triggerall = Command != "Condor_x"
triggerall = Command != "Condor_y"
triggerall = Command != "Condor_z"
triggerall = Vel x != 0
trigger1 = ctrl
ignorehitpause = 1
value = 422

;Jump Light Kick
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = Command = "a"
trigger1 = ctrl
ignorehitpause = 1
value = 430

;Jump Medium Kick
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = Command = "b"
trigger1 = ctrl
ignorehitpause = 1
value = 440

;Jump Strong Kick
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = Command = "c"
trigger1 = ctrl
ignorehitpause = 1
value = 450

;--------------Special---------------------------

;Rising Hawk Light
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = Command = "RisingH_L"
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = ctrl
ignorehitpause = 1
value = 1000

;Rising Hawk Light
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = Command = "RisingH_M"
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = ctrl
ignorehitpause = 1
value = 1010

;Rising Hawk Light
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = Command = "RisingH_S"
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = ctrl
ignorehitpause = 1
value = 1020

;Condor Dive
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = vel y <=0
triggerall = pos y <=-30
trigger1 = Command = "Condor_D" && ctrl
trigger2 = Command = "Condor_x" && ctrl
trigger3 = Command = "Condor_y"  && ctrl
trigger4 = Command = "Condor_z" && ctrl
trigger5 = stateno = 400
trigger5 = animelemtime(2) <=2
trigger5 = (Command = "Condor_D"||Command = "Condor_x"||Command = "Condor_y"||Command = "Condor_z")
ignorehitpause = 1
value = 1100

;Mexican Typhoon Light
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "Mex-Typhoon_L"
triggerall = Command != "R-Typhoon"
triggerall = StateType != A
triggerall = stateno != 52
triggerall = stateno != 40
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = ctrl
ignorehitpause = 1
value = 1200
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "Mex-Typhoon_L"
triggerall = Command != "R-Typhoon"
trigger1 = stateno = 52
trigger2 = stateno = 40
ignorehitpause = 1
value = 1201

;Mexican Typhoon Medium
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "Mex-Typhoon_M"
triggerall = Command != "R-Typhoon"
triggerall = StateType != A
triggerall = stateno != 52
triggerall = stateno != 40
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = ctrl
ignorehitpause = 1
value = 1210
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "Mex-Typhoon_M"
triggerall = Command != "R-Typhoon"
trigger1 = stateno = 52
trigger2 = stateno = 40
ignorehitpause = 1
value = 1211

;Mexican Typhoon Strong
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
triggerall = StateType != A
triggerall = stateno != 52
triggerall = stateno != 40
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = ctrl
ignorehitpause = 1
value = 1220
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "Mex-Typhoon_S"
triggerall = Command != "R-Typhoon"
trigger1 = stateno = 52
trigger2 = stateno = 40
ignorehitpause = 1
value = 1221

;Raging Typhoon
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "R-Typhoon"
triggerall = Power >= 3000
triggerall = StateType != A
triggerall = stateno != 52
triggerall = stateno != 40
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = ctrl
ignorehitpause = 1
value = 1230
[State -1]
type = ChangeState
triggerall = var(50) = 0
triggerall = Roundstate = 2
triggerall = Command = "R-Typhoon"
triggerall = Power >= 3000
trigger1 = stateno = 52
trigger2 = stateno = 40
ignorehitpause = 1
value = 1231





























































































;****************************************************************************
;****************************************************************************
;************   AI  ***********************************************************
;****************************************************************************
;****************************************************************************

[State -1, GUARD]
type = changestate
triggerall = var(50) = 1
triggerall = StateNo != [120,155]
triggerall = ctrl || (stateno = [14,15]) || stateno = 20
triggerall = statetype != A
triggerall = inguarddist
trigger1 = Random <= 150
trigger2 = enemy,numproj > 0
trigger2 = Random <= 200
trigger3 = P2BodyDist X > 90
trigger3 = Random <= 200
trigger4 = EnemyNear,Time > 50
trigger4 = Random <= 300
value = 120

[State -3, VELX-0]
Type = velset
trigger1 = var(50) = 1
Trigger1 = stateno = [120,131]
Trigger1 = statetype != A
x = 0

[State -1, STOP]
type = ChangeState
trigger1 = roundstate = 2
trigger1 = var(50) = 1
trigger1 = p2bodydist x > 20
trigger1 = ctrl
trigger1 = random < 300
trigger1 = !InGuardDist
trigger1 = statetype = S
trigger1 = stateno != [120,155]
trigger1 = stateno != [20,21]
trigger1 = stateno != [10,15]
value = 20

[State -1, JUMP]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = statetype = S
trigger1 = ctrl || stateno = 20 || (stateno = [120,131])
trigger1 = P2BodyDist X >= 50
trigger1 = Random < 10+(P2BodyDist X=[90,140])*10
trigger2 = ctrl || stateno = 20 || (stateno = [120,131])
trigger2 = P2BodyDist X >= 130
trigger2 = Random < 300
trigger2 = enemy, numproj >= 1
trigger3 = ctrl || stateno = 20
trigger3 = P2bodydist X <= 60
trigger3 = BackEdgeBodyDist <= 10
trigger3 = Random < 500
trigger3 = p2statetype = L
trigger4 = ctrl || stateno = 20 || (stateno = [120,131])
trigger4 = P2BodyDist X <= 85
trigger4 = Random < 50+(enemynear,hitdefattr=SCA,HT)*250
trigger4 = enemynear,hitdefattr = SCA,NT,ST,HT
value = 40

;---------------------------------------------------------------------------
;Stand Light Punch
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [46,50]
triggerall = P2BodyDist Y = [-60,0]
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != C
triggerall = Command != "holddown"
triggerall = Command != "holdfwd"
triggerall = Command != "holdback"
trigger1 = ctrl || stateno = 20
trigger2 = stateno = 300
trigger2 = animelemtime(3) >=1
ignorehitpause = 1
value = 200


;Stand Light Punch FWD-BACK
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [0,45]
triggerall = P2BodyDist Y = [-60,0]
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != C
triggerall = Command != "holddown"
triggerall = (Command = "holdfwd" || Command = "holdback")
trigger1 = ctrl || stateno = 21
trigger2 = stateno = 200
trigger2 = animelemtime(2) >=3
trigger3 = stateno = 300
trigger3= animelemtime(3) >=1
ignorehitpause = 0
value = 201

;Stand Medium Punch
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Roundstate = 2
triggerall = Random < AILevel * 8
triggerall = P2BodyDist X = [47,55]
triggerall = P2BodyDist Y = [-100,0]
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype = S
triggerall = Command != "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = Ifelse(stateno=20,1700,210)

;Stand Medium Punch Near
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Roundstate = 2
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [0,46]
triggerall = P2BodyDist Y = [-100,0]
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != C
triggerall = Command != "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = Ifelse(stateno=20,1700,211)

;Stand Strong Punch
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Roundstate = 2
triggerall = Random < AILevel * 10
triggerall = P2BodyDist X = [47,70]
triggerall = P2BodyDist Y = [-90,0]
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != C
triggerall = Command != "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = Ifelse(stateno=20,1500,220)

;Stand Strong Punch Near
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Roundstate = 2
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [0,46]
triggerall = P2BodyDist Y = [-90,0]
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != A
triggerall = Command != "holddown"
trigger1 = ctrl
ignorehitpause = 1
value = Ifelse(stateno=20,1500,221)

;Stand Light Kick
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = Random = [300,550]
triggerall = P2BodyDist X = [46,70]
triggerall = P2BodyDist Y = [-50,0]
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != A
triggerall = Command != "holddown"
triggerall = Command != "holdfwd"
triggerall = Command != "holdback"
trigger1 = ctrl || stateno = 20
ignorehitpause = 1
value = 230

;Stand Light Kick Near
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [0,47]
triggerall = P2BodyDist Y = [-50,0]
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != A
triggerall = Command != "holddown"
triggerall = Command != "holdfwd"
triggerall = Command != "holdback"
trigger1 = ctrl || stateno = 20
ignorehitpause = 1
value = 231

;Stand Medium Kick
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = Random = [400,630]
triggerall = P2BodyDist X = [45,70]
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != A
triggerall = p2statetype != L
triggerall = Command != "holddown"
trigger1 = ctrl || stateno = 20
ignorehitpause = 0
value = 240

;Stand Medium Kick Near
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [0,45]
triggerall = P2BodyDist Y = [-50,0]
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != A
triggerall = p2statetype != L
triggerall = Command != "holddown"
trigger1 = ctrl || stateno = 20
ignorehitpause = 0
value = 241

;Stand Strong Kick
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = P2bodydist X = [41,90]
triggerall = P2BodyDist Y = [-80,0]
triggerall = var(50) = 1
triggerall = Random = [0,500-(stateno=11)*30]
triggerall = Command != "holddown"
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != A
trigger1 = ctrl || Stateno= 11 || stateno = 20
ignorehitpause = 1
value = Ifelse(stateno=20,1600,250)

;Stand Strong Kick Near
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = P2bodydist X = [0,41]
triggerall = P2BodyDist Y = [-60,0]
triggerall = var(50) = 1
triggerall = Random = [50,700-(stateno=11)*30]
triggerall = Command != "holddown"
triggerall = statetype = S
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != A
trigger1 = ctrl || Stateno= 11 || stateno = 20
ignorehitpause = 1
value = Ifelse(stateno=20,1600,251)

;Crouch Light Punch
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [0,40]
triggerall = P2BodyDist Y = [-30,0]
triggerall = statetype != A
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != C
trigger1 = ctrl || stateno = 11
trigger2 = stateno = 300
trigger2 = animelemtime(3) >=2
ignorehitpause = 1
value = 300

;Crouch Medium Punch
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = P2BodyDist X = [30,55]
triggerall = P2BodyDist Y = [-30,0]
triggerall = statetype != A
triggerall = MoveType != H
triggerall = Random = [430,770+(enemy,stateno=[150,153])*40]
triggerall = p2statetype != L
triggerall = p2statetype != A
trigger1 = ctrl || Stateno = 11 || stateno = 20
ignorehitpause = 1
value = 310

;Crouch Strong Punch
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = roundstate = 2
triggerall = P2bodydist X = [0,30+(stateno=20)*15]
triggerall = P2BodyDist Y = [-60,0]
triggerall = Random < AILevel * 11
triggerall = statetype != A
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype = S
trigger1 = ctrl || Stateno = 11 || stateno = 20
ignorehitpause = 1
value = 320

;Crouch Light Kick
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [0,40]
triggerall = P2BodyDist Y = [-30,0]
triggerall = statetype != A
triggerall = MoveType != H
triggerall = p2statetype != L
triggerall = p2statetype != A
trigger1 = ctrl || stateno = 20
trigger2 = stateno = 300
trigger2 = animelemtime(3) >=2
ignorehitpause = 1
value = 330

;Crouch Medium Kick
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = P2BodyDist X = [0,50]
triggerall = statetype != A
triggerall = MoveType != H
triggerall = Random = [850,950-(stateno=11)*40]
triggerall = p2statetype = S
triggerall = p2statetype != L
trigger1 = ctrl || Stateno = 11 || stateno = 20
ignorehitpause = 1
value = 340

;Crouch Strong Kick
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = P2BodyDist X = [0,70]
triggerall = statetype != A
triggerall = MoveType != H
triggerall = Random = [0,380]
triggerall = p2statetype != A
triggerall = p2statetype != L
trigger1 = ctrl || Stateno = 11 || stateno = 20
ignorehitpause = 0
value = 350

;Jump Light Punch
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = P2BodyDist X = [0,80+(vel X>0)*15]
triggerall = P2BodyDist Y = [-60,60]
triggerall = statetype = A
triggerall = MoveType != H
triggerall = pos Y <= -30
triggerall = vel Y <0 
triggerall = p2statetype != L
trigger1 = Random = [50,800-(P2statetype=A)*50]
trigger1 = ctrl || stateno = [120,132]
ignorehitpause = 1
value = 400

;Jump Medium Punch Down
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = P2BodyDist X = [-40,45+(vel X>0)*15]
triggerall = P2BodyDist Y = [-60,60]
triggerall = statetype = A
triggerall = MoveType != H
triggerall = P2statetype = A
triggerall = pos Y <= -30 
triggerall = vel Y >0 
triggerall = p2statetype != L
triggerall = Random < AILevel * 11
trigger1 = ctrl || stateno = [120,132]
ignorehitpause = 1
value = ifelse(Vel x = 0,410,411)

;Jump Strong Attack
[State -1]
type = ChangeState
triggerall = roundstate = 2
triggerall = var(50) = 1
triggerall = P2BodyDist X = [-50,90+(vel X>0)*15]
triggerall = P2BodyDist Y = [-60,60]
triggerall = statetype = A
triggerall = MoveType != H
triggerall = P2statetype != A
triggerall = pos Y <= -30
triggerall = vel Y >0
triggerall = p2statetype != L
triggerall = Random < AILevel * 11
trigger1 = ctrl || stateno = [120,132]
ignorehitpause = 1
value = Ifelse(random<=500,422,450)

;---- AI POWERS ----------------
;---- AI POWERS ----------------
;---- AI POWERS ----------------
;---- AI POWERS ----------------

;Rising Hawk Light
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [0,40+(enemy,vel X>=1.5)*10]
triggerall = P2BodyDist Y = [-50,0]
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = MoveType != H
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = P2BodyDist X >= 70
trigger8 = Random < 300
trigger8 = enemy, numproj >= 1
ignorehitpause = 1
value = 1000

;Rising Hawk Medium
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [30,50+(enemy,vel X>=1.4)*10]
triggerall = P2BodyDist Y = [-100,0]
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = MoveType != H
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = ctrl
trigger8 = random >700
ignorehitpause = 1
value = 1010

;Rising Hawk Strong
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = P2BodyDist X = [40,60+(enemy,vel X>=1)*10]
triggerall = P2BodyDist Y = [-120,0]
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = MoveType != H
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = ctrl
trigger8 = random <300
ignorehitpause = 1
value = 1020

;Condor Dive
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Random < AILevel *11
triggerall = Roundstate = 2
triggerall = StateType = A
triggerall = MoveType != H
triggerall = vel y <=0
triggerall = pos y <=-30
triggerall = stateno != [1100,1102]
trigger1 = Random < 400
trigger1 = stateno = 400
trigger1 = animelemtime(2) <=2
trigger2= pos y <=-80
trigger2 = P2BodyDist X >= 70
trigger2 = Random < 700
trigger2 = enemy, numproj >= 1
trigger3 = ctrl && random <500
trigger3 = p2statetype != L
ignorehitpause = 1
value = 1100

;Mexican Typhoon Light
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = Power < 3000
triggerall = P2BodyDist X = [0,80]
triggerall = P2BodyDist Y = [-50,0]
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = MoveType != H
triggerall = p2StateType = C
triggerall = p2StateType != L
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = Random >= 200
trigger8 = Random <400
trigger8 = ctrl
ignorehitpause = 1
value = ifelse((stateno =40 || stateno = 52),1201,1200)

;Mexican Typhoon Medium
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = Power < 3000
triggerall = P2BodyDist X = [0,80]
triggerall = P2BodyDist Y = [-65,0]
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = MoveType != H
triggerall = p2StateType = S
triggerall = p2StateType != L
triggerall = movecontact
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = p2StateType != A
trigger8 = p2MoveType != A
trigger8 = Random >= 600
trigger8 = Random <700
trigger8 = ctrl
ignorehitpause = 1
value = ifelse((stateno =40 || stateno = 52),1211,1210)

;Mexican Typhoon Strong
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = Power < 3000
triggerall = P2BodyDist X = [0,85]
triggerall = P2BodyDist Y = [-80,0]
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = MoveType != H
triggerall = p2StateType != A
triggerall = p2StateType != L
triggerall = movecontact
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
trigger8 = p2StateType != A
trigger8 = p2MoveType != A
trigger8 = Random >= 800
trigger8 = Random <900
trigger8 = ctrl
ignorehitpause = 1
value = ifelse((stateno =40 || stateno = 52),1221,1220)

;Raging Typhoon
[State -1]
type = ChangeState
triggerall = var(50) = 1
triggerall = Random < AILevel * 11
triggerall = Power >= 3000
triggerall = P2BodyDist X = [0,85]
triggerall = P2BodyDist Y = [-100,0]
triggerall = Roundstate = 2
triggerall = StateType != A
triggerall = MoveType != H
triggerall = p2StateType != A
triggerall = p2StateType != L
trigger1 = stateno = 200
trigger1 = animelemtime(2) <=2
trigger2 = stateno = 201
trigger2 = animelemtime(2) <=2
trigger3 = stateno = 211
trigger3 = animelemtime(2) <=2
trigger4 = stateno = 231
trigger4 = animelemtime(2) <=2
trigger5 = stateno = 241
trigger5 = animelemtime(2) <=2
trigger6 = stateno = 300
trigger6 = animelemtime(3) <=2
trigger7 = stateno = 330
trigger7 = animelemtime(3) <=2
ignorehitpause = 1
value = ifelse((stateno =40 || stateno = 52),1231,1230)

