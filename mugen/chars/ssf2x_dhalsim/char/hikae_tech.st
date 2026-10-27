[StateDef 1000];ヨガファイヤー 弱
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 1000
PowerAdd = 48+Var(10)*52

[State 1000]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=5) + (AnimElem=4)*2

[State 1000]
Type = Helper
Trigger1 = AnimElem = 4, 2
Name = "Guard_HANTEI"
ID = 1000
Pos = 40, 0
PosType = P1
StateNo = 1008

[State 1000]
Type = Projectile
trigger1 = Var(10) && AnimElem = 4,2
ProjAnim = 1001
ProjHitAnim = 1002
ProjID = 2
OffSet = 0, 0
Velocity = 3.5*Const(Size.XScale)
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
Attr = S, SP
GuardFlag = MA
HitFlag = MA
Damage = 98, 14
GetPower = 0
GivePower = 0
PauseTime = 0, 6
SparkNo = -1
Guard.SparkNo = -1
HitSound = S1, 3
GuardSound = S2, 0
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 8
Ground.HitTime = 13
Air.HitTime = 20
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -5*Const(Size.XScale), -8
AirGuard.Velocity = -4*Const(Size.XScale), 0
Fall.YVelocity = 0
YAccel = .4
P2Facing = 1
ForceStand = 1
[State 1000]
Type = Projectile
trigger1 = !Var(10) && AnimElem = 4,2 && EnemyNear,StateNo != [5020, 5099]
ProjAnim = 1001
ProjHitAnim = 1002
ProjID = 1
OffSet = 0, 0
Velocity = 3*Const(Size.XScale)
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
Attr = S, SP
GuardFlag = M
HitFlag = MA
Damage = 112, 28
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S1, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 21
Ground.Velocity = -12*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Air.Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2Facing = 1
ForceStand = 1
[State 1000]
Type = Projectile
Trigger1 = !Var(10) && AnimElem = 4,2 && EnemyNear,StateNo = [5020, 5099]
ProjAnim = 1001
ProjHitAnim = 1002
ProjCanselAnim = 1003
ProjRemAnim = 1004
ProjID = 1
OffSet = 0, 0
Velocity = 3*Const(Size.XScale)
ProjPriority = 1
ProjRemoveTime = 128
ProjScale = Const(Size.XScale), 1
Attr = S, SP
GuardFlag = M
HitFlag = MA
Damage = 112, 28
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S1, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 21
Ground.Velocity = -12*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Air.Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2Facing = 1
ForceStand = 1

[State 1000]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1001];ヨガファイヤー 中
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 1000
PowerAdd = 48+Var(10)*52

[State 1001]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=5) + (AnimElem=4)*2

[State 1001]
Type = Helper
Trigger1 = AnimElem = 4, 2
Name = "Guard_HANTEI"
ID = 1000
Pos = 40, 0
PosType = P1
StateNo = 1008

[State 1001]
Type = Projectile
trigger1 = Var(10) && AnimElem = 4, 2
ProjAnim = 1001
ProjHitAnim = 1002
ProjID = 2
OffSet = 0, 0
Velocity = 4.5*Const(Size.XScale)
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
Attr = S, SP
GuardFlag = MA
HitFlag = MAFD
Damage = 98, 14
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = S67+(Var(15)+Var(19))*47+TeamSide
Guard.SparkNo = S61+IFelse(Var(19)!=0,48,Var(15)*49)
SparkXY = 70, -57
HitSound = S2, 5+Var(19)*3
GuardSound = S2, 0+Var(9)+Var(19)
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 6
Ground.HitTime = 10
Guard.HitTime = 12
Air.HitTime = 8
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -5*Const(Size.XScale), -4
AirGuard.Velocity = -4*Const(Size.XScale), 0
Fall.YVelocity = 0
YAccel = .4
P2Facing = 1
ForceStand = 1
[State 1001]
Type = Projectile
trigger1 = !Var(10) && AnimElem = 4, 2 && EnemyNear,StateNo != [5020, 5099]
ProjAnim = 1005
ProjHitAnim = 1002
ProjID = 0
OffSet = 0, 0
Velocity = 2.5
ProjPriority = 3
Attr = S, SP
AnimType = Hard
GuardFlag = MA
HitFlag = MA
Damage = 99, 22
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S0, 3
GuardSound = S0, 0
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -4.5
Air.Velocity = -1.5, -5
AirGuard.Velocity = -4
Air.Fall = 1
YAccel = .25
Fall.YVelocity = -3
P2Facing = 1
ForceStand = 1
[State 1001]
Type = Projectile
trigger1 = !Var(10) && AnimElem = 4, 2 && EnemyNear,StateNo = [5020, 5099]
ProjAnim = 1005
ProjHitAnim = 1002
ProjID = 0
OffSet = 0, 0
Velocity = 2.5
ProjPriority = 3
Attr = S, SP
AnimType = Hard
GuardFlag = MA
HitFlag = M
Damage = 99, 22
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S0, 3
GuardSound = S0, 0
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -4.5
Air.Velocity = -1.5, -5
AirGuard.Velocity = -4
Air.Fall = 1
YAccel = .25
Fall.YVelocity = -3
P2Facing = 1
ForceStand = 1

