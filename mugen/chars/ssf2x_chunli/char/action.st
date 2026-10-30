[StateDef 200];弱パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 200
VelSet = 0
SprPriority = 2

[State 200]
Type = Turn
Trigger1 = P2Dist X < 0 && Time = 0

[State 200]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 7

[State 200]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 28
GetPower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -20+Floor(P2Dist X)/2, -66
HitSound = S0, 1
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 200]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 201];近距離弱パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 201
VelSet = 0
SprPriority = 2

[State 201]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 7

[State 201]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 112
GetPower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -20+Floor(P2Dist X)/2, -73
HitSound = S0, 1
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 201]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 210];中パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 210
VelSet = 0
SprPriority = 2

[State 210]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 210]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 133
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/2, -57
HitSound = S0, 2
GuardSound = S0, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24
Air.HitTime = 99
Ground.Velocity = -7*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 210]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 211];近距離中パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 211
VelSet = 0
SprPriority = 2

[State 211]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 211]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
Hitflag = MA
GuardFlag = M
Damage = 140
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/2, -74
HitSound = S0, 2
GuardSound = S0, 0
Animtype = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24
Air.HitTime = 99
Ground.Velocity = -7*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 211]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 220];強パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 220
VelSet = 0
SprPriority = 2

[State 220]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 220]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 147
GetPower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/2, -74
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 220]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 221];近距離強パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 221
VelSet = 0
SprPriority = 2

[State 221]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 221]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 147
GetPower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/2, -54
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 221]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 230];弱キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 230
VelSet = 0
SprPriority = 2

[State 230]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 7

[State 230]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 119
Getpower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -56
HitSound = S0, 4
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 230]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 231];近距離弱キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 231
VelSet = 0
SprPriority = 2

[State 231]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 7

[State 231]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 126
Getpower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/2, -54
HitSound = S0, 4
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 231]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 240];中キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 240
VelSet = 0
SprPriority = 2

[State 240]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 240]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 147
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -14+Floor(P2Dist X)/2, -78
HitSound = S0, 5
GuardSound = S0, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24
Air.HitTime = 99
Ground.Velocity = -7*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 240]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 241];近距離中キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 241
VelSet = 0
SprPriority = 2

[State 241]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 241]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 147
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -34+Floor(P2Dist X)/2, -52
HitSound = S0, 5
GuardSound = S0, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24
Air.HitTime = 99
Ground.Velocity = -7*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 241]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 250];強キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 250
VelSet = 0
SprPriority = 2

[State 250]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 250]
Type = HitDef
Trigger1 = AnimElem = 4
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 168
Getpower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/2, -86
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 250]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 251];近距離強キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 251
VelSet = 0
SprPriority = 2

[State 251]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 251]
Type = HitDef
Trigger1 = AnimElem = 5
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 168
Getpower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -20+Floor(P2Dist X)/2, -86
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 251]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 260];後方回転脚
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 450
SprPriority = 2

[State 260]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 260]
Type = ChangeAnim
Trigger1 = Time = 1
Value = 450

[State 260]
Type = StateTypeSet
Trigger1 = AnimElem = 6
Value = A

[State 260]
Type = VelSet
Trigger1 = AnimElem = 6
X = -1.9*Const(Size.XScale)
Y = -8
[State 260]
Type = VelAdd
Trigger1 = AnimElem = 6,>0
Y = .44

[State 260]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 133
GetPower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -34+Floor(P2Dist X)/2, -52
HitSound = S0, 5
GuardSound = S0, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24
Air.HitTime = 99
Ground.Velocity = -7*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14*Const(Size.XScale)
YAccel = .25
P2Facing = 1
[State 260]
Type = HitDef
Trigger1 = AnimElem = 4 && !MoveContact
Attr = A, NA
HitFlag = MA
GuardFlag = M
Damage = 119
GetPower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -50 - (AnimElem=4,>0)*20 - (AnimElem=5,>0)*10
HitSound = S0, 5
GuardSound = S0, 0
AnimType = Up
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24
Air.HitTime = 99
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -7*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14*Const(Size.XScale)
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 260]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 52

