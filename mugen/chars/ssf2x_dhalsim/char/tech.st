[StateDef 1000];ヨガファイヤー 弱
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 1000
PowerAdd = 60

[State 1000]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=5) + (AnimElem=4)*2

[State 1000]
Type = Helper
Trigger1 = AnimElem = 4,2
Name = "Guard_HANTEI"
ID = 1000
Pos = 40, 0
PosType = P1
StateNo = 1009

[State 1000]
Type = Projectile
trigger1 = AnimElem = 4,2 && EnemyNear,StateNo != [5020,5099]
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
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Air.Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 1004
P2Facing = 1
HitOnce = 1
ForceStand = 0
[State 1000]
Type = Projectile
Trigger1 = AnimElem = 4,2 && EnemyNear,StateNo = [5020,5099]
ProjAnim = 1001
ProjHitAnim = 1002
ProjCanselAnim = 1003
ProjRemAnim = 1004
ProjID = 1
OffSet = 0, 0
Velocity = 3*Const(Size.XScale)
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
Attr = S, SP
GuardFlag = M
HitFlag = M
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
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Air.Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 1004
P2Facing = 1
HitOnce = 1
ForceStand = 0

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
PowerAdd = 60

[State 1001]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=5) + (AnimElem=4)*2

[State 1001]
Type = Helper
Trigger1 = AnimElem = 4,2
Name = "Guard_HANTEI"
ID = 1000
Pos = 40, 0
PosType = P1
StateNo = 1009

[State 1001]
Type = Projectile
trigger1 = AnimElem = 4,2 && EnemyNear,StateNo != [5020,5099]
ProjAnim = 1001
ProjHitAnim = 1002
ProjID = 1
OffSet = 0, 0
Velocity = 5*Const(Size.XScale)
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
Attr = S, SP
AnimType = Hard
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
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 1005
P2Facing = 1
HitOnce = 1
[State 1001]
Type = Projectile
trigger1 = AnimElem = 4,2 && EnemyNear,StateNo = [5020,5099]
ProjAnim = 1001
ProjHitAnim = 1002
ProjID = 1
OffSet = 0, 0
Velocity = 5*Const(Size.XScale)
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
Attr = S, SP
AnimType = Hard
GuardFlag = M
HitFlag = M
Damage = 112, 28
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S1, 3
GuardSound = S0, 0
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 1005
P2Facing = 1
HitOnce = 1

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
PowerAdd = 60

[State 1002]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=5) + (AnimElem=4)*2

[State 1002]
Type = Helper
Trigger1 = AnimElem = 4,2
Name = "Guard_HANTEI"
ID = 1000
Pos = 40, 0
PosType = P1
StateNo = 1009

[State 1002]
Type = Projectile
trigger1 = AnimElem = 4,2 && EnemyNear,StateNo != [5020,5099]
ProjAnim = 1001
ProjHitAnim = 1002
ProjID = 1
OffSet = 0, 0
Velocity = 6.4*Const(Size.XScale)
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
Attr = S, SP
AnimType = Hard
GuardFlag = M
HitFlag = MA
Damage = 119, 35
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S1, 3
GuardSound = S0, 0
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 1005
P2Facing = 1
HitOnce = 1
[State 1002]
Type = Projectile
trigger1 = AnimElem = 4,2 && EnemyNear,StateNo = [5020,5099]
ProjAnim = 1001
ProjHitAnim = 1002
ProjID = 1
OffSet = 0, 0
Velocity = 6.4*Const(Size.XScale)
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
Attr = S, SP
AnimType = Hard
GuardFlag = M
HitFlag = M
Damage = 119, 35
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S1, 3
GuardSound = S0, 0
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 1005
P2Facing = 1
HitOnce = 1

[State 1002]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1004];炎上喰らい 地上
Type = U
MoveType = H
Physics = N
VelSet = 0, 0
SprPriority = -1

[State 1004]
Type = AssertSpecial
Trigger1 = 1
Flag = Invisible

[State 1004]
Type = ChangeAnim
Trigger1 = Time = 0
Value = IFelse((GetHitVar(AnimType) = [3, 5]), 5030, IFelse((GetHitVar(GroundType) = 1), 5000, 5010) + GetHitVar(AnimType))
[State 1004]
Type = ChangeAnim
Trigger1 = Time = 0 && (GetHitVar(AnimType) = [4, 5]) && (SelfAnimExist(5047 + GetHitVar(AnimType)))
Value = 5047 + GetHitVar(AnimType)
[State 1004]
Type = ChangeAnim
Trigger1 = Time > 0
Value = Anim

[State 1004]
Type = ForceFeedback
Trigger1 = Time= 0
Time = 15
WaveForm = SineSquare
Ampl = 140

[State 1004]
Type = HitVelSet
Trigger1 = HitShakeOver
Persistent = 0
X = 1

[State 1004]
Type = StateTypeSet
Trigger1 = Time = 11
Physics = S

[State 1004]
Type = ChangeAnim
Trigger1 = AnimTime = 0
Value = 5005 + GetHitVar(AnimType) + (GetHitVar(GroundType) = 2) * 10

[State 1004]
Type = VelMul
Trigger1 = Time >= GetHitVar(SlideTime) + 7
X = .6

[State 1004]
Type = VelSet
Trigger1 = HitOver
X = 0

[State 1004]
Type = DefenceMulSet
Trigger1 = HitOver
Value = 1

[State 1004]
Type = PosAdd
Trigger1 = !HitShakeOver
X = IFelse(TimeMod=2,0, 1, -1)

[State 1004]
Type = SelfState
Trigger1 = HitOver
Value = 0 + (StateType=C)*11
Ctrl = 1

[State 1004]
Type = ChangeState
Trigger1 = StateType = A || !Alive
Value = 1005

[StateDef 1005];炎上喰らい 吹っ飛び
Type = A
MoveType = H
Physics = N
VelSet = 0, 0
SprPriority = -1

[State 1005]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 1005]
Type = AssertSpecial
Trigger1 = 1
Flag = Invisible

[State 1005]
Type = ChangeAnim
Trigger1 = 1
IgnoreHitPause = 1
Value = 5030

[State 1005]
Type = HitVelSet
Trigger1 = HitShakeOver && PrevStateNo != 505
Persistent = 0
X = 1
Y = 1
[State 1005]
Type = VelAdd
Trigger1 = HitShakeOver
Y = GetHitVar(YAccel)

[State 1005]
Type = ChangeState
Trigger1 = BackEdgeDist < 2 && Time > 0 && PrevStateNo != 505 && (Name="SSF2Xリュウ"||(Name="SSF2Xダルシム"&&!Var(10)))
Value = 505

[State 1005]
Type = SelfState
Trigger1 = Vel Y > 0 &&  Pos Y >= 25
Value = 5100

[StateDef 1007];灼熱波動拳 喰らった相手 吹っ飛び 壁引っ掛かり後
Type = A
MoveType = H
Physics = N

[State 1007]
Type = AssertSpecial
Trigger1 = !SelfAnimExist(5400)
Flag = Invisible

[State 1007]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 1007]
Type = VelAdd
Trigger1 = 1
Y = .25

[State 1007]
Type = ChangeState
Trigger1 = Vel Y > 0 &&  Pos Y >= 25
Value = 1046