[State 1001]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1002];ヨガファイヤー 強
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 1000
PowerAdd = 48+Var(10)*52

[State 1002]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=5) + (AnimElem=4)*2

[State 1002]
Type = Helper
Trigger1 = AnimElem = 4, 2
Name = "Guard_HANTEI"
ID = 1000
Pos = 40, 0
PosType = P1
StateNo = 1008

[State 1002]
Type = Projectile
trigger1 = Var(10) && AnimElem = 4, 2
ProjAnim = 1001
ProjHitAnim = 1002
ProjID = 2
OffSet = 0, 0
Velocity = 5.5*Const(Size.XScale)
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
Attr = S, SP
GuardFlag = MA
HitFlag = MAFD
Damage = 112, 21
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = S67+(Var(15)+Var(19))*47+TeamSide
Guard.SparkNo = S61+IFelse(Var(19)!=0,48,Var(15)*49)
SparkXY = 70, -57
HitSound = S2, 5+Var(19)*3
GuardSound = S2, 0+Var(9)+Var(19)
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 6
Ground.HitTime = 10
Guard.HitTime = 12
Air.HitTime = 8
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -5*Const(Size.XScale), -4
AirGuard.Velocity = -4*Const(Size.XScale), 0
Fall.YVelocity = 0
YAccel = .4
P2Facing = 1
ForceStand = 1
[State 1002]
Type = Projectile
trigger1 = !Var(10) && AnimElem = 4, 2 && EnemyNear,StateNo != [5020, 5099]
ProjAnim = 1005
ProjHitAnim = 1002
ProjID = 0
OffSet = 0, 0
Velocity = 2.5
ProjPriority = 3
Attr = S, SP
AnimType = Hard
GuardFlag = MA
HitFlag = MA
Damage = 99, 22
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S0, 3
GuardSound = S0, 0
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -4.5
Air.Velocity = -1.5, -5
AirGuard.Velocity = -4
Air.Fall = 1
YAccel = .25
Fall.YVelocity = -3
P2Facing = 1
ForceStand = 1
[State 1002]
Type = Projectile
trigger1 = !Var(10) && AnimElem = 4, 2 && EnemyNear,StateNo = [5020, 5099]
ProjAnim = 1005
ProjHitAnim = 1002
ProjID = 0
OffSet = 0, 0
Velocity = 2.5
ProjPriority = 3
Attr = S, SP
AnimType = Hard
GuardFlag = MA
HitFlag = M
Damage = 99, 22
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S0, 3
GuardSound = S0, 0
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -4.5
Air.Velocity = -1.5, -5
AirGuard.Velocity = -4
Air.Fall = 1
YAccel = .25
Fall.YVelocity = -3
P2Facing = 1
ForceStand = 1

[State 1002]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1003];ヨガファイヤー スペシャル
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 1000
PowerAdd = -1000

[State 1003]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=5) + (AnimElem=4)*2

[State 1003]
Type = Helper
Trigger1 = AnimElem = 4, 2
Name = "Guard_HANTEI"
ID = 1000
Pos = 40, 0
PosType = P1
StateNo = 1008