[StateDef 270];鶴脚落
Type = A
MoveType = A
Physics = N
Ctrl = 0
VelSet = 1.5*Const(Size.XScale), -7
Anim = 270
SprPriority = 2

[State 270]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 270]
Type = PosAdd
Trigger1 = Time = 0
Y = -16

[State 270]
Type = VelAdd
Trigger1 = 1
Y = .27

[State 270]
Type = Turn
Trigger1 = AnimElem = 6

[State 270]
Type = HitDef
Trigger1 = AnimElem = 7,>0 && !MoveContact && P2StateType != A
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 119
Getpower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -12+Floor(P2Dist X)/3, 6
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Up
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
[State 270]
Type = HitDef
Trigger1 = AnimElem = 7,>0 && !MoveContact && P2StateType = A
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 119
Getpower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -12+Floor(P2Dist X)/3, 6
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = 2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
YAccel = .25

[State 270]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 52

[StateDef 300];しゃがみ弱パンチ
Type = C
MoveType = A
Physics = N
Ctrl = 0
Anim = 300
VelSet = 0
SprPriority = 2

[State 300]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 7

[State 300]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = C, NA
HitFlag = MA
GuardFlag = M
Damage = 28
GetPower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -30+Floor(P2Dist X)/2, -38
HitSound = S0, 1
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 300]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 310];しゃがみ中パンチ
Type = C
MoveType = A
Physics = N
Ctrl = 0
Anim = 310
VelSet = 0
SprPriority = 2

[State 310]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 310]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = C, NA
HitFlag = MA
GuardFlag = M
Damage = 119
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/2, -42
HitSound = S0, 2
GuardSound = S0, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24
Air.HitTime = 99
Ground.Velocity = -7*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 310]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 320];しゃがみ強パンチ
Type = C
MoveType = A
Physics = N
Ctrl = 0
Anim = 320
VelSet = 0
SprPriority = 2

[State 320]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 320]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = C, NA
HitFlag = MA
GuardFlag = M
Damage = 133
GetPower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY =  -36+Floor(P2Dist X)/2, -42
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 320]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 330];しゃがみ弱キック
Type = C
MoveType = A
Physics = N
Ctrl = 0
Anim = 330
VelSet = 0
SprPriority = 2

[State 330]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 7

[State 330]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 84
GetPower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -35+Floor(P2Dist X)/2, -8
HitSound = S0, 4
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 330]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 340];しゃがみ中キック
Type = C
MoveType = A
Physics = N
Ctrl = 0
Anim = 340
VelSet = 0
SprPriority = 2

[State 340]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 340]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 119
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -38+Floor(P2Dist X)/2, -12
HitSound = S0, 5
GuardSound = S0, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24
Air.HitTime = 99
Ground.Velocity = -7*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 340]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 350];しゃがみ強キック
Type = C
MoveType = A
Physics = N
Ctrl = 0
Anim = 350
VelSet = 0
SprPriority = 2

[State 350]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 350]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 133
GetPower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -35+Floor(P2Dist X)/2, -52
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Ground.Type = Trip
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Ground.Velocity = -2*Const(Size.XScale), -6
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Fall = 1
Fall.Recover = 0
Fall.YVelocity = 0
YAccel = .25
P2Facing = 1

[State 350]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 400];垂直空中弱パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 400

[State 400]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 7

[State 400]
Type = HitDef
Trigger1 = AnimElemNo(0) = 4 && !MoveContact
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 112
GetPower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -49
HitSound = S0, 1
GuardSound = S0, 0
Animtype = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 401];空中弱パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 401

[State 401]
Type = VelAdd
Trigger1 = PrevStateNo = 460 || PrevStateNo = 51
Y = -.17

[State 401]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 7