[StateDef 1008];VAMPIRE 炎上喰らい
Type = U
MoveType = H
Physics = N
VelSet = 0, 0
SprPriority = -1

[State 1008]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 1008]
Type = ChangeAnim2
Trigger1 = Time = 0
Value = 1105
[State 1008]
Type = ChangeAnim
Trigger1 = Time = 0 && AuthorName = "MASA" && SelfAnimExist(5400)
Value = 5400 + (SelfAnimExist(5401))
[State 1008]
Type = ChangeAnim
Trigger1 = Time = 0 && (Name="Demitri(DS)"||Name="Donovan"||Name="Hsien-ko"||Name="Zabel"||Name="Jon Talbain"||Name="Rikuo")
Value = 7697
[State 1008]
Type = ChangeAnim
Trigger1 = Time = 0 && (Name="Anakaris By Third")
Value = 5621

[State 1008]
Type = PalFX
Trigger1 = Anim = 1105
Time = 2
Mul = 100,100,100
Add = 100,100,100

[State 1008]
Type = ForceFeedback
Trigger1 = Anim = 5000 || Anim = 5010
Persistent = 0
Time = 6
WaveForm = Square
[State 1008]
Type = ForceFeedback
Trigger1 = Anim = 5001 || Anim = 5011
Persistent = 0
Time = 8
WaveForm = SineSquare
Ampl = 110, -1, -.3
[State 1008]
Type = ForceFeedback
Trigger1 = Anim = 5012 || Anim = 5002 || Anim = 5030
Persistent = 0
Time = 15
WaveForm = SineSquare
Ampl = 140

[State 1008]
Type = VelSet
Trigger1 = Time = 9
X = -1
Y = -3

[State 1008]
Type = VelAdd
Trigger1 = Time > 9
Y = .125 + (Time>49)*.3

[State 1008]
Type = VelSet
Trigger1 = Time = [35,49]
X = 0
Y = 0

[State 1008]
Type = SelfState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 5100

[StateDef 1009];飛び道具 CPUに防御させ易くするヘルパー
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 99

[State 1009]
Type = VelSet
trigger1 = Time = 0
X = (3 + (Parent,StateNo=1001)*2 + (Parent,StateNo=1002)*3.4)*Const(Size.XScale)

[State 1009]
Type = ChangeState
Trigger1 = Parent,ProjHitTime1 = 1
Value = 1103
[State 1009]
Type = ChangeState
Trigger1 = Parent,ProjHitTime(1) = 1
Value = 1103

[State 1009]
Type = VarAdd
Trigger1 = Parent,NumProjID(0) = 0 && Parent,NumProjID(1) = 0 && Parent,NumProjID(2) = 0
V = 0
Value = 1

[State 1009]
Type = DestroySelf
Trigger1 = Var(0) > 3

[StateDef 1100];ヨガフレイム 弱
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1100
PowerAdd = 60+Var(10)*40

[State 1100]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 6 || AnimElem = 8
Value = 10 - (AnimElem=6)*9, 0 + (AnimElem=6||(AnimElem=8)&&!Var(10))*2 + (Time=0&&Var(10))*4 + (AnimElem=8&&Var(10))*5

[State 1100]
Type = Helper
Trigger1 = AnimElem = 6, 2
Pos = 0, 0
PosType = P1
StateNo = 1101
OwnPal = 1

[State 1100]
Type = ChangeAnim
Trigger1 = AnimElem = 8,>20 && NumProjID(2) = 0 && NumProjID(3) = 0
Persistent = 0
Value = 1104

[State 1100]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1101];ヨガフレイム ヘルパー
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
Type = AssertSpecial
Trigger1 = 1
Flag = NoShadow

[State 1101]
Type = Helper
Trigger1 = Parent,ProjHitTime(2) = 1 || Parent,ProjHitTime(3) = 1
ID = 1
Pos = 25, -10
PosType = P1
StateNo = 1103
OwnPal = 1

[State 1101]
Type = ChangeState
Trigger1 = Time > 9 && Parent,NumProjID(2) = 0 && Parent,NumProjID(3) = 0
Value = 1102
[State 1101]
Type = DestroySelf
Trigger1 = Parent,StateNo != [1100,1199]

[State 1101]
Type = Projectile
Trigger1 = ((Parent,StateNo=1100&&Time=[9,40]) || (Parent,StateNo=1104&&Time=[9,56]) || ((Parent,StateNo=[1105,1106])&&Time=[9,72])) && NumProjID(2)=0&&NumProjID(3)=0 &&  EnemyNear,StateNo = [5020,5099]
ProjAnim = 1103 + (AnimElem=12,>0)*3
ProjID = 2 + Parent,Var(10)
OffSet = 0, 0
Velocity = 0
ProjRemoveTime = 1
ProjPriority = 1
Attr = S, SP
GuardFlag = M
HitFlag = M
Damage = 182 + (Parent,StateNo=1104)*9 + (Parent,StateNo=1105)*49 + (Parent,StateNo=1106)*70, 42 - (Parent,StateNo=1105)*14 + (Parent,StateNo=1106)*14
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = S1002
Guard.SparkNo = S1002
SparkXY = 25, 0
HitSound = S1 + Parent,Var(10)*2, 3 - Parent,Var(10)*3
GuardSound = S0 + Parent,Var(10)*2, 0
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = (-9 +Parent,Var(10))*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 1005 + Parent,Var(10)*3
P2Facing = 1
HitOnce = 1
[State 1101]
Type = Projectile
Trigger1 = ((Parent,StateNo=1100&&Time=[9,40]) || (Parent,StateNo=1104&&Time=[9,56]) || ((Parent,StateNo=[1105,1106])&&Time=[9,72])) && NumProjID(2)=0&&NumProjID(3)=0 &&  EnemyNear,StateNo != [5020,5099]
ProjAnim = 1103 + (AnimElem=12,>0)*3
ProjID = 2 + Parent,Var(10)
OffSet = 0, 0
Velocity = 0
ProjRemoveTime = 1
ProjPriority = 1
Attr = S, SP
GuardFlag = M
HitFlag = MA
Damage = 182 + (Parent,StateNo=1104)*9 + (Parent,StateNo=1105)*49 + (Parent,StateNo=1106)*70, 42 - (Parent,StateNo=1105)*14 + (Parent,StateNo=1106)*14
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = S1002
Guard.SparkNo = S1002
SparkXY = 25, 0
HitSound = S1 + Parent,Var(10)*2, 3 - Parent,Var(10)*3
GuardSound = S0 + Parent,Var(10)*2, 0
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = (-9 +Parent,Var(10))*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 1005 + Parent,Var(10)*3
P2Facing = 1
HitOnce = 1

[StateDef 1102];ヨガフレイム ヘルパー消滅
Type = S
MoveType = I
Physics = N
Anim = 1102

[State 1102]
Type = Helper
Trigger1 = (Parent,ProjHit2 = 1 || Parent,ProjHit3 = 1) && !NumHelper(1) && Time = 0
ID = 1
Pos = 25, -10
PosType = P1
StateNo = 1103
OwnPal = 1

[State 1102]
Type = AssertSpecial
Trigger1 = 1
Flag = NoShadow

[State 1102]
Type = DestroySelf
Trigger1 = AnimTime = 0