[State 1003]
Type = Projectile
trigger1 = AnimElem = 4,2
ProjAnim = 1003
ProjHitAnim = 1004
ProjID = 2
OffSet = 46*Const(Size.XScale), -62
Velocity = 3.5*Const(Size.XScale)
ProjPriority = 1
ProjHits = 2
ProjScale = 2*Const(Size.XScale), 2
Attr = S, SP
GuardFlag = MA
HitFlag = MA
Damage = 80, 14
GetPower = 0
GivePower = 0
PauseTime = 4, 9
SparkNo = -1
Guard.SparkNo = -1
HitSound = S2, 5+Var(19)*3
GuardSound = S2, 0+Var(9)+Var(19)
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 22
Guard.HitTime = 12
Air.HitTime = 20
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -5*Const(Size.XScale), -8
AirGuard.Velocity = -4*Const(Size.XScale), 0
Fall.YVelocity = 0
YAccel = .4
P2StateNo = 1007
P2Facing = 1

[State 1003]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1004];ヨガファイヤー 喰らった相手 地上
Type = U
MoveType = H
Physics = N
VelSet = 0, 0
SprPriority = -1

[State 1004]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 1044]
Type = ChangeAnim
Trigger1 = Time = 0 && AuthorName = "MASA" && SelfAnimExist(5400)
Value = IFelse((GetHitVar(AnimType) = [3, 5]), 5030, IFelse((GetHitVar(GroundType) = 1), 5000, 5010) + GetHitVar(AnimType))
[State 1044]
Type = ChangeAnim
Trigger1 = Time = 0
Value = IFelse((GetHitVar(AnimType) = [3, 5]), 5030, IFelse((GetHitVar(GroundType) = 1), 5000, 5010) + GetHitVar(AnimType))
[State 1044]
Type = ChangeAnim
Trigger1 = Time = 0 && (GetHitVar(AnimType) = [4, 5]) && (SelfAnimExist(5047 + GetHitVar(AnimType)))
Value = 5047 + GetHitVar(AnimType)
[State 1044]
Type = ChangeAnim
Trigger1 = Time > 0
Value = Anim

[State 1044]
Type = ForceFeedback
Trigger1 = Anim = 5000 || Anim = 5010
Persistent = 0
Time = 6
WaveForm = Square
[State 1044]
Type = ForceFeedback
Trigger1 = Anim = 5001 || Anim = 5011
Persistent = 0
Time = 8
WaveForm = SineSquare
Ampl = 110, -1, -.3
[State 1044]
Type = ForceFeedback
Trigger1 = Anim = 5012 || Anim = 5002 || Anim = 5030
Persistent = 0
Time = 15
WaveForm = SineSquare
Ampl = 140

[State 1044]
Type = HitVelSet
Trigger1 = HitShakeOver
Persistent = 0
X = 1

[State 1044]
Type = StateTypeSet
Trigger1 = Time = 4
Physics = S

[State 1044]
Type = ChangeAnim
Trigger1 = AnimTime = 0
Value = 5005 + GetHitVar(AnimType) + (GetHitVar(GroundType) = 2) * 10

[State 1044]
Type = VelMul
Trigger1 = Time >= GetHitVar(SlideTime)
X = .6

[State 1044]
Type = VelSet
Trigger1 = HitOver
X = 0

[State 1044]
Type = DefenceMulSet
Trigger1 = HitOver
Value = 1

[State 1044]
Type = PosAdd
Trigger1 = 1
X = IFelse(TimeMod = 2, 0, 1, -1)

[State 1044]
Type = SelfState
Trigger1 = HitOver
Value = IFelse(StateType = S, 0, 11)
Ctrl = 1

[State 1044]
Type = ChangeState
Trigger1 = HitShakeOver && (StateType = A || GetHitVar(YVel) != 0 || !Alive)
Value = 1045

[StateDef 1045];灼熱波動拳 喰らった相手 吹っ飛び
Type = A
MoveType = H
Physics = N
VelSet = 0, 0
SprPriority = -1

[State 1045]
Type = RemoveExplod
Trigger1 = Time = 0 && SelfAnimExist(5400)
ID = 1044

[State 1045]
Type = NotHitBy
Trigger1 = !Alive || (Vel Y > 1 && PrevStateNo = [2431, 2433]) || PrevStateNo != [2431, 2433]
Value = SCA

[State 1045]
Type = AssertSpecial
Trigger1 = !SelfAnimExist(5400)
Flag = Invisible