[State 401]
Type = HitDef
Trigger1 = AnimElemNo(0) = 3 && !MoveContact
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 105
GetPower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -49
HitSound = S0, 1
GuardSound = S0, 0
Animtype = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[Statedef 410];垂直空中中パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 410

[State 410]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 410]
Type = HitDef
Trigger1 = AnimElem = 4
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 133
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -49
HitSound = S0, 2
GuardSound = S0, 0
Animtype = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
YAccel = .25
P2Facing = 1

[Statedef 411];空中中パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 411

[State 411]
Type = VelAdd
Trigger1 = PrevStateNo = 460 || PrevStateNo = 51 || (PrevStateNo=514&&sysvar(0))
Y = -.17

[State 411]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 411]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 119
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -49
HitSound = S0, 2
GuardSound = S0, 0
Animtype = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
YAccel = .25
P2Facing = 1

[StateDef 420];垂直空中強パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 420

[State 420]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 420]
Type = HitDef
Trigger1 = AnimElem = 4
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 154
GetPower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -49
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 421];空中強パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 421

[State 421]
Type = VelAdd
Trigger1 = PrevStateNo = 460 || PrevStateNo = 51 || (PrevStateNo=510&&sysvar(0))
Y = -.17

[State 421]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 421]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 147
GetPower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -49
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 430];空中垂直弱キック
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 430

[State 430]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 7

[State 430]
Type = HitDef
Trigger1 = AnimElemNo(0) = 3 && !MoveContact
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 112
GetPower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -12+Floor(P2Dist X)/2, -64
HitSound = S0, 4
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 431];空中弱キック
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 431

[State 431]
Type = VelAdd
Trigger1 = PrevStateNo = 460 || PrevStateNo = 51
Y = -.17

[State 431]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 7

[State 431]
Type = HitDef
Trigger1 = AnimElemNo(0) = 3 && !MoveContact
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 147
GetPower = 0 +(!Var(10))*40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -18+Floor(P2Dist X)/2, -46
HitSound = S0, 4
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 440];空中垂直中キック
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 440

[State 440]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 440]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 133
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -12+Floor(P2Dist X)/2, -64
HitSound = S0, 5
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 441];空中中キック
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 441

[State 441]
Type = VelAdd
Trigger1 = PrevStateNo = 460 || PrevStateNo = 51
Y = -.17

[State 441]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 441]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 119
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -46
HitSound = S0, 5
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 450];空中垂直強キック
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 450

[State 450]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 450]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 154
GetPower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -50 - (AnimElem=4,>0)*20 - (AnimElem=5,>0)*10
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Up
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -2*Const(Size.XScale), -6
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[StateDef 451];空中強キック
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 451

[State 451]
Type = VelAdd
Trigger1 = PrevStateNo = 460 || PrevStateNo = 51
Y = -.17

[State 451]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 9

[State 451]
Type = HitDef
Trigger1 = AnimElem = 3,>0 && !MoveContact
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 147
GetPower = 0 +(!Var(10))*100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -12+Floor(P2Dist X)/2, -64
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 460];鷹爪脚
Type = A
MoveType = A
Physics = N
Ctrl = 0
Anim = 460

[State 460]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 8

[State 460]
Type = CtrlSet
Trigger1 = MoveContact
Value = 1
[State 460]
Type = VelSet
Trigger1 = MoveContact
X = IFelse(FrontEdgeDist<35||Vel X<0, -Vel X, IFelse(Vel X=0, 4*Const(Size.XScale), Vel X))
Y = -5
[State 460]
Type = VelAdd
Trigger1 = 1
Y = IFelse(Ctrl||PrevStateNo=51||PrevStateNo=460, .27, .44)
[State 460]
Type = MoveHitReset
Trigger1 = MoveContact

[State 460]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 119
GetPower = 0 +(!Var(10))*80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -18+Floor(P2Dist X)/3, -5
HitSound = S0, 5
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[State 460]
Type = ChangeState
Trigger1 = Pos Y >= 0
Value = 52