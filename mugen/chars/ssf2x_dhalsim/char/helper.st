[StateDef 3000];ポーズヘルパー待機
Type = S
Ctrl = 0
Anim = 99

[State 3000];スーパーコンボフィニッシュ演出へ移行
Type = ChangeState
Trigger1 = Parent,StateNo = 2000 && RoundState = 3 && Parent,ProjContactTime(2) = 2
Value = 3003

[State 3000];ダークストーカー時のパワー減少処理
Type = VarAdd
Trigger1 = Parent,Var(10)
V = 5
Value = 1
[State 3000]
Type = VarSet
Trigger1 = Parent,MoveType = A && !Parent,Var(14)
V = 5
Value = 0
[State 3000]
Type = VarSet
Trigger1 = (Var(5)>6&&Parent,Var(14)) || Var(5) > 28
V = 5
Value = -1
[State 3000]
Type = ParentVarSet
Trigger1 = Var(5) = -1
V = 13
Value = 1
[State 3000]
Type = ParentVarSet
Trigger1 = Var(5) = 0
V = 13
Value = 0
[State 3000];Specialの表記
Type = ParentVarSet
Trigger1 = ((Power = 1000 && !Parent,Var(14) && Parent,Var(10)) || (Power = 0 && Parent,Var(14)) || (Parent,Var(14) && (!Parent,Var(10)||RoundState=4)))
V = 14
Value = IFelse(Power=1000&&Parent,Var(10), 1, 0)
[State 3000]
Type = PlaySnd
Trigger1 = (Parent,Var(14) && Power > 0) && !NumExplod(10)
Value = 2, 8
[State 3000]
Type = Explod
Trigger1 = (Parent,Var(14) && Power > 0) && !NumExplod(10)
Anim = 91-Facing
ID = 10
Pos = Floor(33 * Const(Size.XScale)), 230
PosType = Back
BindTime = -1
OnTop = 1
Facing = Facing
RemoveTime = -1
Scale = Const(Size.XScale), 1
[State 3000]
Type = RemoveExplod
Trigger1 = (!Parent,Var(14) || (Power=0&&Parent,StateNo=[1000,2999])) && NumExplod(10)
ID = 10

[State 3000];FIRST ATTACKの表記
Type = Explod
Trigger1 = (P2MoveType = H && P2StateNo != [150, 199]) && !Parent,Var(7) && !Parent,Var(10)
Anim = 81-Facing
Pos = 0, 80
PosType = Back
BindTime = -1
SprPriority = -100
Facing = Facing
RemoveTime = IFelse(Var(10), 63, 63)
Scale = Const(Size.XScale), 1
[State 3000]
Type = ParentVarSet
Trigger1 = !Parent,Var(7) && ((P2MoveType = H && P2StateNo != [150, 199]) || Parent,StateNo = [5000, 5999])
V = 7
Value = 1
[State 3000];RECOVERYの表記
Type = Explod
Trigger1 = Parent,PrevStateNo = 4001 && Parent,Time = 1 && Parent,MoveType != H
Anim = 85 - Facing
Pos = -20, 60
PosType = Back
BindTime = -1
SprPriority = -100
Facing = Facing
[State 3000];REVERSAL ATTACKの表記
Type = Explod
Trigger1 = Parent,Var(18) > 0 && Parent,Time=1 && !Parent,Var(10) && ((Parent,StateNo=[1000,1002])||(Parent,StateNo=[1100,1105])||(Parent,StateNo=[1200,1203])||Parent,StateNo=2000||Parent,StateNo=[1300,1305])
;Trigger1 = Parent,PrevStateNo=5120 && Parent,Time=1 && !Parent,Var(10) && ((Parent,StateNo=[1000,1002])||(Parent,StateNo=[1100,1105])||(Parent,StateNo=[1200,1203])||Parent,StateNo=2000||Parent,StateNo=[1300,1305])
Anim = 82 - Facing
Pos = 18, 60
PosType = Back
BindTime = -1
SprPriority = -100
Facing = Facing

