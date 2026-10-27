[StateDef 1000];百烈脚 弱
Type = U
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
SprPriority = 2
PowerAdd = 0 +(PrevStateNo<1000&&!Var(10))*60

[State 1000]
Type = VarSet
Trigger1 = Time = 0 || Command = "waza1_light"
V = 14
Value = IFelse(Time=0, 0, 1)

[State 1000]
Type = ChangeAnim
Trigger1 = Anim = 1000 && AnimElem = 3
Value = 1001
[State 1000]
Type = ChangeAnim
Trigger1 = Time = 0 || (Anim=1001&&Time=48&&!Var(14))
Value = 1000
Elem = IFelse(PrevStateNo=1001||Anim=1001, 4, 1)

[State 1000]
Type = VarSet
Trigger1 = Time = 0 || MoveHit
V = 15
Value = IFelse(Time=0||(Var(15)&&MoveContact), 0, 1)
[State 1000]
Type = MoveHitReset
Trigger1 = MoveHit

[State 1000]
Type = HitDef
Trigger1 = Time = 5
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 189, 42
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, -76
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -1.5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1000]
Type = HitDef
Trigger1 = Time > 5 && (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 189, 42
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -80, IFelse(AnimElemNo(0)=7, -32, -48))
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
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1000]
Type = HitDef
Trigger1 = Time > 5 && (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 189, 42
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -80, IFelse(AnimElemNo(0)=7, -32, -48))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1000]
Type = HitDef
Trigger1 = Time > 5 && AnimElem = 7 && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = L
Damage = 189, 42
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -80, IFelse(AnimElemNo(0)=7, -32, -48))
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
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1000]
Type = HitDef
Trigger1 = Time > 5 && AnimElem = 7 && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = L
Damage = 189, 42
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -80, IFelse(AnimElemNo(0)=7, -32, -48))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 1000]
Type = ChangeState
Trigger1 = (Anim=1000&&AnimTime=0) || (Time=48&&Var(14))
Value = IFelse((Anim=1000&&AnimTime=0), 0, 1001)
Ctrl = IFelse((Anim=1000&&AnimTime=0), 1, 0)

[State 1000]
Type = PlaySnd
Trigger1 = AnimElem = 2 &&Time > 32
Value = 10, 1
Channel = 0

[StateDef 1001];百烈脚中 弱
Type = U
MoveType = A
Physics = N
SprPriority = 2

[State 1001]
Type = ChangeAnim
Trigger1 = Time = 0 && PrevStateNo != 1000
Value = 1001
Elem = AnimElemNo(0)

[State 1001]
Type = PlaySnd
Trigger1 = AnimElem = 2
Value = 10, 1
Channel = 0

[State 1001]
Type = VarSet
Trigger1 = Time = 0 || MoveHit
V = 15
Value = IFelse(Time=0||(Var(15)&&MoveContact), 0, 1)
[State 1001]
Type = MoveHitReset
Trigger1 = MoveHit

[State 1001]
Type = HitDef
Trigger1 = (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 189, 42
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -80, IFelse(AnimElemNo(0)=7, -32, -48))
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
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1001]
Type = HitDef
Trigger1 = (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 189, 42
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -80, IFelse(AnimElemNo(0)=7, -32, -48))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1001]
Type = HitDef
Trigger1 = AnimElem = 7 && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = L
Damage = 189, 42
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -80, IFelse(AnimElemNo(0)=7, -32, -48))
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
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1001]
Type = HitDef
Trigger1 = AnimElem = 7 && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = L
Damage = 189, 42
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -80, IFelse(AnimElemNo(0)=7, -32, -48))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 1001]
Type = ChangeState
Trigger1 = TimeMod = 16,8 && Var(14) != 1
Value = IFelse(Var(14)=2, 1003, IFelse(Var(14)=3, 1005, 1000))

[State 1001]
Type = VarSet
Trigger1 = Command = "a" || Command = "b" || Command = "c" || TimeMod = 16,8
V = 14
Value = IFelse(Command="a", 1, IFelse(Command="b", 2, IFelse(Command="c", 3, 0)))

[StateDef 1002];百烈脚 中
Type = U
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
SprPriority = 2
PowerAdd = 0 +(PrevStateNo<1000&&!Var(10))*60

[State 1002]
Type = VarSet
Trigger1 = Time = 0 || Command = "waza1_middle"
V = 14
Value = IFelse(Time=0, 0, 2)