[StateDef 1103];ヨガフレイム 炎上喰らいヘルパー
Type = A
MoveType = I
Physics = N
Anim = 99

[State 1103]
Type = Turn
Trigger1 = Time = 0

[State 1103]
Type = RemoveExplod
Trigger1 = ((P2StateNo=1005&&EnemyNear,Time=1) || Time < 15 || P2StateNo != [505,2002]) && NumExplod(1)
ID = 1
[State 1103]
Type = Explod
Trigger1 = !NumExplod(1) && ((P2StateNo=1005&&EnemyNear,Time=1) || Time < 15) && !Root,Var(10)
Anim = 5400 - Root,Var(17)*4390 + (P2StateType=S) + (P2StateType=C)*2
ID = 1
Pos = 0, 0
PosType = P2
BindTime = -1
SprPriority = 0
RemoveTime = -1
RemoveOnGetHit = 1
Shadow = -1
OwnPal = 1
Scale = Const(Size.XScale), 1

[State 1103]
Type = Explod
Trigger1 = Time = 0 && Root,Var(10)
ID = 2
Anim = 63
Pos = 0, -56
PosType = P2
BindTime = -1
SprPriority = 0
OwnPal = 1
Scale = Const(Size.XScale), 1

[State 1103]
Type = PosAdd
Trigger1 = 1
X = P2Dist X
Y = P2Dist Y

[State 1103]
Type = DestroySelf
Trigger1 = !NumExplod(1)

[StateDef 1104];ヨガフレイム 中
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1100
PowerAdd = 60+Var(10)*40

[State 1104]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 6 || AnimElem = 8
Value = 10 - (AnimElem=6)*9, 0 + (AnimElem=6||(AnimElem=8)&&!Var(10))*2 + (Time=0&&Var(10))*4 + (AnimElem=8&&Var(10))*5

[State 1104]
Type = Helper
Trigger1 = AnimElem = 6, 2
Pos = 0, 0
PosType = P1
StateNo = 1101
OwnPal = 1

[State 1104]
Type = ChangeAnim
Trigger1 = AnimElem = 8,>20 && NumProjID(2) = 0 && NumProjID(3) = 0
Persistent = 0
Value = 1104

[State 1104]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1105];ヨガフレイム 強
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1100
PowerAdd = 60+Var(10)*40

[State 1105]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 6 || AnimElem = 8
Value = 10 - (AnimElem=6)*9, 0 + (AnimElem=6||(AnimElem=8)&&!Var(10))*2 + (Time=0&&Var(10))*4 + (AnimElem=8&&Var(10))*5

[State 1105]
Type = Helper
Trigger1 = AnimElem = 6, 2
Pos = 0, 0
PosType = P1
StateNo = 1101
OwnPal = 1

[State 1105]
Type = ChangeAnim
Trigger1 = AnimElem = 8,>20 && NumProjID(2) = 0 && NumProjID(3) = 0
Persistent = 0
Value = 1104

[State 1105]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1106];ヨガフレイム スペシャル
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1100
PowerAdd = -1000

[State 1106]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 6 || AnimElem = 8
Value = 10 - (AnimElem=6)*9, 0 + (AnimElem=6||(AnimElem=8)&&!Var(10))*2 + (Time=0&&Var(10))*4 + (AnimElem=8&&Var(10))*5

[State 1106]
Type = Helper
Trigger1 = AnimElem = 6,2
Pos = 0, 0
PosType = P1
StateNo = 1101
OwnPal = 1

[State 1106]
Type = ChangeAnim
Trigger1 = AnimElem = 8,>20 && NumProjID(2) = 0 && NumProjID(3) = 0
Persistent = 0
Value = 1104

[State 1106]
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
Trigger1 = Time = 1
Value = 1204
[StateDef 1201];ヨガテレポート 後 パンチ
Type = S
MoveType = I
Physics = S
Ctrl = 0
[State 1201]
Type = ChangeState
Trigger1 = Time = 1
Value = 1204
[StateDef 1202];ヨガテレポート 前 キック
Type = S
MoveType = I
Physics = S
Ctrl = 0
[State 1202]
Type = ChangeState
Trigger1 = Time = 1
Value = 1204
[StateDef 1203];ヨガテレポート 後 キック
Type = S
MoveType = I
Physics = S
Ctrl = 0
[State 1203]
Type = ChangeState
Trigger1 = Time = 1
Value = 1204

[StateDef 1204];ヨガテレポート中
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1200
PowerAdd = 60 + Var(10)*40 - Var(14)*1100

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
Type = VarSet
Trigger1 = Time = 0
V = 15
Value = (Var(14)&&Power=0)

[State 1204]
Type = Helper
Trigger1 = AnimElem = 19 && Var(15)
Pos = Floor(100*Const(Size.XScale)), 0
PosType = P1
StateNo = 1206
[State 1204]
Type = Helper
Trigger1 = AnimElem = 19 && Var(15)
Pos = Floor(-100*Const(Size.XScale)), 0
PosType = P1
StateNo = 1206
[State 1204]
Type = Helper
Trigger1 = AnimElem = 19 && Var(15)
Pos = Floor(200*Const(Size.XScale)), 0
PosType = P1
StateNo = 1206
[State 1204]
Type = Helper
Trigger1 = AnimElem = 19 && Var(15)
Pos = Floor(-200*Const(Size.XScale)), 0
PosType = P1
StateNo = 1206
[State 1204]
Type = Helper
Trigger1 = AnimElem = 19 && Var(15)
Pos = Floor(300*Const(Size.XScale)), 0
PosType = P1
StateNo = 1206
[State 1204]
Type = Helper
Trigger1 = AnimElem = 19 && Var(15)
Pos = Floor(-300*Const(Size.XScale)), 0
PosType = P1
StateNo = 1206

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

[StateDef 1206];ヨガテレポートスペシャル 相手惑わしヘルパー
Type = S
MoveType = I
Physics = N
Ctrl = 0

[State 1206]
Type = ChangeAnim
Trigger1 = Time = 0 || AnimElem = 30
Value = 1200
Elem = 6 + (Time=0)*13

[State 1206]
Type = PlayerPush
Trigger1 = 1
Value = 0

[State 1206]
Type = ScreenBound
Trigger1 = 1
Value = 0

[State 1206]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 1206]
Type = DestroySelf
Trigger1 = AnimElem = 17

[StateDef 1300];ヨガブラスト 弱
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1300
PowerAdd = 60+Var(10)*40

[State 1300]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=4||(AnimElem=5)&&!Var(10))*2 + (Time=0&&Var(10))*4 + (AnimElem=5&&Var(10))*5

[State 1300]
Type = Helper
Trigger1 = AnimElem = 5
Pos = 0, 0
PosType = P1
StateNo = 1301
OwnPal = 1

[State 1300]
Type = ChangeAnim
Trigger1 = AnimElem = 5,>20 && NumProjID(2) = 0 && NumProjID(3) = 0
Persistent = 0
Value = 1305

[State 1300]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1301];ヨガブラスト ヘルパー
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1301
SprPriority = 99

[State 1301]
Type = BindToParent
Trigger1 = 1
Pos = 0, 0

[State 1301]
Type = AssertSpecial
Trigger1 = 1
Flag = NoShadow