[State 3000];飛び道具ヒット時のスロー
Type = VarSet
Trigger1 = Parent,ProjHit1 = 1,16 || Parent,ProjHit2 = 1,16 || Var(11) = 40
V = 10
Value = IFelse(Var(11)=40, 0, 1)
[State 3000]
Type = VarSet
Trigger1 = Var(11) = 40
V = 11
Value = 0
[State 3000]
Type = VarAdd
Trigger1 = Var(10)
V = 11
Value = 1
[State 3000]
Type = Pause
Trigger1 = RoundState = 2 && Var(11) = [1,41]
Persistent = 2
Time = 1
MoveTime = 1

[State 3000];気絶値初期化
Type = VarAdd
Trigger1 = Parent,Var(1) > 0
V = 20
Value = 1
[State 3000]
Type = ParentVarSet
Trigger1 = Parent,Var(1) > 0 && Var(20) > 180
V = 1
Value = 0
[State 3000]
Type = VarSet
Trigger1 = Var(20) > 0 && (Var(20)>180||Parent,MoveType=H||Parent,Var(1)<0)
V = 20
Value = 0

[State 3000]
Type = ChangeState
Trigger1 = Parent,Alive && RoundState = 2 && Command = "start" && Parent,Command = "start" && Parent,Var(30) = 2
Value = 3001

[StateDef 3001];ポーズ
Type = S
Ctrl = 0

[State 3001]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 17

[State 3001]
Type = AssertSpecial
Trigger1 = 1
Flag = NoFG

[State 3001]
Type = BGPalFX
Trigger1 = 1
Time = 10
Mul = 100,100,100
Add = 70, 70, 70

[State 3001]
Type = PlaySnd
Trigger1 = Command="down"&&!Var(0)&&!Parent,Var(10)
Trigger2 = Command="up"&&Var(0)&&!Parent,Var(10)
Trigger3 = (Command="fwd"&&((!Var(0)&&!Parent,Var(5))||(Var(0)&&!Parent,Var(10))))|| (Command="back"&&((!Var(0)&&Parent,Var(5))||(Var(0)&&Parent,Var(10))))
Value = 0, 18

[State 3001]
Type = VarSet
Trigger1 = ((Command = "up" || Command = "down") && !Parent,Var(10)) || (Parent,Var(10) && Time = 0)
V = 0
Value = IFelse(Command="down"||(Parent,Var(10)&&Time=0), 1, 0)
[State 3001]
Type = RemoveExplod
Trigger1 = Command = "up" || Command = "down" || (Var(0) && (Command="fwd"||Command="back"))
ID = 3001

[State 3001]
Type = ParentVarSet
Trigger1 = !Var(0) && (Command = "fwd" || Command = "back")
V = 5
Value = IFelse(Command = "fwd", 1, 0)
[State 3001]
Type = RemoveExplod
Trigger1 = Command = "fwd" || Command = "back"
ID = 3002

[State 3001]
Type = ParentVarSet
Trigger1 = Var(0) && (Command = "fwd" || Command = "back")
V = 10
Value = IFelse(Command = "fwd", 1, 0)
[State 3001]
Type = RemoveExplod
Trigger1 = Var(0) && (Command = "fwd" || Command = "back")
ID = 3003