[State 1002]
Type = ChangeAnim
Trigger1 = Anim = 1000 && AnimElem = 3
Value = 1002
[State 1002]
Type = ChangeAnim
Trigger1 = Time = 0 || (Anim=1002&&Time=26&&Var(14)=0)
Value = 1000
Elem = IFelse(PrevStateNo=1003||Anim=1002, 4, 1)

[State 1002]
Type = VarSet
Trigger1 = Time = 0 || MoveHit
V = 15
Value = IFelse(Time=0||(Var(15)&&MoveContact), 0, 1)
[State 1002]
Type = MoveHitReset
Trigger1 = MoveHit

[State 1002]
Type = HitDef
Trigger1 = Time = 5
Attr = S, SA
AnimType = Light
HitFlag = MA
GuardFlag = M
Damage = 196, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, -76
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1002]
Type = HitDef
Trigger1 = Time > 5 && (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
AnimType = Light
HitFlag = MA
GuardFlag = M
Damage = 196, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
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
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1002]
Type = HitDef
Trigger1 = Time > 5 && (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && Var(15)
Attr = S, SA
AnimType = Light
HitFlag = MA
GuardFlag = M
Damage = 196, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1002]
Type = HitDef
Trigger1 = Time > 5 && AnimElem = 7 && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
AnimType = Light
HitFlag = MA
GuardFlag = L
Damage = 196, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
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
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1002]
Type = HitDef
Trigger1 = Time > 5 && AnimElem = 7 && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && Var(15)
Attr = S, SA
AnimType = Light
HitFlag = MA
GuardFlag = L
Damage = 196, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 1002]
Type = ChangeState
Trigger1 = (Anim=1000&&AnimTime=0) || (Time=26&&Var(14)=2)
Value = IFelse((Anim=1000&&AnimTime=0), 0, 1003)
Ctrl = IFelse((Anim=1000&&AnimTime=0), 1, 0)

[State 1002]
Type = PlaySnd
Trigger1 = AnimElem = 2 &&Time > 14
Value = 10, 1
Channel = 0

[StateDef 1003];百烈脚中 中
Type = U
MoveType = A
Physics = N
SprPriority = 2

[State 1003]
Type = ChangeAnim
Trigger1 = Time = 0 && PrevStateNo != 1002
Value = 1002
Elem = AnimElemNo(0)

[State 1003]
Type = PlaySnd
Trigger1 = AnimElem = 2
Value = 10, 1
Channel = 0

[State 1003]
Type = VarSet
Trigger1 = Time = 0 || MoveHit
V = 15
Value = IFelse(Time=0||(Var(15)&&MoveContact), 0, 1)
[State 1003]
Type = MoveHitReset
Trigger1 = MoveHit

[State 1003]
Type = HitDef
Trigger1 = (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
AnimType = Light
HitFlag = MA
GuardFlag = M
Damage = 196, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
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
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1003]
Type = HitDef
Trigger1 = (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && Var(15)
Attr = S, SA
AnimType = Light
HitFlag = MA
GuardFlag = M
Damage = 196, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1003]
Type = HitDef
Trigger1 = AnimElem = 7 && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
AnimType = Light
HitFlag = MA
GuardFlag = L
Damage = 196, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
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
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1003]
Type = HitDef
Trigger1 = AnimElem = 7 && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && Var(15)
Attr = S, SA
AnimType = Light
HitFlag = MA
GuardFlag = L
Damage = 196, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 1003]
Type = ChangeState
Trigger1 = TimeMod = 12,6 && Var(14) != 2
Value = IFelse(Var(14)=1, 1001, IFelse(Var(14)=3, 1005, 1002))

[State 1003]
Type = VarSet
Trigger1 = Command = "a" || Command = "b" || Command = "c" || TimeMod = 12,6
V = 14
Value = IFelse(Command="a", 1, IFelse(Command="b", 2, IFelse(Command="c", 3, 0)))

[StateDef 1004];百烈脚 強
Type = U
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
SprPriority = 2
PowerAdd = 0 +(PrevStateNo<1000&&!Var(10))*60

[State 1004]
Type = VarSet
Trigger1 = Time = 0 || Command = "waza1_hard"
V = 14
Value = IFelse(Time=0, 0, 3)