[State 1301]
Type = Helper
Trigger1 = Parent,ProjHitTime(2) = 1 || Parent,ProjHitTime(3) = 1
Pos = 25, -10
PosType = P1
StateNo = 1103
OwnPal = 1

[State 1301]
Type = ChangeState
Trigger1 = Time > 1 && Parent,NumProjID(2) = 0 && Parent,NumProjID(3) = 0
Value = 1302
[State 1301]
Type = DestroySelf
Trigger1 = Parent,StateNo != [1300,1399]

[State 1301]
Type = Projectile
Trigger1 = ((Parent,StateNo=1300&&Time=[1,16]) || (Parent,StateNo=1304&&Time=[1,21]) || ((Parent,StateNo=[1305,1306])&&Time=[1,21])) && NumProjID(2)=0&&NumProjID(3)=0 &&  EnemyNear,StateNo = [5020,5099]
ProjAnim = 1304
ProjID = 2 + Parent,Var(10)
OffSet = 0, 0
Velocity = 0
ProjRemoveTime = 1
ProjPriority = 1
Attr = S, SP
GuardFlag = M
HitFlag = M
Damage = 133 - (Parent,StateNo=1300)*21 + (Parent,StateNo=1306)*49, 35 - (Parent,StateNo=1300)*7 + (Parent,StateNo=1306)*14
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = S1002
Guard.SparkNo = S1002
SparkXY = -30, -80
HitSound = S1 + Parent,Var(10)*2, 3 - Parent,Var(10)*3
GuardSound = S0 + Parent,Var(10)*2, 0
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = (-9 +Parent,Var(10))*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 1005 + Parent,Var(10)*3
P2Facing = 1
HitOnce = 1
[State 1301]
Type = Projectile
Trigger1 = ((Parent,StateNo=1300&&Time=[1,16]) || (Parent,StateNo=1304&&Time=[1,21]) || ((Parent,StateNo=[1305,1306])&&Time=[1,21])) && NumProjID(2)=0&&NumProjID(3)=0 &&  EnemyNear,StateNo != [5020,5099]
ProjAnim = 1304
ProjID = 2 + Parent,Var(10)
OffSet = 0, 0
Velocity = 0
ProjRemoveTime = 1
ProjPriority = 1
Attr = S, SP
GuardFlag = M
HitFlag = MA
Damage = 133 - (Parent,StateNo=1300)*21 + (Parent,StateNo=1306)*49, 35 - (Parent,StateNo=1300)*7 + (Parent,StateNo=1306)*14
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = S1002
Guard.SparkNo = S1002
SparkXY = -30, -80
HitSound = S1 + Parent,Var(10)*2, 3 - Parent,Var(10)*3
GuardSound = S0 + Parent,Var(10)*2, 0
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = (-9 +Parent,Var(10))*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 1005 + Parent,Var(10)*3
P2Facing = 1
HitOnce = 1

[StateDef 1302];ヨガブラスト ヘルパー消滅
Type = S
MoveType = I
Physics = N
Anim = 1303 - (Parent,StateNo=1300)

[State 1302]
Type = AssertSpecial
Trigger1 = 1
Flag = NoShadow

[State 1302]
Type = DestroySelf
Trigger1 = AnimTime = 0

[StateDef 1304];ヨガブラスト 中
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1300
PowerAdd = 60+Var(10)*40

[State 1304]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=4||(AnimElem=5)&&!Var(10))*2 + (Time=0&&Var(10))*4 + (AnimElem=5&&Var(10))*5

[State 1304]
Type = Helper
Trigger1 = AnimElem = 5,-1
Pos = 0, 0
PosType = P1
StateNo = 1301
OwnPal = 1

[State 1304]
Type = ChangeAnim
Trigger1 = AnimElem = 5,>20 && NumProjID(2) = 0 && NumProjID(3) = 0
Persistent = 0
Value = 1305

[State 1304]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1305];ヨガブラスト 強
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1300
PowerAdd = 60+Var(10)*40

[State 1305]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=4||(AnimElem=5)&&!Var(10))*2 + (Time=0&&Var(10))*4 + (AnimElem=5&&Var(10))*5

[State 1305]
Type = Helper
Trigger1 = AnimElem = 5,-1
Pos = 0, 0
PosType = P1
StateNo = 1301
OwnPal = 1

[State 1305]
Type = ChangeAnim
Trigger1 = AnimElem = 5,>20 && NumProjID(2) = 0 && NumProjID(3) = 0
Persistent = 0
Value = 1305

[State 1305]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1306];ヨガブラスト スペシャル
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1300
PowerAdd = -1000

[State 1306]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 4 || AnimElem = 5
Value = 10 - (AnimElem=4)*9, 0 + (AnimElem=4||(AnimElem=5)&&!Var(10))*2 + (Time=0&&Var(10))*4 + (AnimElem=5&&Var(10))*5

[State 1306]
Type = Helper
Trigger1 = AnimElem = 5,-1
Pos = 0, 0
PosType = P1
StateNo = 1301
OwnPal = 1

[State 1306]
Type = ChangeAnim
Trigger1 = AnimElem = 5,>20 && NumProjID(2) = 0 && NumProjID(3) = 0
Persistent = 0
Value = 1305

[State 1306]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 2000];ヨガインフェルノ
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 2000
PowerAdd = -1000

[State 2000]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 5 || AnimElem = 6 || AnimElem = 8
Value = 10 - (AnimElem=6)*9 - (AnimElem=5)*10, 0 + (AnimElem=6||AnimElem=8)*2 + (AnimElem=5)*19

[State 2000]
Type = Pause
Trigger1 = Time = 0
Time = 26
MoveTime = 26

[State 2000]
Type = AfterImage
Trigger1 = AnimElem = 6,-2
Time = 80
Trans = None
FrameGap = 3
Length = 10
PalColor = 0
PalBright = 50,50,125
PalContrast = 135,135,195
PalAdd = 30,30,-20
[State 2000]
Type = AfterImage
Trigger1 = Anim = 1104 || AnimTime = 0
Time = 0

[State 2000]
Type = Explod
Trigger1 = AnimElem = 5,1
Anim = 93
Pos = 0, 0
PosType = P1
OwnPal = 1
SprPriority = 10
PauseMoveTime = -1
BindTime = -1
Scale = Const(Size.XScale), 1

[State 2000]
Type = Helper
Trigger1 = AnimElem = 6,2
Pos = 0, 0
PosType = P1
StateNo = 2001
OwnPal = 1

[State 2000]
Type = NotHitBy
Trigger1 = AnimElem = 8,<20 || NumProjID(2) = 0
Value = SCA

[State 2000]
Type = ChangeAnim
Trigger1 = AnimElem = 8,>20 && Var(15)
Persistent = 0
Value = 1104

[State 2000]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 2001];ヨガインフェルノ ヘルパー
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 1101
SprPriority = 99

[State 2001]
Type = BindToParent
Trigger1 = 1
Pos = 0, 0

[State 2001]
Type = AssertSpecial
Trigger1 = 1
Flag = NoShadow

[State 2001]
Type = Helper
Trigger1 = Parent,ProjHitTime(2) = 1 && !NumHelper(1)
ID = 1
Pos = 25, -10
PosType = P1
StateNo = 1103
OwnPal = 1

