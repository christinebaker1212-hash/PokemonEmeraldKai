[StateDef 3000];ポーズヘルパー待機
Type = S
Ctrl = 0
Anim = 99

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