[State 1004]
Type = ChangeAnim
Trigger1 = Anim = 1000 && AnimElem = 3
Value = 1003
[State 1004]
Type = ChangeAnim
Trigger1 = Time = 0 || (Anim=1003&&Time=15&&Var(14)=0)
Value = 1000
Elem = IFelse(PrevStateNo=1005||Anim=1003, 4, 1)

[State 1004]
Type = VarSet
Trigger1 = Time = 0 || MoveHit
V = 15
Value = IFelse(Time=0||(Var(15)&&MoveContact), 0, 1)
[State 1004]
Type = MoveHitReset
Trigger1 = MoveHit

[State 1004]
Type = HitDef
Trigger1 = Time = 4
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 231, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, -76
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -12*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1004]
Type = HitDef
Trigger1 = Time > 5 && (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 231, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
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
Guard.Velocity = -12*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1004]
Type = HitDef
Trigger1 = Time > 5 && (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 231, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -12*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 1004]
Type = ChangeState
Trigger1 = (Anim=1000&&AnimTime=0) || (Time=15&&Var(14)=3)
Value = IFelse((Anim=1000&&AnimTime=0), 0, 1005)
Ctrl = IFelse((Anim=1000&&AnimTime=0), 1, 0)

[State 1004]
Type = PlaySnd
Trigger1 = AnimElem = 2 &&Time > 7
Value = 10, 1
Channel = 0

[StateDef 1005];百烈脚中 強
Type = U
MoveType = A
Physics = N
SprPriority = 2

[State 1005]
Type = ChangeAnim
Trigger1 = Time = 0 && PrevStateNo != 1004
Value = 1003
Elem = AnimElemNo(0)

[State 1005]
Type = PlaySnd
Trigger1 = AnimElem = 2
Value = 10, 1
Channel = 0

[State 1005]
Type = VarSet
Trigger1 = Time = 0 || MoveHit
V = 15
Value = IFelse(Time=0||(Var(15)&&MoveContact), 0, 1)
[State 1005]
Type = MoveHitReset
Trigger1 = MoveHit

[State 1005]
Type = HitDef
Trigger1 = (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && !Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 231, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
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
Guard.Velocity = -12*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 1005]
Type = HitDef
Trigger1 = (AnimTime=0||AnimElem=3||AnimElem=5||(AnimElem=7&&EnemyNear,StateNo!=[120,199])) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199]) && Var(15)
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 231, 49
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -28+Floor(P2Dist X)/2, IFelse(AnimElemNo(0)=1, -76, IFelse(AnimElemNo(0)=7, -32, -48))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -12*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 1005]
Type = ChangeState
Trigger1 = TimeMod = 8,4 && Var(14) != 3
Value = IFelse(Var(14)=1, 1001, IFelse(Var(14)=2, 1003, 1004))

[State 1005]
Type = VarSet
Trigger1 = Command = "a" || Command = "b" || Command = "c" || TimeMod = 8,4
V = 14
Value = IFelse(Command="a", 1, IFelse(Command="b", 2, IFelse(Command="c", 3, 0)))

[StateDef 1100];スピニングバードキック 弱
Type = U
MoveType = A
Physics = N
Anim = 1100
Ctrl = 0
VelSet = 0, 0
SprPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1100]
Type = NotHitBy
Trigger1 = AnimElem = 8,<2
Value = SCA

[State 1100]
Type = StateTypeSet
Trigger1 = AnimElem = 8
Value = A
[State 1100]
Type = VelSet
Trigger1 = AnimElem = 8 || AnimElem = 34
X = 3.5*Const(Size.XScale)
Y = IFelse((AnimElem=8,0), -3, 4)
[State 1100]
Type = VelAdd
Trigger1 = AnimElem = 8,>0 && AnimElem = 15,<0
Y = .27
[State 1100]
Type = VelSet
Trigger1 = AnimElem = 10,>0 && AnimElem = 34,<0 && Vel Y > 0
Y = 0

[State 1100]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 4
Channel = 0
[State 1100]
Type = PlaySnd
Trigger1 = AnimElem=14,>0&&AnimElem=33,<=0 && TimeMod = 8,0
Value = 0, 7

[State 1100]
Type = HitDef
Trigger1 = (AnimElem=11||AnimElem=15||AnimElem=19||AnimElem=23||AnimElem=27||AnimElem=31||AnimElem=35) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199])
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 84, 14
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -36+Floor(P2Dist X)/2, -68
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = (-9+(P2Dist X<0)*18)*Const(Size.XScale)
Air.Velocity = (-1.5+(P2Dist X<0)*3)*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = IFelse(P2Dist X>0, 1, -1)