[State 2001]
Type = VarSet
Trigger1 = Parent,ProjContactTime(2) = 1 || Time = 0
V = 0
Value = 1 - (Time=0)
[State 2001]
Type = VarAdd
Trigger1 = Var(0) > 0
V = 0
Value = 1

[State 2001]
Type = VarSet
Trigger1 = Time = 0
V = 1
Value = 0
[State 2001]
Type = VarAdd
Trigger1 = Parent,ProjContactTime(2) = 1
V = 1
Value = 1

[State 2001]
Type = VarAdd
Trigger1 = Parent,ProjContact2 = 1 &&  Var(1) = 0
Persistent = 0
V = 1
Value = 1

[State 2001]
Type = ParentVarSet
Trigger1 = Time = 0 || Var(1) = 5
V = 15
Value = 1 - (Time=0)

[State 2001]
Type = DestroySelf
Trigger1 = Parent,StateNo != 2000

[State 2001]
Type = Projectile
Trigger1 = RoundState = 2 && Time > 0 && ((Var(1)=1&&Var(0)=13)||((Var(1)=[2,3])&&Var(0)=12)||(Var(1)=4&&Var(0)=11) || Var(0) = 0) && Var(1) < 1 && Parent,NumProjID(2) = 0 &&  EnemyNear,StateNo = [5020,5099]
ProjAnim = 2001 + (AnimElem=12,>0)
ProjID = 2
OffSet = 0, 0
Velocity = 0
ProjRemoveTime = 1
ProjPriority = 1
Attr = S, HP
GuardFlag = M
HitFlag = M
Damage = 112, 21
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = S(IFelse(Var(1)=4, 1002, 54+TeamSide))
Guard.SparkNo = S(IFelse(Var(1)=4, 1002, 54+TeamSide))
SparkXY = 25 - (AnimElem=3,<0)*10 + (Var(1)<4)*60, 0 - (AnimElem=3,<0)*10 - (Var(1)<4)*50
HitSound = S1, 3
GuardSound = S0, 0
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = (-1.5 -(Var(1)>3)*1.5)*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 2002 - (Var(1)>3)*997
P2Facing = 1
HitOnce = 1
ForceStand = 0
[State 2001]
Type = Projectile
Trigger1 = RoundState = 2 && Time > 0 && ((Var(1)=1&&Var(0)=13)||((Var(1)=[2,3])&&Var(0)=12)||(Var(1)=4&&Var(0)=11) || Var(0) = 0) && Var(1) < 5 && Parent,NumProjID(2) = 0 &&  EnemyNear,StateNo != [5020,5099]
ProjAnim = 2001 + (AnimElem=12,>0)
ProjID = 2
OffSet = 0, 0
Velocity = 0
ProjRemoveTime = 1
ProjPriority = 1
Attr = S, HP
GuardFlag = M
HitFlag = MAF
Damage = 112, 21
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = S(IFelse(Var(1)=4, 1002, 54+TeamSide))
Guard.SparkNo = S(IFelse(Var(1)=4, 1002, 54+TeamSide))
SparkXY = 25 - (AnimElem=3,<0)*10 + (Var(1)<4)*60, 0 - (AnimElem=3,<0)*10 - (Var(1)<4)*50
HitSound = S1, 3
GuardSound = S0, 0
AnimType = Hard
Ground.Type = High
Ground.SlideTime = 43
Ground.HitTime = 43
Ground.Velocity = (-1.5 -(Var(1)>3)*1.5)*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
YAccel = .25
Fall.YVelocity = -4
P2StateNo = 2002 - (Var(1)>3)*997
P2Facing = 1
HitOnce = 1
ForceStand = 0

[State 2001]
Type = ChangeState
Trigger1 = Time = 72 || Var(1) = 5
Value = 1102

[StateDef 2002];炎上喰らい ヨガインフェルノ
Type = U
MoveType = H
Physics = N
VelSet = 0, 0
SprPriority = -1

[State 2002]
Type = AssertSpecial
Trigger1 = 1
Flag = Invisible

[State 2002]
Type = PosAdd
Trigger1 = Time = 10 && StateType = A && Anim = 5050
Y = -16
[State 2002]
Type = VarSet
Trigger1 = Time = 10
sysvar(0) = 1

[State 2002]
Type = ChangeAnim
Trigger1 = Time = 0
Value = IFelse(StateType=A, 5030 +(Anim=5030||Anim=5050)*20, 5002 +(StateType=C)*20)
[State 2002]
Type = ChangeAnim
Trigger1 = Time > 0
Value = Anim

[State 2002]
Type = ForceFeedback
Trigger1 = Time = 0
Time = 15
WaveForm = SineSquare
Ampl = 140

[State 2002]
Type = HitVelSet
Trigger1 = HitShakeOver
Persistent = 0
X = 1

[State 2002]
Type = StateTypeSet
Trigger1 = Time = 11
Physics = S

[State 2002]
Type = PosAdd
Trigger1 = !HitShakeOver
X = IFelse(TimeMod=2,0, 1, -1)

[State 2002]
Type = ChangeState
Trigger1 = Time = 15
Value = 1005

[StateDef 2100];ヨガインドプレジャー
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
Anim = 2100
PowerAdd = -1000

[State 2100]
Type = VelSet
Trigger1 = Time = 2
X = 3*Const(Size.XScale)

[State 2100]
Type = HitDef
Trigger1 = Time = 0
Attr = S, HT
HitFlag = M-
Priority = 0, Miss
Damage = 0
GetPower = 0
GivePower = 0
NumHits = 0
SparkNo = -1
HitSound = -1
AnimType = Light
Ground.Velocity = -3*Const(Size.XScale), -6
Fall.YVelocity = -3
ID = 1
P1StateNo = 2101
P2StateNo = 2102
P1Facing = 1
P2Facing = 1

[State 2100]
Type = ChangeState
Trigger1 = Time = 29
Value = 101

[StateDef 2101];ヨガインドプレジャー中
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 2101

[State 2101]
Type = HitAdd
Trigger1 = AnimElem=3||AnimElem=6||AnimElem=9||AnimElem=12||AnimElem=15||AnimElem=18||AnimElem=21||AnimElem=24||AnimElem=27,1||AnimElem=29,1||AnimElem=31,1||AnimElem=35,1
Value = 1

[State 2101]
Type = TargetBind
Trigger1 =  Time = 0 || AnimElem = 2
ID = 1
Pos = Floor(43 + (Time=0)*47)*Const(Size.XScale), 0

[State 2101]
Type = PlaySnd
Trigger1 = AnimElem=3||AnimElem=6||AnimElem=9||AnimElem=12||AnimElem=15||AnimElem=18||AnimElem=21||AnimElem=24
Value = 10, 4
[State 2101]
Type = PlaySnd
Trigger1 = AnimElem=3||AnimElem=6||AnimElem=9||AnimElem=12||AnimElem=15||AnimElem=18||AnimElem=21||AnimElem=24||AnimElem=27,1||AnimElem=29,1||AnimElem=31,1||AnimElem=35,1
Value = 2, 3