[State 1045]
Type = ChangeAnim
Trigger1 = (!HitShakeOver || (Anim = 5030 && AnimTime = 0) || (Anim = 5035 && AnimTime = 0)) && !SelfAnimExist(5400)
IgnoreHitPause = 1
Value = IFelse(!HitShakeOver, 5030, IFelse(Anim = 5030, 5035, 5050))
[State 1045]
Type = ChangeAnim
Trigger1 = SelfAnimExist(5400)
Persistent = 0
IgnoreHitPause = 1
Value = 5400

[State 1045]
Type = VelSet
Trigger1 = HitShakeOver
Persistent = 0
X = -1.5
Y = -5
[State 1045]
Type = VelAdd
Trigger1 = HitShakeOver
Y = .25

[State 1045]
Type = ChangeState
Trigger1 = BackEdgeDist < 2
Value = 505

[State 1045]
Type = ChangeState
Trigger1 = Vel Y > 0 &&  Pos Y >= 25
Value = 1046

[StateDef 1046];灼熱波動拳 喰らった相手開放
Type = U
MoveType = H
Physics = U

[State 1046]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 1044

[State 1046]
Type = SelfState
Trigger1 = Time = 0
Value = 5100

[StateDef 1047];灼熱波動拳 喰らった相手 吹っ飛び 壁引っ掛かり後
Type = A
MoveType = H
Physics = N

[State 1047]
Type = AssertSpecial
Trigger1 = !SelfAnimExist(5400)
Flag = Invisible

[State 1047]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 1047]
Type = VelAdd
Trigger1 = 1
Y = .25

[State 1047]
Type = ChangeState
Trigger1 = Vel Y > 0 &&  Pos Y >= 25
Value = 1046

[StateDef 1007];VAMPIRE 炎上喰らい
Type = U
MoveType = H
Physics = N
VelSet = 0, 0
SprPriority = -1

[State 1007]
Type = NotHitBy
Trigger1 = Time < 4
Value = SCA

[State 1007]
Type = ChangeAnim
Trigger1 = Time = 0
Value = 5000

[State 1007]
Type = ChangeAnim
Trigger1 = Time = 0 && AuthorName = "MASA" && SelfAnimExist(5400)
Value = 5400

[State 1007]
Type = ChangeAnim
Trigger1 = Time = 0 && (Name="Demitri(DS)"||Name="Donovan"||Name="Hsien-ko"||Name="Zabel"||Name="Jon Talbain"||Name="Rikuo")
Value = 7697
[State 1007]
Type = ChangeAnim
Trigger1 = Time = 0 && (Name="Anakaris By Third")
Value = 5621

[State 1007]
Type = ForceFeedback
Trigger1 = Anim = 5000 || Anim = 5010
Persistent = 0
Time = 6
WaveForm = Square
[State 1007]
Type = ForceFeedback
Trigger1 = Anim = 5001 || Anim = 5011
Persistent = 0
Time = 8
WaveForm = SineSquare
Ampl = 110, -1, -.3
[State 1007]
Type = ForceFeedback
Trigger1 = Anim = 5012 || Anim = 5002 || Anim = 5030
Persistent = 0
Time = 15
WaveForm = SineSquare
Ampl = 140

[State 1007]
Type = VelSet
Trigger1 = Time = 4
X = -2
Y = -7

[State 1007]
Type = VelAdd
Trigger1 = Time > 4
Y = .4

[State 1007]
Type = VelSet
Trigger1 = Time = 20
X = 0

[State 1007]
Type = SelfState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 5100

[StateDef 1008];波動拳 CPUに防御させ易くするヘルパー
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 99

[State 1008]
Type = VelSet
trigger1 = Time = 0
X=IFelse(Parent,StateNo=1000,3+Parent,Var(10)/2,IFelse(Parent,StateNo=1001||Parent,StateNo=1041,2.5,IFelse(Parent,StateNo=1002||Parent,StateNo=1042,3,IFelse(Parent,StateNo=1005,1.5,IFelse(Parent,StateNo=1006,2,3)))))

[State 1008]
Type = DestroySelf
Trigger1 = Pos Y > 47 || FrontEdgeDist < -50 || Parent,ProjContact0 = 1, 1 || Parent,ProjCancelTime(0) = 1