[State 1100]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 1103

[StateDef 1101];スピニングバードキック 中
Type = U
MoveType = A
Physics = N
Anim = 1101
Ctrl = 0
VelSet = 0, 0
SprPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1101]
Type = NotHitBy
Trigger1 = AnimElem = 8,<2
Value = SCA

[State 1101]
Type = StateTypeSet
Trigger1 = AnimElem = 8
Value = A
[State 1101]
Type = VelSet
Trigger1 = AnimElem = 8 || AnimElem = 42
X = 3.5*Const(Size.XScale)
Y = IFelse((AnimElem=8,0), -3, 4)
[State 1101]
Type = VelAdd
Trigger1 = AnimElem = 8,>0 && AnimElem = 15,<0
Y = .27
[State 1101]
Type = VelSet
Trigger1 = AnimElem = 10,>0 && AnimElem = 42,<0 && Vel Y > 0
Y = 0

[State 1101]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 4
Channel = 0
[State 1101]
Type = PlaySnd
Trigger1 = AnimElem=14,>0&&AnimElem=41,<=0 && TimeMod = 8,0
Value = 0, 7

[State 1101]
Type = HitDef
Trigger1 = (AnimElem=11||AnimElem=15||AnimElem=19||AnimElem=23||AnimElem=27||AnimElem=31||AnimElem=35||AnimElem=39||AnimElem=43) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199])
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 84, 21
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -36+Floor(P2Dist X)/2, -68
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = (-9+(P2Dist X<0)*18)*Const(Size.XScale)
Air.Velocity = (-1.5+(P2Dist X<0)*3)*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = IFelse(P2Dist X>0, 1, -1)

[State 1101]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 1103

[StateDef 1102];スピニングバードキック 強
Type = U
MoveType = A
Physics = N
Anim = 1102
Ctrl = 0
VelSet = 0, 0
SprPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1102]
Type = NotHitBy
Trigger1 = AnimElem = 8,<2
Value = SCA

[State 1102]
Type = StateTypeSet
Trigger1 = AnimElem = 8
Value = A
[State 1102]
Type = VelSet
Trigger1 = AnimElem = 8 || AnimElem = 66
X = 3.5*Const(Size.XScale)
Y = IFelse((AnimElem=8,0), -3, 4)
[State 1102]
Type = VelAdd
Trigger1 = AnimElem = 8,>0 && AnimElem = 15,<0
Y = .27
[State 1102]
Type = VelSet
Trigger1 = AnimElem = 10,>0 && AnimElem = 66,<0 && Vel Y > 0
Y = 0

[State 1102]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 4
Channel = 0
[State 1102]
Type = PlaySnd
Trigger1 = AnimElem=14,>0&&AnimElem=65,<=0 && TimeMod = 8,0
Value = 0, 7

[State 1102]
Type = HitDef
TriggerAll = EnemyNear,MoveType != H || EnemyNear,StateNo = [120,199]
Trigger1 = AnimElem=11||AnimElem=15||AnimElem=19||AnimElem=23||AnimElem=27||AnimElem=31||AnimElem=35
Trigger2 = AnimElem=39||AnimElem=43||AnimElem=47||AnimElem=51||AnimElem=55||AnimElem=59||AnimElem=63||AnimElem=67
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 112, 28
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -36+Floor(P2Dist X)/2, -68
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = (-9+(P2Dist X<0)*18)*Const(Size.XScale)
Air.Velocity = (-1.5+(P2Dist X<0)*3)*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = IFelse(P2Dist X>0, 1, -1)

[State 1102]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 1103

[StateDef 1103];スピニングバードキック 着地
Type = S
MoveType = A
Physics = N
Anim = 1103
Ctrl = 0
VelSet = 0, 0

[State 1103]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 1103]
Type = PosSet
Trigger1 = Time = 0
Y = 0

[State 1103]
Type = Turn
Trigger1 = Time = 0

[State 1103]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1104];空中スピニングバードキック 弱
Type = A
MoveType = A
Physics = N
Anim = 1104
Ctrl = 0
VelSet = Vel X, Vel Y-1
SprPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1104]
Type = VelAdd
Trigger1 = AnimElem = 6
X = (1 -(Vel X<0)*2)*Const(Size.XScale)
[State 1104]
Type = VelAdd
Trigger1 = 1
Y = .44