[State 3001]
Type = Explod
Trigger1 = Time = 0 || (Var(0) && (Command = "fwd" || Command = "back"))
Anim = 3013+Parent,Var(10)
ID = 3003
Pos = 160, 120
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0
Anim = 3000
ID = 3000
Pos = 160, 120
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = (Time = 0 || (Var(0) && (Command = "fwd" || Command = "back"))) && !Parent,Var(10)
Anim = 3010
ID = 3003
Pos = 160, 120 + Parent,Var(10)*12
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0 || (Var(0) && (Command = "fwd" || Command = "back"))
Anim = 3016
ID = 3003
Pos = 160, 120
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0 || (Var(0) && (Command="fwd"||Command="back"))
Anim = 3003 + Parent,Var(10)
ID = 3003
Pos = 160, 120
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = (Time = 0 && Parent,Var(10)) || (Var(0) && Command = "fwd")
Anim = 3005
ID = 3003
Pos = 160, 120
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0
Anim = IFelse(Parent,Facing = 1, 3001, 3002)
ID = 3000
Pos = 160, 120
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = (Time = 0 && Parent,Var(10)) || (Var(0) && Command = "fwd")
Anim = IFelse(Parent,Facing = 1, 3006, 3007)
ID = 3003
Pos = 160, 120
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0 || Command = "down" || Command = "up" || (Var(0)&&(Command="fwd"||Command="back"))
Anim = 3015
ID = 3001
Pos = 160, IFelse(Var(0), 133, 120)
PosType = Left
OwnPal = 1
SprPriority = 101
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = (Time = 0 || Command = "fwd" || Command = "back") && !Parent,Var(10)
Anim = 3011+Parent,Var(5)
ID = 3002
Pos = 160, 120 + Parent,Var(10)*12
PosType = Left
OwnPal = 1
SprPriority = 101
PauseMoveTime = -1
BindTime = -1

[State 3001]
Type = Explod
Trigger1 = Time = 0 || (Var(0) && (Command = "fwd" || Command = "back"))
Anim = 3020+Parent,Var(10)
ID = 3003
Pos = 160, 120 + Parent,Var(10)*12
PosType = Left
OwnPal = 1
SprPriority = 101
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0 || (Var(0) && (Command = "fwd" || Command = "back"))
Anim = 3022 + Parent,Var(10)*3
ID = 3003
Pos = 160, 120 + Parent,Var(10)*12
PosType = Left
OwnPal = 1
SprPriority = 101
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0 || (Var(0) && (Command = "fwd" || Command = "back"))
Anim = 3023+IFelse(Parent,Facing=1, 0, 1) + Parent,Var(10)*3
ID = 3003
Pos = 160, 120 + Parent,Var(10)*12
PosType = Left
OwnPal = 1
SprPriority = 101
PauseMoveTime = -1
BindTime = -1

[State 3001]
Type = Pause
Trigger1 = 1
Time = 10
MoveTime = 10

[State 3001]
Type = ChangeState
Trigger1 = Time >= 6 && Command = "start"
Value = 3002

[StateDef 3002];ポーズ待機へ
Type = S
Ctrl = 0

[State 3002]
Type = Pause
Trigger1 = Time = 0
Time = 0

[State 3002]
Type = VarSet
Trigger1 = Time = 0
V = 0
Value = 0

[State 3002]
Type = BGPalFX
Trigger1 = Time = 0
Time = 1

[State 3002]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3000
[State 3002]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3001
[State 3002]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3002
[State 3002]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3003

[State 3002]
Type = ChangeState
Trigger1 = Time = 1
Value = 3000

[StateDef 3003];スーパーコンボ勝利演出
Type = S
Ctrl = 0
Anim = 99

[State 3003]
Type = Turn
Trigger1 = Facing = -1

[State 3003]
Type = AssertSpecial
Trigger1 = Time < 102
Flag = GlobalNoShadow

[State 3003]
Type = Pause
Trigger1 = TimeMod = 3,0 && Time < 102
Time = 2
MoveTime = 2

[State 3003]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 20

[State 3003]
Type = Explod
Trigger1 = Time = 0
Anim = 94
Pos = 0, 0
PosType = Left
BindTime = -1
SprPriority = -100
RemoveTime = 101
PauseMoveTime = 101
[State 3003]
Type = Explod
Trigger1 = Time = 0
Anim = 94
Pos = 319, 0
PosType = Left
BindTime = -1
SprPriority = -100
RemoveTime = 101
PauseMoveTime = 101
Facing = -1
[State 3003]
Type = Explod
Trigger1 = Time = 0
Anim = 94
Pos = 0, 239
PosType = Left
BindTime = -1
SprPriority = -100
RemoveTime = 101
PauseMoveTime = 101
VFacing = -1
[State 3003]
Type = Explod
Trigger1 = Time = 0
Anim = 94
Pos = 319, 239
PosType = Left
BindTime = -1
SprPriority = -100
RemoveTime = 101
PauseMoveTime = 101
Facing = -1
VFacing = -1