[StateDef 1100];ヨガフレイム 弱
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1100

[State 1100]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 7
Value = 10, 0 + (AnimElem=7)*2

[State 1100]
Type = Helper
Trigger1 = AnimElem = 6, 2
Pos = 0, 0
PosType = P1
StateNo = 1101
OwnPal = 1

[State 1100]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1101];ヨガフレイム 弱 ヘルパー
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1101
SprPriority = 99

[State 1101]
Type = BindToParent
Trigger1 = 1
Pos = 0, 0

[State 1101]
Type = ChangeState
Trigger1 = Time > 12 && Parent,NumProjID(2) = 0
Value = 1102
[State 1101]
Type = DestroySelf
Trigger1 = Parent,StateNo != [1100,1199]

[State 1101]
Type = Projectile
Trigger1 = Time = [12,52]
ProjAnim = 1103
ProjID = 2
OffSet = 0, 0
Velocity = 0
ProjRemoveTime = 1
ProjPriority = 1
Attr = S, SP
AnimType = Hard
GuardFlag = M
HitFlag = MA
Damage = 99, 22
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = S1002
Guard.SparkNo = S1002
HitSound = S0, 3
GuardSound = S0, 0
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -4.5
Air.Velocity = -1.5, -5
AirGuard.Velocity = -4
Air.Fall = 1
YAccel = .25
Fall.YVelocity = -3
P2Facing = 1
ForceStand = 1

[StateDef 1102];ヨガフレイム ヘルパー消滅
Type = S
MoveType = I
Physics = N
Anim = 1102

[State 1102]
Type = DestroySelf
Trigger1 = AnimTime = 0

[StateDef 1011];豪昇龍拳 中
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1010
Juggle = 2

[State 1011]
Type = ChangeAnim
Trigger1 = AnimElem = 3, 22 || AnimElem = 5, 16
Value = 1010
Elem = IFelse((AnimElem = 3, 22), 4, 6)

[State 1011]
Type = NotHitBy
Trigger1 = AnimElem = 4, < 0
Value = SCA

[State 1011]
Type = VelSet
Trigger1 = AnimElem = 3
X = 2.5
Y = -4.75
[State 1011]
Type = StateTypeSet
Trigger1 = Time = 8
Value = A
[State 1011]
Type = StateTypeSet
Trigger1 = AnimElem = 6
Value = S
[State 1011]
Type = VelAdd
Trigger1 = Time > 15 && AnimElem = 6, < 0
Y = .3
[State 1011]
Type = VelAdd
Trigger1 = Time > 15 && AnimElem = 3, < 16
X = -.3
[State 1011]
Type = VelSet
Trigger1 = AnimElem = 3, 16
X = 0
[State 1011]
Type = VelSet
Trigger1 = AnimElem = 6
Y = 0
[State 1011]
Type = PosSet
Trigger1 = AnimElem = 6
Y = 0

[State 1011]
Type = PlaySnd
Trigger1 = AnimElem = 1
Value = 1, 2

[State 1011]
Type = HitDef
Trigger1 = Time = 2 || AnimElem = 2 || AnimElem = 3
Attr = S, SA
AnimType = Up
GuardFlag = M
Damage = 99, 22
GetPower = 0
GivePower = 0
PauseTime = 16, 14
SparkNo = S(59 + TeamSide) - Facing * 5
Guard.SparkNo = S(59 + TeamSide) - Facing * 5
SparkXY = 5, IFelse(Time = 2, -35, IFelse((AnimElem = 2, 0), -55, -80))
HitSound = S0, 3
GuardSound = S0, 0
Ground.Type = High
Ground.SlideTime = 30
Ground.HitTime = 30
Ground.Velocity = -1.25, -5
Guard.Velocity = -4
Air.Velocity = -1.25, -5
Fall = 1
Fall.YVelocity = -3
YAccel = .25
P2Facing = 1

[State 1011]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1012];豪昇龍拳 強
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1010
Juggle = 2

[State 1012]
Type = ChangeAnim
Trigger1 = AnimElem = 3, 26 || AnimElem = 5, 22
Value = 1010
Elem = IFelse((AnimElem = 3, 26), 4, 6)