[State 1104]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 4
Channel = 0
[State 1104]
Type = PlaySnd
Trigger1 = AnimElem = 14
Value = 0, 7

[State 1104]
Type = HitDef
Trigger1 = (AnimElem=9||AnimElem=13) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199])
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 91, 14
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -36+Floor(P2Dist X)/2, -68
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = (-9+(P2Dist X<0)*18)*Const(Size.XScale)
Air.Velocity = (-1.5+(P2Dist X<0)*3)*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = IFelse(P2Dist X>0, 1, -1)

[State 1104]
Type = ChangeState
Trigger1 = Pos Y >= 0
Value = 1103 +(AnimElem=15,>=0)*4

[StateDef 1105];空中スピニングバードキック 中
Type = A
MoveType = A
Physics = N
Anim = 1104
Ctrl = 0
VelSet = Vel X, Vel Y-1
SprPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1105]
Type = VelAdd
Trigger1 = AnimElem = 6
X = (1 -(Vel X<0)*2)*Const(Size.XScale)
[State 1105]
Type = VelAdd
Trigger1 = 1
Y = .44

[State 1105]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 4
Channel = 0
[State 1105]
Type = PlaySnd
Trigger1 = AnimElem = 14
Value = 0, 7

[State 1105]
Type = HitDef
Trigger1 = (AnimElem=9||AnimElem=13) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199])
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 77, 21
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -36+Floor(P2Dist X)/2, -68
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = (-9+(P2Dist X<0)*18)*Const(Size.XScale)
Air.Velocity = (-1.5+(P2Dist X<0)*3)*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = IFelse(P2Dist X>0, 1, -1)

[State 1105]
Type = ChangeState
Trigger1 = Pos Y >= 0
Value = 1103 +(AnimElem=15,>=0)*4

[StateDef 1106];空中スピニングバードキック 強
Type = A
MoveType = A
Physics = N
Anim = 1106
Ctrl = 0
VelSet = Vel X, Vel Y-1
SprPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1106]
Type = VelAdd
Trigger1 = AnimElem = 6
X = (1 -(Vel X<0)*2)*Const(Size.XScale)
[State 1106]
Type = VelAdd
Trigger1 = 1
Y = .44

[State 1106]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 4
Channel = 0
[State 1106]
Type = PlaySnd
Trigger1 = AnimElem = 14
Value = 0, 7

[State 1106]
Type = HitDef
Trigger1 = (AnimElem=9||AnimElem=13) && (EnemyNear,MoveType!=H||EnemyNear,StateNo=[120,199])
Attr = S, SA
HitFlag = MA
GuardFlag = M
Damage = 77, 21
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+TeamSide
SparkXY = -36+Floor(P2Dist X)/2, -68
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 99
Ground.Velocity = (-9+(P2Dist X<0)*18)*Const(Size.XScale)
Air.Velocity = (-1.5+(P2Dist X<0)*3)*Const(Size.XScale), -5
Ground.CornerPush.VelOff = -18*Const(Size.XScale)
Air.Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = IFelse(P2Dist X>0, 1, -1)

[State 1106]
Type = ChangeState
Trigger1 = Pos Y >= 0
Value = 1103 +(AnimElem=15,>=0)*4

[StateDef 1107];空中スピニングバードキック 着地
Type = S
MoveType = A
Physics = N
Anim = 1107
Ctrl = 0
VelSet = 0, 0

[State 1107]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 1107]
Type = Turn
Trigger1 = Time = 0

[State 1107]
Type = PosSet
Trigger1 = Time = 0
Y = 0

[State 1107]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 1200];気功拳 弱
Type = U
MoveType = A
Physics = U
Ctrl = 0
VelSet = 0, 0
Anim = 1200
SorPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1200]
Type = VelSet
Trigger1 = StateType = A
Y = 0

[State 1200]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 5
Channel = 0

[State 1200]
Type = Helper
Trigger1 = AnimElem = 7
Name = "Guard_HANTEI"
ID = 1000
Pos = 40, 0
PosType = P1
StateNo = 1208

