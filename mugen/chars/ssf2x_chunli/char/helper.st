[StateDef 3000];ポーズヘルパー待機
Type = S
Ctrl = 0
Anim = 99

[State 3000];スーパーコンボフィニッシュ演出へ移行
Type = ChangeState
Trigger1 = Parent,StateNo = 2000 && RoundState = 3 && Parent,MoveContact
Value = 3003
[State 3000];勝利演出用KOスロー止め
Type = AssertSpecial
Trigger1 = Parent,StateNo = 2000 && Parent,MoveContact
Flag = NoKOSlow

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
Trigger1 = Parent,ProjHit1 = 1,16 || Var(11) = 30
V = 10
Value = IFelse(Var(11)=30, 0, 1)
[State 3000]
Type = VarSet
Trigger1 = Var(11) = 30
V = 11
Value = 0
[State 3000]
Type = VarAdd
Trigger1 = Var(10)
V = 11
Value = 1
[State 3000]
Type = Pause
Trigger1 = RoundState = 2 && Var(11) = [1,21]
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
Trigger1 = Command="down"&&Var(0)<2
Trigger2 = Command="up"&&Var(0)>0
Trigger3 = (Command="fwd"&&((Var(0)=0&&!Parent,Var(5))||(Var(0)=1&&!Parent,Var(10))||(Var(0)=2&&Parent,Var(13))))|| (Command="back"&&((Var(0)=0&&Parent,Var(5))||(Var(0)=1&&Parent,Var(10))||(Var(0)=2&&!Parent,Var(13))))
Value = 0, 18

[State 3001]
Type = VarAdd
Trigger1 = (Command="up"&&Var(0)>0)||(Command="down"&&Var(0)<2)
V = 0
Value = IFelse(Command = "down", 1, -1)
[State 3001]
Type = RemoveExplod
Trigger1 = Command = "up" || Command = "down"
ID = 3001

[State 3001]
Type = ParentVarSet
Trigger1 = Var(0)=0 && (Command = "fwd" || Command = "back")
V = 5
Value = IFelse(Command = "fwd", 1, 0)
[State 3001]
Type = RemoveExplod
Trigger1 = Var(0)=0 && (Command = "fwd" || Command = "back")
ID = 3002

[State 3001]
Type = ParentVarSet
Trigger1 = Var(0)=1 && (Command = "fwd" || Command = "back")
V = 10
Value = IFelse(Command = "fwd", 1, 0)
[State 3001]
Type = RemoveExplod
Trigger1 = Var(0)=1 && (Command = "fwd" || Command = "back")
ID = 3003
[State 3001]
Type = RemoveExplod
Trigger1 = (Var(0)=[1,2]) && (Command="fwd"||Command="back")
ID = 3000
[State 3001]
Type = RemoveExplod
Trigger1 = Var(0)=1 && (Command = "fwd" || Command = "back")
ID = 3008

[State 3001];コマンド変更
Type = ParentVarSet
Trigger1 = Var(0)=2 && (Command = "fwd" || Command = "back")
V = 13
Value = IFelse(Command="fwd", 0, 1)
[State 3001]
Type = RemoveExplod
Trigger1 = Var(0)=2 && (Command = "fwd" || Command = "back")
ID = 3009
[State 3001]
Type = Explod
Trigger1 = Time = 0 || (Var(0)=2 && (Command="fwd"||Command="back"))
Anim = 3012-Parent,Var(13)
ID = 3009
Pos = 160, 134
PosType = Left
OwnPal = 1
SprPriority = 101
PauseMoveTime = -1
BindTime = -1

[State 3001];モード
Type = Explod
Trigger1 = Time = 0
Anim = 3013
ID = 3010
Pos = 160, 110
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = (Time=0&&Parent,Var(10)) || (Var(0)=1&&Command="fwd")
Anim = 3014
ID = 3003
Pos = 160, 110
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1

[State 3001]
Type = Explod
Trigger1 = Time = 0 || ((Var(0)=[1,2]) && (Command="fwd"||Command="back"))
Anim = 3000
ID = 3000
Pos = 160, 130 + Parent,Var(10)*18
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0 || ((Var(0)=[1,2]) && (Command="fwd"||Command="back"))
Anim = IFelse(Parent,Facing=1, 3001, 3003) +Parent,Var(13)
ID = 3000
Pos = 160, 130 + Parent,Var(10)*18
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0 || ((Var(0)=[1,2]) && (Command="fwd"||Command="back"))
Anim = 3010
ID = 3000
Pos = 160, 110
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = (!Parent,Var(10)&&Time=0) || (Var(0)=1&&Command="back")
Anim = 3005
ID = 3008
Pos = 160, 130
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = (!Parent,Var(10)&&Time=0) || ((Var(0)=1)&&Command="back")
Anim = IFelse(Parent,Facing=1, 3006, 3007)
ID = 3008
Pos = 160, 130
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0 || Command = "down" || Command = "up"
Anim = 3015
ID = 3001
Pos = 160, 110+Var(0)*14
PosType = Left
OwnPal = 1
SprPriority = 101
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0 || (Var(0)=0 && (Command="fwd"||Command="back"))
Anim = 3011+Parent,Var(5)
ID = 3002
Pos = 160, 110
PosType = Left
OwnPal = 1
SprPriority = 101
PauseMoveTime = -1
BindTime = -1

[State 3001]
Type = Explod
Trigger1 = (Time = 0 || ((Var(0)=[1,2])&&(Command="fwd"||Command="back"))) && !Parent,Var(10)
Anim = IFelse(Parent,Var(13), IFelse(Parent,Facing=1, 3009, 3016), 3008)
ID = 3000
Pos = 160, 130
PosType = Left
OwnPal = 1
SprPriority = 100
PauseMoveTime = -1
BindTime = -1
[State 3001]
Type = Explod
Trigger1 = Time = 0 || ((Var(0)=[1,2])&&(Command="fwd"||Command="back"))
Anim = IFelse(Parent,Var(10)||Parent,Var(13), 3018, IFelse(Parent,Facing=1, 3017, 3019))
ID = 3000
Pos = 160, 130+Parent,Var(10)*18
PosType = Left
OwnPal = 1
SprPriority = 100
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
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3004
[State 3002]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3005
[State 3002]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3006
[State 3002]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3007
[State 3002]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3008
[State 3002]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3009
[State 3002]
Type = RemoveExplod
Trigger1 = Time = 0
ID = 3010

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
Anim = 91
Pos = 0, 0
PosType = Left
BindTime = -1
SprPriority = -100
RemoveTime = 101
PauseMoveTime = 101
[State 3003]
Type = Explod
Trigger1 = Time = 0
Anim = 91
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
Anim = 91
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
Anim = 91
Pos = 319, 239
PosType = Left
BindTime = -1
SprPriority = -100
RemoveTime = 101
PauseMoveTime = 101
Facing = -1
VFacing = -1