[State 1012]
Type = NotHitBy
Trigger1 = AnimElem = 4, < 0
Value = SCA

[State 1012]
Type = VelSet
Trigger1 = AnimElem = 3
Persistent = 0
X = 4.5
Y = -6.5
[State 1012]
Type = StateTypeSet
Trigger1 = Time = 8
Value = A
[State 1012]
Type = StateTypeSet
Trigger1 = AnimElem = 6
Value = S
[State 1012]
Type = VelAdd
Trigger1 = AnimElem = 3, > 6 && AnimElem = 6, < 0
Y = .3
[State 1012]
Type = VelAdd
Trigger1 = AnimElem = 3, > 5 && AnimElem = 3, < 20
X = -.3
[State 1012]
Type = VelSet
Trigger1 = AnimElem = 3, 20
X = 0
[State 1012]
Type = VelSet
Trigger1 = AnimElem = 6
Y = 0
[State 1012]
Type = PosSet
Trigger1 = AnimElem = 6
Y = 0

[State 1012]
Type = PlaySnd
Trigger1 = AnimElem = 1
Value = 1, 2

[State 1012]
Type = HitDef
Trigger1 = Time = 2 || AnimElem = 2 || AnimElem = 3
Attr = S, SA
AnimType = Up
GuardFlag = M
Damage = 99, 12
GetPower = 0
GivePower = 0
PauseTime = 16, 14
SparkNo = S(59 + TeamSide) - Facing * 5
Guard.SparkNo = S(59 + TeamSide) - Facing * 5
SparkXY = 5, IFelse(Time = 2, -35, IFelse((AnimElem = 2, 0), -55, -80))
HitSound = S0, 3
GuardSound = S0, 0
Ground.Type = High
Ground.SlideTime = 30
Ground.HitTime = 30
Ground.Velocity = -1.25, -5
Guard.Velocity = -4
Air.Velocity = -1.25, -5
Fall = 1
Fall.YVelocity = -3
YAccel = .25
P2Facing = 1

[State 1012]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1200];ヨガテレポート 前 パンチ
Type = S
MoveType = I
Physics = S
Ctrl = 0
[State 1200]
Type = ChangeState
Trigger1 = Time = 0
Value = 1204
[StateDef 1201];ヨガテレポート 後 パンチ
Type = S
MoveType = I
Physics = S
Ctrl = 0
[State 1201]
Type = ChangeState
Trigger1 = Time = 0
Value = 1204
[StateDef 1202];ヨガテレポート 前 キック
Type = S
MoveType = I
Physics = S
Ctrl = 0
[State 1202]
Type = ChangeState
Trigger1 = Time = 0
Value = 1204
[StateDef 1203];ヨガテレポート 後 キック
Type = S
MoveType = I
Physics = S
Ctrl = 0
[State 1203]
Type = ChangeState
Trigger1 = Time = 0
Value = 1204

[StateDef 1204];ヨガテレポート中
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1200
PowerAdd = 48

[State 1204]
Type = Helper
Trigger1 = AnimElem = 18,-1 && (PrevStateNo = 1200 || PrevStateNo = 1202)
Pos = 0, 0
PosType = P1
StateNo = 1205
HelperType = Player

[State 1204]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 18
Value = 1 + (Time=0)*9, 0

[State 1204]
Type = NotHitBy
Trigger1 = AnimElem = 2,>=0 && AnimElem = 33,<0
Value = SCA

[State 1204]
Type = StateTypeSet
Trigger1 = AnimElem = 26
MoveType = I

[State 1204]
Type = PosAdd
Trigger1 = AnimElem = 18 && (PrevStateNo = 1200 || PrevStateNo = 1202)
X = Floor(P2Dist X) + IFelse(P2Dist X<160, IFelse(PrevStateNo=1200,100,200), 40)*Const(Size.XScale)
[State 1204]
Type = PosAdd
Trigger1 = AnimElem = 18 && (PrevStateNo = 1201 || PrevStateNo = 1203)
X = Floor(P2Dist X) - IFelse(PrevStateNo=1201,112,192)*Const(Size.XScale)

[State 1204]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1205];相手が画面端にいる時の押し返しヘルパー
Type = S
MoveType = I
Physics = N
Anim = 5111

[State 1205]
Type = ScreenBound
Trigger1 = 1
Value = 0