[State 1200]
Type = Projectile
trigger1 = AnimElem = 7 && EnemyNear,StateNo != [5020,5099]
ProjAnim = 1201
ProjHitAnim = 1202
ProjID = 1
OffSet = 0, 0
Velocity = 3*Const(Size.XScale)
ProjPriority = 1
ProjRemoveTime = 73
ProjScale = Const(Size.XScale), 1
Attr = S, SP
GuardFlag = M
HitFlag = MA
Damage = 105, 21
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Air.Fall = 1
Fall.Recover = 0
YAccel = .25
Fall.YVelocity = -4
P2Facing = 1
ForceStand = 1
[State 1200]
Type = Projectile
Trigger1 = AnimElem = 7 && EnemyNear,StateNo = [5020,5099]
ProjAnim = 1201
ProjHitAnim = 1202
ProjID = 1
OffSet = 0, 0
Velocity = 3*Const(Size.XScale)
ProjPriority = 1
ProjRemoveTime = 73
ProjScale = Const(Size.XScale), 1
Attr = S, SP
GuardFlag = M
HitFlag = M
Damage = 105, 21
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Air.Fall = 1
Fall.Recover = 0
YAccel = .25
Fall.YVelocity = -4
P2Facing = 1
ForceStand = 1

[State 1200]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0 + (StateType=A)*50
Ctrl = 1 - (StateType=A)

[StateDef 1201];気功拳 中
Type = U
MoveType = A
Physics = U
Ctrl = 0
VelSet = 0, 0
Anim = 1200
SorPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1201]
Type = VelSet
Trigger1 = StateType = A
Y = 0

[State 1201]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 5
Channel = 0

[State 1201]
Type = Helper
Trigger1 = AnimElem = 7
Name = "Guard_HANTEI"
ID = 1000
Pos = 40, 0
PosType = P1
StateNo = 1208

[State 1201]
Type = Projectile
trigger1 = AnimElem = 7 && EnemyNear,StateNo != [5020,5099]
ProjAnim = 1201
ProjHitAnim = 1202
ProjID = 1
OffSet = 0, 0
Velocity = 4*Const(Size.XScale)
ProjPriority = 1
ProjRemoveTime = 44
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
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Air.Fall = 1
Fall.Recover = 0
YAccel = .25
Fall.YVelocity = -4
P2Facing = 1
ForceStand = 1
[State 1201]
Type = Projectile
trigger1 = AnimElem = 7 && EnemyNear,StateNo = [5020,5099]
ProjAnim = 1201
ProjHitAnim = 1202
ProjID = 1
OffSet = 0, 0
Velocity = 4*Const(Size.XScale)
ProjPriority = 1
ProjRemoveTime = 44
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
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Air.Fall = 1
Fall.Recover = 0
YAccel = .25
Fall.YVelocity = -4
P2Facing = 1
ForceStand = 1

[State 1201]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0 + (StateType=A)*50
Ctrl = 1 - (StateType=A)

[StateDef 1202];気功拳 強
Type = U
MoveType = A
Physics = U
Ctrl = 0
VelSet = 0, 0
Anim = 1200
SorPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1202]
Type = VelSet
Trigger1 = StateType = A
Y = 0

[State 1202]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 5
Channel = 0

[State 1202]
Type = Helper
Trigger1 = AnimElem = 7
Name = "Guard_HANTEI"
ID = 1000
Pos = 40, 0
PosType = P1
StateNo = 1208

[State 1202]
Type = Projectile
trigger1 = AnimElem = 7 && EnemyNear,StateNo != [5020,5099]
ProjAnim = 1201
ProjHitAnim = 1202
ProjID = 1
OffSet = 0, 0
Velocity = 5*Const(Size.XScale)
ProjPriority = 1
ProjRemoveTime = 30
ProjScale = Const(Size.XScale), 1
Attr = S, SP
GuardFlag = M
HitFlag = MA
Damage = 119, 28
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Air.Fall = 1
Fall.Recover = 0
YAccel = .25
Fall.YVelocity = -4
P2Facing = 1
ForceStand = 1
[State 1202]
Type = Projectile
trigger1 = AnimElem = 7 && EnemyNear,StateNo = [5020,5099]
ProjAnim = 1201
ProjHitAnim = 1202
ProjID = 1
OffSet = 0, 0
Velocity = 5*Const(Size.XScale)
ProjPriority = 1
ProjRemoveTime = 30
ProjScale = Const(Size.XScale), 1
Attr = S, SP
GuardFlag = M
HitFlag = M
Damage = 119, 28
GetPower = 0
GivePower = 0
PauseTime = 0, 15
SparkNo = -1
Guard.SparkNo = -1
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Air.Fall = 1
Fall.Recover = 0
YAccel = .25
Fall.YVelocity = -4
P2Facing = 1
ForceStand = 1