[State 2101]
Type = Explod
Trigger1 = AnimElem=3||AnimElem=6||AnimElem=9||AnimElem=12||AnimElem=15||AnimElem=18||AnimElem=21||AnimElem=24||AnimElem=27,1||AnimElem=29,1||AnimElem=31,1||AnimElem=35,1
Anim = 62
Pos = Floor(IFelse((AnimElem=26,<0),30*Const(Size.XScale),30+P2Dist X/3)), -90 + (AnimElem=27,1)*26 + (AnimElem=31,1||AnimElem=35,1)*20
PosType = P1
SprPriority = 10
OwnPal = 1
Scale = Const(Size.XScale), 1

[State 2101]
Type = Helper
Trigger1 = AnimTime = -20
Pos = 180, -240
PosType = P1
StateNo = 2104
OwnPal = 1

[State 2101]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 2102];ヨガインドプレジャられる相手
Type = S
MoveType = H
Physics = S
VelSet = 0, 0
SprPriority = -1

[State 2102]
Type = LifeAdd
Trigger1 = AnimElem = 47
Value = -273

[State 2102]
Type = ChangeAnim2
Trigger1 = Time = 0
Value = 2102

[State 2102]
Type = ChangeState
Trigger1 = AnimElem = 47,11
Value = 2103

[StateDef 2103];ヨガインドプレジャられる相手解放
Type = A
MoveType = H
Physics = N

[State 2103]
Type = HitVelSet
Trigger1 = Time = 0 && PrevStateNo != 505
X = 1
Y = 1
[State 2103]
Type = VelAdd
Trigger1 = 1
Y = .4

[State 2103]
Type = SelfState
Trigger1 = Vel Y > 0 && Pos Y > 24
Value = 5100

[StateDef 2104];ヨガインドプレジャー 象
Type = A
MoveType = A
Physics = N
Ctrl = 0
Anim = 2103

[State 2104]
Type = PosSet
Trigger1 = FrontEdgeDist < 30 && Time = 0
X = 130*Facing

[State 2104]
Type = Explod
Trigger1 = Time = 0
ID = 100
Anim = 2104
Pos = 0, 0
PosType = P1
BindTime = -1
SprPriority = 10
Scale = Const(Size.XScale), 1

[State 2104]
Type = ModifyExplod
Trigger1 = Var(1) > 0
IgnoreHitPause = 1
ID = 100
Anim = 2104
Pos = 0, 0
PosType = P1
BindTime = -1
SprPriority = 10
Scale=(((Var(1)=[1,7])||Var(1)=[12,15])*1.7+(Var(1)=[8,11])*1.8+(Var(1)=[16,19])*1.3+(Var(1)=[20,24])*.9+(Var(1)>24))*Const(Size.XScale),((Var(1)=[1,7])||Var(1)=[12,15])*.5+(Var(1)=[8,11])*.4+(Var(1)=[16,19])*.8+(Var(1)=[20,24])*1.1+(Var(1)>24)

[State 2104]
Type = RemoveExplod
Trigger1 = (MoveContact && ScreenPos Y < -30) || ScreenPos Y > 400
ID = 100

[State 2104]
Type = VarAdd
Trigger1 = MoveContact
IgnoreHitPause = 1
V = 1
Value = 1

[State 2104]
Type = VelSet
Trigger1 = Time = 0 || Var(1)=1 || Var(1) = 29
IgnoreHitPause = 1
Y = -8.5 + (Time=0)*17 + (Var(1)=1)*8.5

[State 2104]
Type = PlaySnd
Trigger1 = Time = 0
Value = 1, 1

[State 2104]
Type = HitDef
Trigger1 = Time = 0
Attr = A, HA
HitFlag = MAFD
Damage = 35, 7
GivePower = 0
PauseTime = 20, 7
SparkNo = S62
Guard.SparkNo = S61
SparkXY = 0, -16
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Down.HitTime = 20
Ground.Velocity = -3*Const(Size.XScale), -6
Air.Velocity = -3*Const(Size.XScale), -6
Down.Velocity = 0, 0
Ground.CornerPush.VelOff = 0
Fall = 1
Fall.Recover = 0
YAccel = .4
P2Facing = 1

[State 2104]
Type = DestroySelf
Trigger1 = (MoveContact && ScreenPos Y < -30) || ScreenPos Y > 400

[StateDef 2500];ヨガスピン 弱
Type = A
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
Anim = 181
PowerAdd = 125 - (Var(14))*1125

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
Type = NotHitBy
Trigger1 = Time = [17,23]
Value = SCA

[State 2500]
Type = PosSet
Trigger1 = Time = 21
;X = IFelse(middle, 64)
Y = -97

[State 2500]
Type = VelSet
Trigger1 = Anim = 471 && AnimElem = 2
X = 3
Y = 4

[State 2500]
Type = VarSet
Trigger1 = Time = 0
V = 15
Value = (Var(14)&&Power=0)

[State 2500]
Type = VarSet
Trigger1 = Time = 0
V = 12
Value = 0
[State 2500]
Type = VarAdd
Trigger1 = (MoveHit || MoveGuarded) && !Var(15)
V = 12
Value = 1
[State 2500]
Type = MoveHitReset
Trigger1 = (MoveHit || MoveGuarded) && !Var(15)

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
Value = 10, 4

[State 2500]
Type = HitDef
Trigger1 = Anim = 471 && Pos Y < EnemyNear,Const(Size.Mid.Pos.Y)/2 && ((TimeMod = 2,0 && Var(12) < 3) || !MoveContact)
Attr = S, SA
HitFlag = MAF
GuardFlag = M
Damage = 66 - Var(15)*38, 5
GetPower = 37 - Var(15)*37
GivePower = 0
PauseTime = 6, 7
SparkNo = S62
Guard.SparkNo = S61
SparkXY = -30+Floor(P2Dist X)/2, -42
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
Ground.CornerPush.VelOff = 0
YAccel = .4
P2Facing = 1
ForceStand = 1
[State 2500]
Type = HitDef
Trigger1 = Anim = 471 && Pos Y > EnemyNear,Const(Size.Mid.Pos.Y)/2 && ((TimeMod = 2,0 && Var(12) < 3) || !MoveContact)
Attr = S, SA
HitFlag = MAF
GuardFlag = M
Damage = 66 - Var(15)*38, 5
GetPower = 37 - Var(15)*37
GivePower = 0
PauseTime = 6, 7
SparkNo = S62
Guard.SparkNo = S61
SparkXY = -30+Floor(P2Dist X)/2, -42
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
Ground.CornerPush.VelOff = 0
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2500]
Type = ChangeState
Trigger1 = Time > 32 && Pos Y >= 0
Value = 52

[StateDef 2501];ヨガスピン 中
Type = A
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
Anim = 181
PowerAdd = 125 - (Var(14))*1125

[State 2501]
Type = ChangeAnim
Trigger1 = Time = 2 || Time = 4 || Time = 6 || Time = [8,32]
Value = IFelse(Time=32, 471, 181)
Elem = IFelse(Time=2, 2, IFelse(Time=4, 3, IFelse(Time=6, 4, IFelse(Time=32, 1, 5))))

[State 2501]
Type = Explod
Trigger1 = Time = 6
Anim = 2500
Pos = 10, -70
PosType = P1
SprPriority = 10
OwnPal = 1
Scale = Const(Size.XScale), 1

[State 2501]
Type = AssertSpecial
Trigger1 = Time = [17,23]
Flag = Invisible