[State 1205]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 1205]
Type = PosSet
Trigger1 = 1
X = IFelse(Facing=1, 160, -160)

[State 1205]
Type = DestroySelf
Trigger1 = Time = 2

[StateDef 2500];ヨガスピン 弱
Type = A
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
Anim = 181
PowerAdd = 125

[State 2500]
Type = ChangeAnim
Trigger1 = Time = 2 || Time = 4 || Time = 6 || Time = [8,32]
Value = IFelse(Time=32, 471, 181)
Elem = IFelse(Time=2, 2, IFelse(Time=4, 3, IFelse(Time=6, 4, IFelse(Time=32, 1, 5))))

[State 2500]
Type = Explod
Trigger1 = Time = 6
Anim = 2500
Pos = 10, -70
PosType = P1
SprPriority = 10
OwnPal = 1
Scale = Const(Size.XScale), 1

[State 2500]
Type = AssertSpecial
Trigger1 = Time = [17,23]
Flag = Invisible

[State 2500]
Type = PosSet
Trigger1 = Time = 21
;X = IFelse(middle, 64)
Y = -97
[State 2500]
Type = Null;PosAdd
Trigger1 = Time = 21
X = IFelse(middle, 64, 129)*Const(Size.XScale)

[State 2500]
Type = VelSet
Trigger1 = Anim = 471 && AnimElem = 2
X = 3
Y = 4

[State 2500]
Type = VarSet
Trigger1 = Time = 0
V = 12
Value = 0
[State 2500]
Type = VarAdd
Trigger1 = MoveHit || MoveGuarded
V = 12
Value = 1
[State 2500]
Type = MoveHitReset
Trigger1 = MoveHit || MoveGuarded

[State 2500]
Type = ChangeAnim
Trigger1 = Var(12) = 3
Persistent = 0
Value = 41
Elem = 2
[State 2500]
Type = VelSet
Trigger1 = Var(12) = 3
Persistent = 0
X = -2.5
Y = -4
[State 2500]
Type = VelSet
Trigger1 = Var(12) = 3 && AnimElem = 4
X = -1
[State 2500]
Type = VelAdd
Trigger1 = Var(12) = 3
Y = .2

[State 2500]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 0

[State 2500]
Type = HitDef
Trigger1 = Anim = 471 && TimeMod = 3, 0 && Var(12) < 3
Attr = S, SA
HitFlag = MAF
GuardFlag = M
NumHits = 0
Damage = 66, 5
GetPower = 0
GivePower = 0
PauseTime = 6, 6
SparkNo = S62
Guard.SparkNo = S61
SparkXY = 0, -52
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 28
Ground.HitTime = 28
Ground.Velocity = IFelse(P2Dist X > -1, -1.5, 1.5)
Guard.Velocity = IFelse(P2Dist X > -1, -3, 3)
Air.Velocity = IFelse(P2Dist X > -1, -1.5, 1.5), -5
Fall.YVelocity = -3
YAccel = .25
P2Facing = IFelse(P2Dist X > -1, 1, -1)

[State 2500]
Type = ChangeState
Trigger1 = Time > 32 && Pos Y >= 0
Value = 52

[StateDef 2600];ヨガカレー 弱
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 2600
PowerAdd = 100

[State 2600]
Type = PlaySnd
Trigger1 = AnimElem = 4,2
Value = 10, 0

[State 2600]
Type = Projectile
trigger1 = AnimElem = 4,2
ProjAnim = 2601
ProjHitAnim = 2602
ProjID = 2
OffSet = -4*Const(Size.XScale), 3
Velocity = 2.5*Const(Size.XScale), -5
Accel = 0, .25
ProjHeightBound = -240, 94
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
ProjShadow = -1
Attr = S, SP
GuardFlag = MA
HitFlag = MA
Damage = 98, 14
GetPower = 0
GivePower = 0
PauseTime = 0, 6
SparkNo = S62
Guard.SparkNo = S60
SparkXY = 33, -104
HitSound = S2, 3
GuardSound = S2, 0
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 8
Ground.HitTime = 13
Air.HitTime = 20
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -5*Const(Size.XScale), -8
AirGuard.Velocity = -4*Const(Size.XScale), 0
Fall.YVelocity = 0
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2600]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1