[State 1202]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0 + (StateType=A)*50
Ctrl = 1 - (StateType=A)

[StateDef 1208];飛び道具 CPUに防御させ易くするヘルパー
Type = S
MoveType = A
Physics = N
Ctrl = 0
Anim = 99

[State 1208]
Type = VelSet
trigger1 = Time = 0
X=IFelse(Parent,StateNo=1200,3,IFelse(Parent,StateNo=1201,4,5))*Const(Size.XScale)

[State 1208]
Type = DestroySelf
Trigger1 = FrontEdgeDist < -50 || Parent,ProjContact0 = 1, 1 || Parent,ProjCancelTime(0) = 1

[StateDef 1300];天昇脚 弱
Type = U
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
Anim = 1300
SorPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1300]
Type = VelSet
Trigger1 = Vel X < 0
X = 0

[State 1300]
Type = VelSet
Trigger1 = AnimElem = 4
X = 4*Const(Size.XScale)
Y = -5
[State 1300]
Type = StateTypeSet
Trigger1 = AnimElem = 4
Value = A
[State 1300]
Type = VelAdd
Trigger1 = AnimElem = 4,>0 && Vel X > 0
X = -.5*Const(Size.XScale)
[State 1300]
Type = VelAdd
Trigger1 = AnimElem = 4,>0
Y = .2

[State 1300]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 0
Channel = 1

[State 1300]
Type = HitDef
Trigger1 = (AnimElem=2||AnimElem=5||AnimElem=10) && HitCount < 2
Attr = S, SA
HitFlag = MAF
GuardFlag = M
Damage = 77 +(AnimElem=2)*7, 14 +(AnimElem=2)*7
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+(TeamSide)
SparkXY = -20+Floor(P2Dist X)/2, -85 +(AnimElem=2)*25
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Up
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 1300]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 1303

[StateDef 1301];天昇脚 中
Type = U
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
Anim = 1301
SorPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1301]
Type = VelSet
Trigger1 = Vel X < 0
X = 0

[State 1301]
Type = VelSet
Trigger1 = AnimElem = 4
X = 5*Const(Size.XScale)
Y = -5.4
[State 1301]
Type = StateTypeSet
Trigger1 = AnimElem = 4
Value = A
[State 1301]
Type = VelAdd
Trigger1 = AnimElem = 4,>0 && Vel X > 0
X = -.5*Const(Size.XScale)
[State 1301]
Type = VelAdd
Trigger1 = AnimElem = 4,>0
Y = .2

[State 1301]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 0
Channel = 1

[State 1301]
Type = HitDef
Trigger1 = (AnimElem=2||AnimElem=5||AnimElem=10||AnimElem=15) && HitCount < 3
Attr = S, SA
HitFlag = MAF
GuardFlag = M
Damage = 77 +(AnimElem=2)*21, 14 +(AnimElem=2)*7
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+(TeamSide)
SparkXY = -20+Floor(P2Dist X)/2, -85 +(AnimElem=2)*25
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Up
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 1301]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 1303

[StateDef 1302];天昇脚 強
Type = U
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0, 0
Anim = 1302
SorPriority = 2
PowerAdd = 0 +(!Var(10))*60

[State 1302]
Type = VelSet
Trigger1 = Vel X < 0
X = 0

[State 1302]
Type = VelSet
Trigger1 = AnimElem = 4
X = 6*Const(Size.XScale)
Y = -5.8
[State 1302]
Type = StateTypeSet
Trigger1 = AnimElem = 4
Value = A
[State 1302]
Type = VelAdd
Trigger1 = AnimElem = 4,>0 && Vel X > 0
X = -.5*Const(Size.XScale)
[State 1302]
Type = VelAdd
Trigger1 = AnimElem = 4,>0
Y = .2

[State 1302]
Type = PlaySnd
Trigger1 = Time = 0
Value = 10, 0
Channel = 1

[State 1302]
Type = HitDef
Trigger1 = (AnimElem=2||AnimElem=5||AnimElem=10||AnimElem=15||AnimElem=20) && HitCount < 3
Attr = S, SA
HitFlag = MAF
GuardFlag = M
Damage = 77 -(AnimElem=5||AnimElem=20)*14 +(AnimElem=15)*7, 14 +(AnimElem=15)*7
GetPower = 0
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S54+(TeamSide)
SparkXY = -20+Floor(P2Dist X)/2, -85 +(AnimElem=2)*25
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Up
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 1302]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 1303