[State 2501]
Type = NotHitBy
Trigger1 = Time = [17,23]
Value = SCA

[State 2501]
Type = PosSet
Trigger1 = Time = 21
Y = -97
[State 2501]
Type = PosAdd
Trigger1 = Time = 21
X = 64*Const(Size.XScale)

[State 2501]
Type = VelSet
Trigger1 = Anim = 471 && AnimElem = 2
X = 3
Y = 4

[State 2501]
Type = VarSet
Trigger1 = Time = 0
V = 15
Value = (Var(14)&&Power=0)

[State 2501]
Type = VarSet
Trigger1 = Time = 0
V = 12
Value = 0
[State 2501]
Type = VarAdd
Trigger1 = (MoveHit || MoveGuarded) && !Var(15)
V = 12
Value = 1
[State 2501]
Type = MoveHitReset
Trigger1 = (MoveHit || MoveGuarded) && !Var(15)

[State 2501]
Type = ChangeAnim
Trigger1 = Var(12) = 3
Persistent = 0
Value = 41
Elem = 2
[State 2501]
Type = VelSet
Trigger1 = Var(12) = 3
Persistent = 0
X = -2.5
Y = -4
[State 2501]
Type = VelSet
Trigger1 = Var(12) = 3 && AnimElem = 4
X = -1
[State 2501]
Type = VelAdd
Trigger1 = Var(12) = 3
Y = .2

[State 2501]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 4

[State 2501]
Type = HitDef
Trigger1 = Anim = 471 && Pos Y < EnemyNear,Const(Size.Mid.Pos.Y)/2 && ((TimeMod = 2,0 && Var(12) < 3) || !MoveContact)
Attr = S, SA
HitFlag = MAF
GuardFlag = M
Damage = 66 - Var(15)*38, 5
GetPower = 37 - Var(15)*37
GivePower = 0
PauseTime = 6, 7
SparkNo = S62
Guard.SparkNo = S61
SparkXY = -30+Floor(P2Dist X)/2, -42
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
Ground.CornerPush.VelOff = 0
YAccel = .4
P2Facing = 1
ForceStand = 1
[State 2501]
Type = HitDef
Trigger1 = Anim = 471 && Pos Y > EnemyNear,Const(Size.Mid.Pos.Y)/2 && ((TimeMod = 2,0 && Var(12) < 3) || !MoveContact)
Attr = S, SA
HitFlag = MAF
GuardFlag = M
Damage = 66 - Var(15)*38, 5
GetPower = 37 - Var(15)*37
GivePower = 0
PauseTime = 6, 7
SparkNo = S62
Guard.SparkNo = S61
SparkXY = -30+Floor(P2Dist X)/2, -42
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
Ground.CornerPush.VelOff = 0
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2501]
Type = ChangeState
Trigger1 = Time > 32 && Pos Y >= 0
Value = 52

[StateDef 2502];ヨガスピン 強
Type = A
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
Anim = 181
PowerAdd = 125 - (Var(14))*1125

[State 2502]
Type = ChangeAnim
Trigger1 = Time = 2 || Time = 4 || Time = 6 || Time = [8,32]
Value = IFelse(Time=32, 471, 181)
Elem = IFelse(Time=2, 2, IFelse(Time=4, 3, IFelse(Time=6, 4, IFelse(Time=32, 1, 5))))

[State 2502]
Type = Explod
Trigger1 = Time = 6
Anim = 2500
Pos = 10, -70
PosType = P1
SprPriority = 10
OwnPal = 1
Scale = Const(Size.XScale), 1

[State 2502]
Type = AssertSpecial
Trigger1 = Time = [17,23]
Flag = Invisible

[State 2502]
Type = NotHitBy
Trigger1 = Time = [17,23]
Value = SCA

[State 2502]
Type = PosSet
Trigger1 = Time = 21
Y = -97
[State 2502]
Type = PosAdd
Trigger1 = Time = 21
X = 129*Const(Size.XScale)

[State 2502]
Type = VelSet
Trigger1 = Anim = 471 && AnimElem = 2
X = 3
Y = 4

[State 2502]
Type = VarSet
Trigger1 = Time = 0
V = 15
Value = (Var(14)&&Power=0)

[State 2502]
Type = VarSet
Trigger1 = Time = 0
V = 12
Value = 0
[State 2502]
Type = VarAdd
Trigger1 = (MoveHit || MoveGuarded) && !Var(15)
V = 12
Value = 1
[State 2502]
Type = MoveHitReset
Trigger1 = (MoveHit || MoveGuarded) && !Var(15)

[State 2502]
Type = ChangeAnim
Trigger1 = Var(12) = 3
Persistent = 0
Value = 41
Elem = 2
[State 2502]
Type = VelSet
Trigger1 = Var(12) = 3
Persistent = 0
X = -2.5
Y = -4
[State 2502]
Type = VelSet
Trigger1 = Var(12) = 3 && AnimElem = 4
X = -1
[State 2502]
Type = VelAdd
Trigger1 = Var(12) = 3
Y = .2

[State 2502]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 4

[State 2502]
Type = HitDef
Trigger1 = Anim = 471 && Pos Y < EnemyNear,Const(Size.Mid.Pos.Y)/2 && ((TimeMod = 2,0 && Var(12) < 3) || !MoveContact)
Attr = S, SA
HitFlag = MAF
GuardFlag = M
Damage = 66 - Var(15)*38, 5
GetPower = 37 - Var(15)*37
GivePower = 0
PauseTime = 6, 7
SparkNo = S62
Guard.SparkNo = S61
SparkXY = -30+Floor(P2Dist X)/2, -42
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
Ground.CornerPush.VelOff = 0
YAccel = .4
P2Facing = 1
ForceStand = 1
[State 2502]
Type = HitDef
Trigger1 = Anim = 471 && Pos Y > EnemyNear,Const(Size.Mid.Pos.Y)/2 && ((TimeMod = 2,0 && Var(12) < 3) || !MoveContact)
Attr = S, SA
HitFlag = MAF
GuardFlag = M
Damage = 66 - Var(15)*38, 5
GetPower = 37 - Var(15)*37
GivePower = 0
PauseTime = 6, 7
SparkNo = S62
Guard.SparkNo = S61
SparkXY = -30+Floor(P2Dist X)/2, -42
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
Ground.CornerPush.VelOff = 0
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2502]
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
Trigger1 = AnimElem = 2,1
Value = 10, 4

[State 2600]
Type = Projectile
trigger1 = AnimElem = 4,2
ProjAnim = 2601
ProjHitAnim = 2602
ProjID = 3
OffSet = 31*Const(Size.XScale), -105
Velocity = 2.5*Const(Size.XScale), -5
Accel = 0, .25
ProjHeightBound = -240, -10
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
Guard.SparkNo = S61
SparkXY = 0, 0
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -16*Const(Size.XScale)
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2600]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 2601];ヨガカレー 中
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 2600
PowerAdd = 100

[State 2601]
Type = PlaySnd
Trigger1 = AnimElem = 2,1
Value = 10, 4

[State 2601]
Type = Projectile
trigger1 = AnimElem = 4,2
ProjAnim = 2601
ProjHitAnim = 2602
ProjID = 3
OffSet = 31*Const(Size.XScale), -105
Velocity = 3*Const(Size.XScale), -5.5
Accel = 0, .25
ProjHeightBound = -240, -10
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
Guard.SparkNo = S61
SparkXY = 0, 0
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -16*Const(Size.XScale)
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2601]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 2602];ヨガカレー 強
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 2600
PowerAdd = 100

[State 2602]
Type = PlaySnd
Trigger1 = AnimElem = 2,1
Value = 10, 4

[State 2602]
Type = Projectile
trigger1 = AnimElem = 4,2
ProjAnim = 2601
ProjHitAnim = 2602
ProjID = 3
OffSet = 31*Const(Size.XScale), -105
Velocity = 3.5*Const(Size.XScale), -6
Accel = 0, .25
ProjHeightBound = -240, -10
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
Guard.SparkNo = S61
SparkXY = 0, 0
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -16*Const(Size.XScale)
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2602]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 2603];ヨガカレー スペシャル
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 2600
PowerAdd = -1000

[State 2603]
Type = VarSet
Trigger1 = Time = 0
V = 15
Value = (Command="holdx")*2 + (Command="holdy") + (Command="holdz")*3 + (Command="holdy"&&Command="holdx")*4

[State 2603]
Type = PlaySnd
Trigger1 = (AnimElem = 2,1 && Var(15) < 4) || (AnimElem = 2,4 && Var(15) > 3)
Value = 10, 4

[State 2603]
Type = ChangeAnim
Trigger1 = ((((AnimElem = 4,3 && Var(15) = [2,3]) || (AnimElem = 4,12 && Var(15) = 1)) && ((Time<60&&Command!="holdc")||Command="holda")) || (Var(15) > 3 && Time = [4,32])) && Var(30) = 1
Trigger2 = (((AnimElem = 4,3 && Var(15) = [2,3]) || (AnimElem = 4,12 && Var(15) = 1)) && Time<60) || (Var(15) > 3 && Time = [4,32])
Value = 2600
Elem = 2

[State 2603]
Type = Projectile
trigger1 = AnimElem = 4,2 && Var(15) = 2
ProjAnim = 2601
ProjHitAnim = 2602
ProjID = 3
OffSet = 33*Const(Size.XScale), -105
Velocity = 0.0 +((Time-8)/14)*Const(Size.XScale), -7
Accel = 0, .275
ProjHeightBound = -480, -10
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
ProjShadow = -1
Attr = S, SP
GuardFlag = MA
HitFlag = MA
Damage = 98, 10
GetPower = 0
GivePower = 0
PauseTime = 0, 6
SparkNo = S62
Guard.SparkNo = S61
SparkXY = 0, 0
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = (-3+(Random<500)*6)*Const(Size.XScale), -3
Air.Fall = 1
Fall.YVelocity = -3
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2603]
Type = VarSet
Trigger1 = AnimElem = 4,0 && Var(15) < 2
V = 16
Value = 0
[State 2603]
Type = VarAdd
Trigger1 = AnimElem = 4,>0 && Var(15) < 2
Persistent = 2
V = 16
Value = 1

[State 2603]
Type = Projectile
trigger1 = Var(15) < 2 && AnimElem = 4,[0,6]
Persistent = 2
ProjAnim = 2601
ProjHitAnim = 2602
ProjID = 3
OffSet = 33*Const(Size.XScale), -105
Velocity = 0.0 +(-1.5+Var(16))*Const(Size.XScale), -6
Accel = 0, .275
ProjHeightBound = -480, -10
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
ProjShadow = -1
Attr = S, SP
GuardFlag = MA
HitFlag = MA
Damage = 70, 10
GetPower = 0
GivePower = 0
PauseTime = 0, 6
SparkNo = S62
Guard.SparkNo = S61
SparkXY = 0, 0
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = (-3+(Random<500)*6)*Const(Size.XScale), -3
Air.Fall = 1
Fall.YVelocity = -3
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2605]
Type = VarRandom
Trigger1 = Var(15) = 3
V = 16
Range = 0,1

[State 2603]
Type = Projectile
trigger1 = AnimElem = 4,2 && !Var(16) && Var(15) = 3
ProjAnim = 2601
ProjHitAnim = 2602
ProjID = 3
OffSet = 33*Const(Size.XScale), -105
;Velocity = ((Time-8)/20)*Const(Size.XScale), -7
Velocity = 0.0+((P2Dist X/(60+EnemyNear,Const(Size.Mid.Pos.Y)/10))-(Random/500)-IFelse(TimeMod=4,0,0,.5)), -7.0+P2Dist Y/50
Accel = 0, .275
ProjHeightBound = -480, -10
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
ProjShadow = -1
Attr = S, SP
GuardFlag = MA
HitFlag = MA
Damage = 84, 10
GetPower = 0
GivePower = 0
PauseTime = 0, 6
SparkNo = S62
Guard.SparkNo = S61
SparkXY = 0, 0
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = (-8+(Random<500)*16)*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = (-3+(Random<500)*6)*Const(Size.XScale), -3
Air.Fall = 1
Fall.YVelocity = -3
YAccel = .4
P2Facing = 1
ForceStand = 1
[State 2603]
Type = Projectile
trigger1 = AnimElem = 4,2 && Var(16) && Var(15) = 3
ProjAnim = 2601
ProjHitAnim = 2602
ProjID = 3
OffSet = 33*Const(Size.XScale), -105
;Velocity = ((Time-8)/20)*Const(Size.XScale), -7
Velocity = (P2Dist X/(60+EnemyNear,Const(Size.Mid.Pos.Y)/10))-(Random/500)-IFelse(TimeMod=4,0,0,.5), -7+P2Dist Y/50
Accel = 0, .275
ProjHeightBound = -480, -10
ProjPriority = 1
ProjScale = Const(Size.XScale), 1
ProjShadow = -1
Attr = S, SP
GuardFlag = MA
HitFlag = MA
Damage = 84, 10
GetPower = 0
GivePower = 0
PauseTime = 0, 6
SparkNo = S62
Guard.SparkNo = S61
SparkXY = 0, 0
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = (-8+(Random<500)*16)*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = (-3+(Random<500)*6)*Const(Size.XScale), -3
Air.Fall = 1
Fall.YVelocity = -3
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2603]
Type = Projectile
trigger1 = Var(15) > 3 && AnimElem = 4,2
Persistent = 2
ProjAnim = 2603
ProjHitAnim = 2602
ProjID = 3
OffSet = 33*Const(Size.XScale), -105
Velocity = 2, -7
Accel = 0, .3
ProjHeightBound = -480, -10
ProjPriority = 2
ProjScale = 2*Const(Size.XScale), 2
ProjShadow = -1
Attr = S, SP
GuardFlag = MA
HitFlag = MA
Damage = 240, 24
GetPower = 0
GivePower = 0
PauseTime = 0, 6
SparkNo = S62
Guard.SparkNo = S61
SparkXY = 0, 0
HitSound = S2, 3
GuardSound = S2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 24
Air.HitTime = 18
Ground.Velocity = -8*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = (-3+(Random<500)*6)*Const(Size.XScale), -3
Air.Fall = 1
Fall.YVelocity = -3
YAccel = .4
P2Facing = 1
ForceStand = 1

[State 2603]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1