[StateDef 1303];天昇脚 着地
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0, 0

[State 1303]
Type = ChangeAnim
Trigger1 = Time = 0
Value = 1100 - (PrevStateNo>1199)*1053
Elem = 6 - (PrevStateNo>1199)*5

[State 1303]
Type = PosSet
Trigger1 = Time = 0
Y = 0

[State 1303]
Type = ChangeState
Trigger1 = (Anim=1100&&AnimTime=0) || (Anim=47&&Time=6)
Value = 0
Ctrl = 1

[StateDef 2000];千烈脚
Type = U
MoveType = A
Physics = N
Anim = 2000
Ctrl = 0
VelSet = 0, 0
PowerAdd = -1000

[State 2000]
Type = PlaySnd
Trigger1 = Time = 0 || AnimElem = 6
Value = 0 +(AnimElem=6)*10, 19 -(AnimElem=6)*18

[State 2000]
Type = Pause
Trigger1 = Time = 0
Time = 17
MoveTime = 17

[State 2000]
Type = AfterImage
Trigger1 = Time = 0
Time = 72
Trans = None
FrameGap = 3
Length = 10
PalColor = 0
PalBright = 90, 90, 120
PalContrast = 170, 170, 240
PalAdd = -25, -25, -25
[State 2000]
Type = AfterImage
Trigger1 = Time = 72
Time = 0

[State 2000]
Type = Explod
Trigger1 = Time = 0
Anim = 90
Pos = 0, 0
PosType = P1
OwnPal = 1
SprPriority = 10
PauseMoveTime = -1
BindTime = -1
Scale = Const(Size.XScale), 1

[State 2000]
Type = NotHitBy
Trigger1 = AnimElem = 7,<0
Value = SCA

[State 2000]
Type = VelSet
Trigger1 = StateType = A
Y = 0
[State 2000]
Type = VelSet
Trigger1 = Vel X <= 0
X = 0
[State 2000]
Type = VelSet
Trigger1 = AnimElem = 2 || AnimElem = 8
X = (5 +(AnimElem=8)*4)*Const(Size.XScale)
[State 2000]
Type = VelAdd
Trigger1 = Vel X > 0
X = -.4*Const(Size.XScale)

[State 2000]
Type = HitDef
Trigger1 = AnimElem = 6 || AnimElem = 15 || AnimElem = 19
Attr = S, HA
HitFlag = MAF
GuardFlag = M
Damage = 98, 21
GetPower = 0
GivePower = 0
PauseTime = 5, 8
SparkNo = S54+TeamSide
Guard.SparkNo = S54+(TeamSide)
SparkXY = -20+Floor(P2Dist X)/2, IFelse((AnimElem=9,<0), -56, IFelse((AnimElem=17,<0), -82, -54))
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air,AnimType = Up
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.SlideTime = 22
Ground.HitTime = 22
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5, -5
Ground.CornerPush.VelOff = 0
Air.Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 2000]
Type = HitDef
Trigger1 = AnimElem = 12 || AnimElem = 17
Attr = S, HA
HitFlag = MAF
GuardFlag = M
Damage = 98, 21
GetPower = 0
GivePower = 0
PauseTime = 5, 8
SparkNo = S54+TeamSide
Guard.SparkNo = S54+(TeamSide)
SparkXY = -20+Floor(P2Dist X)/2, IFelse((AnimElem=14,<0), -56, -48)
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Up
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -5*Const(Size.XScale)
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = 0
Air.Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1
[State 2000]
Type = HitDef
Trigger1 = AnimElem = 21
Attr = S, HA
HitFlag = MAF
GuardFlag = M
Damage = 98, 21
GetPower = 0
GivePower = 0
PauseTime = 5, 8
SparkNo = S54+TeamSide
Guard.SparkNo = S54+(TeamSide)
SparkXY = -20+Floor(P2Dist X)/2, -22
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Up
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29
Air.HitTime = 20
Ground.Velocity = -1.5*Const(Size.XScale), -5
Guard.Velocity = -9*Const(Size.XScale)
Air.Velocity = -1.5*Const(Size.XScale), -5
Ground.CornerPush.VelOff = 0
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[State 2000]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1