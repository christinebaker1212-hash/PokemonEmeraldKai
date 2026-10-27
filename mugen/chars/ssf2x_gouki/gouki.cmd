;------------------------------------------------------------------------------
;-| ボタンリマップ（ボタンコンフィグ）|----------------------------------------

;[Remap]
;x = x      ;Ｘボタンの入力判定を実際に押すボタンに割り当てる。
;y = y      ;Ｙボタン　　　　　　　　　〃
;z = z      ;Ｚボタン　　　　　　　　　〃
;a = a      ;Ａボタン　　　　　　　　　〃
;b = b      ;Ｂボタン　　　　　　　　　〃
;c = c      ;Ｃボタン　　　　　　　　　〃
;s = s      ;スタートボタン　　　　　　〃

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;-| デフォルト設定 |-----------------------------------------------------------

;[Defaults]
;command.time = 15  ;標準のコマンド入力受付時間。
                   ;各コマンドで省略している場合に有効。
                   ;このパラメータを消した場合、デフォルトは１フレームになる。
                   ;（　M.U.G.E.Nでの１フレーム　＝　１／６０秒　）

;command.buffer.time = 1  ;標準のコマンド入力記憶時間。
                         ;入力した直後にコマンドを記憶し、
                         ;指を離してもコマンドが成功している状態を
                         ;ここで設定した時間の分維持する。
                         ;１～３０フレームまでの間で設定可能。
                         ;デフォルトは１フレーム。

;==============================================================================
; コマンド定義パート（入力キーを設定する）
;==============================================================================
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;■書式の決まり■
;
; [Command]         ：入力コマンドを１個定義する。
; name = "***"      ：コマンド名を決める。大文字と小文字も区別される。
; command = ###     ：実際に入力するキーを組み合わせる。詳細は後述。
; time = &&&        ：入力受付時間を設定（オプション）。
; buffer.time = @@@ ：入力記憶時間を設定（オプション）。
;
; 小ネタでも説明している通り、登録が可能な数は最大『１２８個』まで。
;
;
;※『必須コマンド名』と書いてるコマンドは、システム側で処理してます。
;　コマンド名を変えたり、消してはいけません。キーの変更は出来ます。
;------------------------------------------------------------------------------
;■必要なアルファベットと記号■
;
; 上記の「command = ###」の『###』に該当する部分で、
; 組み合わせるキーとボタンを設定しなければならない。
;
; ※設定したキーやボタンはM.U.G.E.Nのオプションモードにある
;  「キーコンフィグ」にて設定したキーなどに対応しています。
;
; ★方向キー★（全て必ず大文字で）
;
; 　B 　：「後方」にキーを入れる（ Backward ）
; 　D 　：「下方」にキーを入れる（ Downward ）
; 　F 　：「前方」にキーを入れる（ Forward ）
; 　U 　：「上方」にキーを入れる（ Upward ）
;
; 　DB　：「後ろ斜め下」にキーを入れる（「D」と「B」が同時に入力された事を認識）
; 　UB　：「後ろ斜め上」にキーを入れる（「U」と「B」が同時に入力された事を認識）
; 　DF　：「前斜め下」にキーを入れる（「D」と「F」が同時に入力された事を認識）
; 　UF　：「前斜め上」にキーを入れる（「U」と「F」が同時に入力された事を認識）
;
; ★ボタン★（全て必ず小文字で）
;
; 　a 　：「Ａボタン」を押す
; 　b 　：「Ｂボタン」を押す
; 　c 　：「Ｃボタン」を押す
; 　x 　：「Ｘボタン」を押す
; 　y 　：「Ｙボタン」を押す
; 　z 　：「Ｚボタン」を押す
; 　s 　：「スタートボタン」を押す
;
; ★記号★（入力効果を変化させる命令）
;
; 　/ 　：（スラッシュ）キーやボタンを「押しっぱなし」にしている事を認識する場合に追加する
;
; 　　（例）：　/b       = Ｂボタンを押したままにする
; 　　　　　　　/F       = 前キーを押したままにする
; 　　　　　　　/U,z     = 上キーを押したままＺボタンを入力する
;
;　　━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
;
; 　~ 　：（チルダ）キーやボタンが「離された時」を認識する場合に追加する
;
; 　　（例）：　~x       = Ｘボタンを離す
; 　　　　　　　~DF      =「前斜め下」のキーを離す
; 　　　　　　　~DB,F,c  =「後ろ斜め下」を離した後に前キー・Ｃボタンの順番に入力する
;
; 　　　　　　※「ボタンを離すまでの時間（溜め時間）」も設定する事が出来る
;
; 　　　　　　　~30x     = Ｘボタンを押したままにして、３０フレーム以上経ったら離す
; 　　　　　　　~50B,F,a = 後ろキーを５０フレームまで溜めて前キー・Ａボタンの順番に入力する
;
;　　━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
;
; 　$ 　：（ドル）方向キーの「複数の内どれかが入力されている事」を認識する場合に追加する
;
; 　　（例）：　$B       =「後方」「後ろ斜め下」「後ろ斜め上」のどれかが入力されている状態
; 　　　　　　　$UF      =「前」「上」「前斜め上」のどれかが入力されている状態
;
; 　　　　　　※この記号は「方向キー」でしか使えません。
;
;　　━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
;
; 　+ 　：（プラス）ボタンを「同時押し」している事を認識する場合に追加する
;
; 　　（例）：　x+y      = ＸボタンとＹボタンを同時押しする
; 　　　　　　　a+b+c    = ＡボタンとＢボタンとＣボタンを同時押しする
;
; 　　　　　　※この記号は「ボタン」でしか使えません。
;
;　　━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
;
; 　> 　：（グレーターザン）大なり（Win版で追加された記号）
; 　　　　　　　　　　　　「他のキーが入力されていない事を確認して、そのキーを押す」場合
;
; 　　（例）：　a,>~a    = Ａボタン以外が入力されていない状態でＡボタンを離す
; 　　　　　　　F,>~F,>F = 前キー以外が入力されていない状態で前キーを離し、
;　　　　　　　　　　　　　もう一度前キーを入力する
;
;-------------------------------------------------------------------------------
; ●これらの記号は全て組み合わせて使う事が出来る●
;
; 　　（例）：　~80$DB,DF,F,/a+y+c
; 　　　　　　　
; 　　　　　　「後方」「下」「後ろ斜め下」のどれかを８０フレームまで溜めて
; 　　　　　　「前斜め下」→「前」を入力した後、ＡとＹとＣを同時押ししたままにする
;
;
;-------------------------------------------------------------------------------
[Command]
Name    = "AI_00"
Command =  a, U ,D ,s ,z
Time = 1

[Command]
Name    = "AI_01"
Command =  a, D, U ,s ,z
Time = 1

[Command]
Name    = "AI_02"
Command =  b, U ,D ,s ,z
Time = 1

[Command]
Name    = "AI_03"
Command =  b, D ,U ,s ,z
Time = 1

[Command]
Name    = "AI_04"
Command =  c, U ,D ,s ,z
Time = 1

[Command]
Name    = "AI_05"
Command =  c, D ,U ,s ,z
Time = 1

[Command]
Name    = "AI_06"
Command =  x, U ,D ,s ,c
Time = 1

[Command]
Name    = "AI_07"
Command =  x, D ,U ,s ,c
Time = 1

[Command]
Name    = "AI_08"
Command =  y, U ,D ,s ,c
Time = 1

[Command]
Name    = "AI_09"
Command =  y, D ,U ,s ,c
Time = 1

[Command]
Name    = "AI_10"
Command =  z, U ,D ,s ,c
Time = 1

[Command]
Name    = "AI_11"
Command =  z, D ,U ,s ,c
Time = 1

[Command]
Name    = "AI_12"
Command =  a, UF ,DB ,s ,z
Time = 1

[Command]
Name    = "AI_13"
Command =  a, DB ,UF ,s ,z
Time = 1

[Command]
Name    = "AI_14"
Command =  b, UF ,s ,DB ,z
Time = 1

[Command]
Name    = "AI_15"
Command =  b, UF ,DB ,s ,z
Time = 1

[Command]
Name    = "AI_16"
Command =  c, UF ,DB ,s ,z
Time = 1

[Command]
Name    = "AI_17"
Command =  c, UF ,s ,DB ,z
Time = 1

[Command]
Name    = "AI_18"
Command =  x, UF ,DB ,s ,c
Time = 1

[Command]
Name    = "AI_19"
Command =  x, UF ,s ,DB ,c
Time = 1

[Command]
Name    = "AI_20"
Command =  y, UF ,DB ,s ,c
Time = 1

[Command]
Name    = "AI_21"
Command =  y, UF ,s ,DB ,c
Time = 1

[Command]
Name    = "AI_22"
Command =  z, UF ,DB ,s ,c
Time = 1

[Command]
Name    = "AI_23"
Command =  z, UF ,s ,DB ,c
Time = 1

[Command]
Name    = "AI_24"
Command =  a, b, c, U, D, s ,z
Time = 1

[Command]
Name    = "AI_25"
Command =  a, c, b, U, D, s ,z
Time = 1

[Command]
Name    = "AI_26"
Command =  b, a, c, U, D, s ,z
Time = 1

[Command]
Name    = "AI_27"
Command =  b, c, a, U, D, s ,z
Time = 1

[Command]
Name    = "AI_28"
Command =  c, a, b, U, D, s ,z
Time = 1

[Command]
Name    = "AI_29"
Command =  c, b, a, U, D, s ,z
Time = 1

[Command]
Name    = "AI_30"
Command =  x, y, z, U, D, s ,c
Time = 1

[Command]
Name    = "AI_31"
Command =  x, z, y, U, D, s ,c
Time = 1

[Command]
Name    = "AI_32"
Command =  y, x, z, U, D, s ,c
Time = 1

[Command]
Name    = "AI_33"
Command =  y, z, x, U, D, s ,c
Time = 1

[Command]
Name    = "AI_34"
Command =  z, x, y, U, D, s ,c
Time = 1

[Command]
Name    = "AI_35"
Command =  z, y, x, U, D, s ,c
Time = 1

[Command]
Name    = "AI_36"
Command =  a, b, c, D, U, s ,z
Time = 1

[Command]
Name    = "AI_37"
Command =  a, c, b, D, U, s ,z
Time = 1

[Command]
Name    = "AI_38"
Command =  b, a, c, D, U, s ,z
Time = 1

[Command]
Name    = "AI_39"
Command =  b, c, a, D, U, s ,z
Time = 1

[Command]
Name    = "AI_40"
Command =  c, a, b, D, U, s ,z
Time = 1

[Command]
Name    = "AI_41"
Command =  c, b, a, D, U, s ,z
Time = 1

[Command]
Name    = "AI_42"
Command =  x, y, z, D, U, s ,c
Time = 1

[Command]
Name    = "AI_43"
Command =  x, z, y, D, U, s ,c
Time = 1

[Command]
Name    = "AI_44"
Command =  y, x, z, D, U, s ,c
Time = 1

[Command]
Name    = "AI_45"
Command =  y, z, x, D, U, s ,c
Time = 1

[Command]
Name    = "AI_46"
Command =  z, x, y, D, U, s ,c
Time = 1

[Command]
Name    = "AI_47"
Command =  z, y, x, D, U, s ,c
Time = 1

[Command]
Name    = "AI_48"
Command =  D, a, U, z, s, c
Time = 1

[Command]
Name    = "AI_49"
Command =  D, a, U, z, c, y
Time = 1

[Command]
Name    = "AI_50"
Command =  D, a, U, y, b, x
Time = 1

;------------------------------------------------------------------------------
;-| 超必殺技 |-----------------------------------------------------------------

[Command]
name = "shungoku"
command = ~x, x, F, a, z
time = 37

[Command]
name = "shungoku"
command = ~x, x, F, a, ~z
time = 37

[Command]
name = "m_hadou1"
command = ~D, DF, F, D, DF, x
time = 23

[Command]
name = "m_hadou1"
command = ~D, DF, F, D, DF, ~x
time = 23

[Command]
name = "m_hadou2"
command = ~D, DF, F, D, DF, y
time = 23

[Command]
name = "m_hadou2"
command = ~D, DF, F, D, DF, ~y
time = 23

[Command]
name = "m_hadou3"
command = ~D, DF, F, D, DF, z
time = 23

[Command]
name = "m_hadou3"
command = ~D, DF, F, D, DF, ~z
time = 23

[Command]
name = "m_syou1"
command = ~D, DF, F, D, DF, a
time = 23

[Command]
name = "m_syou1"
command = ~D, DF, F, D, DF, ~a
time = 23

[Command]
name = "m_syou2"
command = ~D, DF, F, D, DF, b
time = 23

[Command]
name = "m_syou2"
command = ~D, DF, F, D, DF, ~b
time = 23

[Command]
name = "m_syou3"
command = ~D, DF, F, D, DF, c
time = 23

[Command]
name = "m_syou3"
command = ~D, DF, F, D, DF, ~c
time = 23

;------------------------------------------------------------------------------
;-| 必殺技 |-------------------------------------------------------------------
[Command]
name = "ashura_P_F"
command = ~F, D, DF, x+y
time = 18

[Command]
name = "ashura_P_F"
command = ~F, D, DF, y+z
time = 18

[Command]
name = "ashura_P_F"
command = ~F, D, DF, z+x
time = 18

[Command]
name = "ashura_P_B"
command = ~B, D, DB, x+y
time = 18

[Command]
name = "ashura_P_B"
command = ~B, D, DB, y+z
time = 18

[Command]
name = "ashura_P_B"
command = ~B, D, DB, z+x
time = 18

[Command]
name = "ashura_K_F"
command = ~F, D, DF, a+b
time = 18

[Command]
name = "ashura_K_F"
command = ~F, D, DF, b+c
time = 18

[Command]
name = "ashura_K_F"
command = ~F, D, DF, c+a
time = 18

[Command]
name = "ashura_K_B"
command = ~B, D, DB, a+b
time = 18

[Command]
name = "ashura_K_B"
command = ~B, D, DB, b+c
time = 18

[Command]
name = "ashura_K_B"
command = ~B, D, DB, c+a
time = 18

[Command]
name = "shakunetsu1"
command = ~F, DF, D, DB, B, x
time = 20

[Command]
name = "shakunetsu1"
command = ~F, DF, D, DB, B, ~x
time = 20

[Command]
name = "shakunetsu2"
command = ~F, DF, D, DB, B, y
time = 20

[Command]
name = "shakunetsu2"
command = ~F, DF, D, DB, B, ~y
time = 20

[Command]
name = "shakunetsu3"
command = ~F, DF, D, DB, B, z
time = 20

[Command]
name = "shakunetsu3"
command = ~F, DF, D, DB, B, ~z
time = 20

[Command]
name = "zenten1"
command = ~D, DB, B, x
time = 12

[Command]
name = "zenten1"
command = ~D, DB, B, ~x
time = 12

[Command]
name = "zenten2"
command = ~D, DB, B, y
time = 12

[Command]
name = "zenten2"
command = ~D, DB, B, ~y
time = 12

[Command]
name = "zenten3"
command = ~D, DB, B, z
time = 12

[Command]
name = "zenten3"
command = ~D, DB, B, ~z
time = 12

[Command]
name = "hadou1"
command = ~D, DF, F, x
time = 12

[Command]
name = "hadou1"
command = ~D, DF, F, ~x
time = 12

[Command]
name = "hadou2"
command = ~D, DF, F, y
time = 12

[Command]
name = "hadou2"
command = ~D, DF, F, ~y
time = 12

[Command]
name = "hadou3"
command = ~D, DF, F, z
time = 12

[Command]
name = "hadou3"
command = ~D, DF, F, ~z
time = 12

[Command]
name = "shoryu1"
command = ~F, D, DF, x
time = 12

[Command]
name = "shoryu1"
command = ~F, D, DF, ~x
time = 12

[Command]
name = "shoryu2"
command = ~F, D, DF, y
time = 12

[Command]
name = "shoryu2"
command = ~F, D, DF, ~y
time = 12

[Command]
name = "shoryu3"
command = ~F, D, DF, z
time = 12

[Command]
name = "shoryu3"
command = ~F, D, DF, ~z
time = 12

[Command]
name = "tatsumaki1"
command = ~D, DB, B, a
time = 12

[Command]
name = "tatsumaki1"
command = ~D, DB, B, ~a
time = 12

[Command]
name = "tatsumaki2"
command = ~D, DB, B, b
time = 12

[Command]
name = "tatsumaki2"
command = ~D, DB, B, ~b
time = 12

[Command]
name = "tatsumaki3"
command = ~D, DB, B, c
time = 12

[Command]
name = "tatsumaki3"
command = ~D, DB, B, ~c
time = 12

[Command]
name = "hyakki1"
command = ~F, D, DF, a
time = 12

[Command]
name = "hyakki1"
command = ~F, D, DF, ~a
time = 12

[Command]
name = "hyakki2"
command = ~F, D, DF, b
time = 12

[Command]
name = "hyakki2"
command = ~F, D, DF, ~b
time = 12

[Command]
name = "hyakki3"
command = ~F, D, DF, c
time = 12

[Command]
name = "hyakki3"
command = ~F, D, DF, ~c
time = 12

;------------------------------------------------------------------------------
;-| キー２回連続入力 |---------------------------------------------------------

[Command]
name = "FF"       ;必須コマンド名
command = F, F
time = 12

[Command]
name = "BB"       ;必須コマンド名
command = B, B
time = 12

;------------------------------------------------------------------------------
;-| 同時押し |-----------------------------------------------------------------

[Command]
name = "recovery" ;必須コマンド名
command = x+a
time = 11

[Command]
name = "y+b"
command = y+b
time = 11

[Command]
name = "z+c"
command = z+c
time = 11

[Command]
name = "x+y"
command = x+y
time = 11

[Command]
name = "x+z"
command = x+z
time = 11

[Command]
name = "y+z"
command = y+z
time = 11

[Command]
name = "a+b"
command = a+b
time = 11

[Command]
name = "a+c"
command = a+c
time = 11

[Command]
name = "b+c"
command = b+c
time = 11

;------------------------------------------------------------------------------
;-| ボタン単発 |---------------------------------------------------------------

[command]
name = "fwd"
command = F
time = 1

[command]
name = "back"
command = B
time = 1

[command]
name = "up"
command = U
time = 1

[command]
name = "down"
command = D
time=1

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
name = "start"
command = s
time = 1

[Command];（前ブロッキングコマンド）
name = "block_f"
command = F
time = 1

[Command];（下ブロッキングコマンド）
name = "block_d"
command = D
time = 1

;------------------------------------------------------------------------------
;-| 方向キー |-----------------------------------------------------------------

[Command]
name = "holdfwd"   ;必須コマンド名
command = /$F
time = 1

[Command]
name = "holdback"  ;必須コマンド名
command = /$B
time = 1

[Command]
name = "holdup"    ;必須コマンド名
command = /$U
time = 1

[Command]
name = "holddown"  ;必須コマンド名
command = /$D
time = 1

[Command]
name = "chargec"
command = /c
time = 1

[Command]
name = "chargez"
command = /z
time = 1

;------------------------------------------------------------------------------
; ■準常時監視ステート（－１）■
; コマンドファイル（のステートエントリーパート）に必要な項目です。
; 絶対に消してはいけないので要注意。
;
; 通常の食らい状態以外の「P2StateNo」や「TargetState」等で制御された、
; 作成者が任意に指定した相手側の食らいステートに限り、
; 登録したステートコントローラが有効にはなりません。
;------------------------------------------------------------------------------

[Statedef -1]

[State 100]
type = VarSet
triggerall = time = 2
trigger1 = stateno != 52
trigger1 = stateno != 455
trigger1 = stateno != 475
trigger1 = stateno != 1605
var(11) = 0

[State 100]
type = VarSet
triggerall = Command = "x" || Command = "y" || Command = "z"
triggerall = Command != "shungoku"
trigger1 = stateno = [200,330]
trigger1 = animtime = [ifelse((var(3)=[1,2]),-5,-2),0]
trigger2 = stateno = [350,450]
trigger2 = Pos Y >= -55
trigger3 = stateno = 475
trigger3 = time >= 2
trigger4 = stateno = 1602 && Var(12) = 20
trigger4 = (Time >= 31 && var(6) = [0,1]) || (Time >= 21 && var(6) = [2,3])
trigger5 = stateno = 1605
trigger6 = stateno = [90,105]
trigger6 = animtime = [-2,0]
var(11) = ifelse(Command = "x", 4, ifelse(Command = "y", 5, 6))
ignorehitpause = 1

[State 100]
type = VarSet
triggerall = Command = "a" || Command = "b" || Command = "c"
triggerall = Command != "shungoku"
trigger1 = stateno = [200,330]
trigger1 = animtime = [ifelse((var(3)=[1,2]),-5,-2),0]
trigger2 = stateno = [350,450]
trigger2 = Pos Y >= -55
trigger3 = stateno = 475
trigger3 = time >= 2
trigger4 = stateno = 1602 && Var(12) = 20
trigger4 = (Time >= 31 && var(6) = [0,1]) || (Time >= 21 && var(6) = [2,3])
trigger5 = stateno = 1605
trigger6 = stateno = [90,105]
trigger6 = animtime = [-2,0]
var(11) = ifelse(Command = "a", 1, ifelse(Command = "b", 2, 3))
ignorehitpause = 1

;trigger2 = (stateno != [220,225]) && (stateno != [270,275])
;trigger2 = stateno != 310 && stateno != 330
;trigger2 = !(var(12)=20 && stateno = 260)
;trigger2 = var(3) = [1,2]
;trigger2 = HitPauseTime = [1,5]

[State 100]
type = VarSet
triggerall = Command = "x"
triggerall = stateno = 300
trigger1 = var(3) = [1,2]
trigger1 = HitPauseTime = [1,6]
var(11) = 4
ignorehitpause = 1

[State 100]
type = VarSet
triggerall = Command = "a"
triggerall = stateno = 320
trigger1 = time > 4
trigger2 = var(3) = [1,2]
trigger2 = HitPauseTime = [1,6]
var(11) = 1
ignorehitpause = 1

[State 100]
type = VarSet
trigger1 = Command = "shungoku"
trigger2 = var(11)
trigger2 = Command = "holdback" && Command != "holdup"
trigger2 = InGuardDist
var(11) = 0
ignorehitpause = 1

;---------------------------------------------------------------------------
;瞬獄殺
[State -1]
type = ChangeState
value = 3200
triggerall = !var(59)
triggerall = (Var(12) = 10 && numhelper(10010) && helper(10010),var(2) = 1) || var(12) = 20
triggerall = roundstate = 2
triggerall = power >= 3000 
triggerall = Command = "shungoku" || (var(12)=10 && Command = "z+c")
triggerall = statetype != A
trigger1 = ctrl
trigger2 = Stateno = [200,330]
ignorehitpause = 0

;---------------------------------------------------------------------------
;滅殺豪波動
[State -1]
type = ChangeState
value = ifelse(var(12)=20,3020,3000)
triggerall = !var(59)
triggerall = (Var(12) = 10 && numhelper(10010) && helper(10010),var(2) = 1) || var(12) = 20
triggerall = roundstate = 2
triggerall = (var(12)=10 && power >= 3000) || (var(12)=20 && power >= 1000)
triggerall = !NumProj
triggerall = (Command = "m_hadou1" || Command = "m_hadou2" || Command = "m_hadou3" ) || (var(12)=10 && Command = "y+b")
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && ((var(3) = [1,2])|| time < 1)
trigger3 = StateNo = 205 && ((var(3) = [1,2])|| time < 1)
trigger4 = StateNo = 210 && var(3) = [1,2]
trigger4 = (anim = 210 && animelemtime(5)<0)||(anim = 211 && animelemtime(4)<2)
trigger5 = StateNo = 215 && (var(3) = [1,2]) && animelemtime(3)<2
trigger6 = StateNo = 220 && (var(3) = [1,2]) && (anim = 221 && animelemtime(4)<2)
trigger7 = StateNo = 225 && (var(3) = [1,2]) && animelemtime(4)<2
trigger8 = StateNo = 250 && (var(3) = [1,2]) && (anim = 251 && animelemtime(4)<2)
trigger9 = StateNo = 260 && (var(3) = [1,2]) && (anim = 261 && animelemtime(4)<0)
trigger10 = StateNo = 265 && (var(3) = [1,2]) && animelemtime(3)<2
trigger11 = StateNo = 270 && (var(3) = [1,2]) && (anim = 270 && animelemtime(4)<2)
trigger12 = StateNo = 300 && ((var(3) = [1,2])|| time < 1)
trigger13 = StateNo = 305 && (var(3) = [1,2]) && animelemtime(4)<2
trigger14 = StateNo = 310 && (var(3) = [1,2]) && animelemtime(4)<2
trigger15 = StateNo = 320 && ((var(3) = [1,2])|| time < 1)
trigger16 = StateNo = 325 && (var(3) = [1,2]) && animelemtime(4)<2
trigger17 = StateNo = 330 && (var(3) = [1,2]) && animelemtime(3)<2
ignorehitpause = 0

;---------------------------------------------------------------------------
;滅殺豪昇龍
[State -1]
type = ChangeState
value = ifelse(var(12)=20,3120,3100)
triggerall = !var(59)
triggerall = (Var(12) = 10 && numhelper(10010) && helper(10010),var(2) = 1) || var(12) = 20
triggerall = roundstate = 2
triggerall = (var(12)=10 && power >= 3000) || (var(12)=20 && power >= 1000)
triggerall = (Command = "m_syou1" || Command = "m_syou2" || Command = "m_syou3") || (var(12)=10 && Command = "recovery")
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && ((var(3) = [1,2])|| time < 1)
trigger3 = StateNo = 205 && ((var(3) = [1,2])|| time < 1)
trigger4 = StateNo = 210 && var(3) = [1,2]
trigger4 = (anim = 210 && animelemtime(5)<0)||(anim = 211 && animelemtime(4)<2)
trigger5 = StateNo = 215 && (var(3) = [1,2]) && animelemtime(3)<2
trigger6 = StateNo = 220 && (var(3) = [1,2]) && (anim = 221 && animelemtime(4)<2)
trigger7 = StateNo = 225 && (var(3) = [1,2]) && animelemtime(4)<2
trigger8 = StateNo = 250 && (var(3) = [1,2]) && (anim = 251 && animelemtime(4)<2)
trigger9 = StateNo = 260 && (var(3) = [1,2]) && (anim = 261 && animelemtime(4)<0)
trigger10 = StateNo = 265 && (var(3) = [1,2]) && animelemtime(3)<2
trigger11 = StateNo = 270 && (var(3) = [1,2]) && (anim = 270 && animelemtime(4)<2)
trigger12 = StateNo = 300 && ((var(3) = [1,2])|| time < 1)
trigger13 = StateNo = 305 && (var(3) = [1,2]) && animelemtime(4)<2
trigger14 = StateNo = 310 && (var(3) = [1,2]) && animelemtime(4)<2
trigger15 = StateNo = 320 && ((var(3) = [1,2])|| time < 1)
trigger16 = StateNo = 325 && (var(3) = [1,2]) && animelemtime(4)<2
trigger17 = StateNo = 330 && (var(3) = [1,2]) && animelemtime(3)<2
ignorehitpause = 0

;---------------------------------------------------------------------------
;天魔豪斬空
[State -1]
type = ChangeState
value = 3300
triggerall = !var(59)
triggerall = var(12) = 20
triggerall = roundstate = 2
triggerall = !NumProj
triggerall = Power >= 1000
triggerall = Command = "m_hadou1" || Command = "m_hadou2" || Command = "m_hadou3"
triggerall = statetype = A
trigger1 = ctrl
ignorehitpause = 0

;---------------------------------------------------------------------------
;阿修羅閃空
[State -1]
type = ChangeState
value = 1600
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = Command = "ashura_P_F" || Command = "ashura_P_B" || Command = "ashura_K_F" || Command = "ashura_K_B"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && ((var(3) = [1,2])|| time < 1)
trigger3 = StateNo = 205 && ((var(3) = [1,2])|| time < 1)
trigger4 = StateNo = 210 && var(3) = [1,2]
trigger4 = (anim = 210 && animelemtime(5)<0)||(anim = 211 && animelemtime(4)<2)
trigger5 = StateNo = 215 && (var(3) = [1,2]) && animelemtime(3)<2
trigger6 = StateNo = 220 && (var(3) = [1,2]) && (anim = 221 && animelemtime(4)<2)
trigger7 = StateNo = 225 && (var(3) = [1,2]) && animelemtime(4)<2
trigger8 = StateNo = 250 && (var(3) = [1,2]) && (anim = 251 && animelemtime(4)<2)
trigger9 = StateNo = 260 && (var(3) = [1,2]) && (anim = 261 && animelemtime(4)<0)
trigger10 = StateNo = 265 && (var(3) = [1,2]) && animelemtime(3)<2
trigger11 = StateNo = 270 && (var(3) = [1,2]) && (anim = 270 && animelemtime(4)<2)
trigger12 = StateNo = 300 && ((var(3) = [1,2])|| time < 1)
trigger13 = StateNo = 305 && (var(3) = [1,2]) && animelemtime(4)<2
trigger14 = StateNo = 310 && (var(3) = [1,2]) && animelemtime(4)<2
trigger15 = StateNo = 320 && ((var(3) = [1,2])|| time < 1)
trigger16 = StateNo = 325 && (var(3) = [1,2]) && animelemtime(4)<2
trigger17 = StateNo = 330 && (var(3) = [1,2]) && animelemtime(3)<2
ignorehitpause = 0

;---------------------------------------------------------------------------
;豪昇龍拳
[State -1]
type = ChangeState
value = ifelse(var(12)=20,1020,1000)
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = Command = "shoryu1" || Command = "shoryu2" || Command = "shoryu3"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && ((var(3) = [1,2])|| time < 1)
trigger3 = StateNo = 205 && ((var(3) = [1,2])|| time < 1)
trigger4 = StateNo = 210 && var(3) = [1,2]
trigger4 = (anim = 210 && animelemtime(5)<0)||(anim = 211 && animelemtime(4)<2)
trigger5 = StateNo = 215 && (var(3) = [1,2]) && animelemtime(3)<2
trigger6 = StateNo = 220 && (var(3) = [1,2]) && (anim = 221 && animelemtime(4)<2)
trigger7 = StateNo = 225 && (var(3) = [1,2]) && animelemtime(4)<2
trigger8 = StateNo = 250 && (var(3) = [1,2]) && (anim = 251 && animelemtime(4)<2)
trigger9 = StateNo = 260 && (var(3) = [1,2]) && (anim = 261 && animelemtime(4)<0)
trigger10 = StateNo = 265 && (var(3) = [1,2]) && animelemtime(3)<2
trigger11 = StateNo = 270 && (var(3) = [1,2]) && (anim = 270 && animelemtime(4)<2)
trigger12 = StateNo = 300 && ((var(3) = [1,2])|| time < 1)
trigger13 = StateNo = 305 && (var(3) = [1,2]) && animelemtime(4)<2
trigger14 = StateNo = 310 && (var(3) = [1,2]) && animelemtime(4)<2
trigger15 = StateNo = 320 && ((var(3) = [1,2])|| time < 1)
trigger16 = StateNo = 325 && (var(3) = [1,2]) && animelemtime(4)<2
trigger17 = StateNo = 330 && (var(3) = [1,2]) && animelemtime(3)<2
trigger18 = StateNo = 1105 && time >= 1
ignorehitpause = 0

;---------------------------------------------------------------------------
;百鬼襲
[State -1]
type = ChangeState
value = 1700
triggerall = !var(59)
triggerall = Var(12) = 20
triggerall = roundstate = 2
triggerall = Command = "hyakki1" || Command = "hyakki2" || Command = "hyakki3"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && ((var(3) = [1,2])|| time < 1)
trigger3 = StateNo = 205 && ((var(3) = [1,2])|| time < 1)
trigger4 = StateNo = 210 && var(3) = [1,2]
trigger4 = (anim = 210 && animelemtime(5)<0)||(anim = 211 && animelemtime(4)<2)
trigger5 = StateNo = 215 && (var(3) = [1,2]) && animelemtime(3)<2
trigger6 = StateNo = 220 && (var(3) = [1,2]) && (anim = 221 && animelemtime(4)<2)
trigger7 = StateNo = 225 && (var(3) = [1,2]) && animelemtime(4)<2
trigger8 = StateNo = 250 && (var(3) = [1,2]) && (anim = 251 && animelemtime(4)<2)
trigger9 = StateNo = 260 && (var(3) = [1,2]) && (anim = 261 && animelemtime(4)<0)
trigger10 = StateNo = 265 && (var(3) = [1,2]) && animelemtime(3)<2
trigger11 = StateNo = 270 && (var(3) = [1,2]) && (anim = 270 && animelemtime(4)<2)
trigger12 = StateNo = 300 && ((var(3) = [1,2])|| time < 1)
trigger13 = StateNo = 305 && (var(3) = [1,2]) && animelemtime(4)<2
trigger14 = StateNo = 310 && (var(3) = [1,2]) && animelemtime(4)<2
trigger15 = StateNo = 320 && ((var(3) = [1,2])|| time < 1)
trigger16 = StateNo = 325 && (var(3) = [1,2]) && animelemtime(4)<2
trigger17 = StateNo = 330 && (var(3) = [1,2]) && animelemtime(3)<2
ignorehitpause = 0

;---------------------------------------------------------------------------
;竜巻斬空脚
[State -1]
type = ChangeState
value = ifelse(var(12)=20,1120,1100)
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = Command = "tatsumaki1" || Command = "tatsumaki2" || Command = "tatsumaki3"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && ((var(3) = [1,2])|| time < 1)
trigger3 = StateNo = 205 && ((var(3) = [1,2])|| time < 1)
trigger4 = StateNo = 210 && var(3) = [1,2]
trigger4 = (anim = 210 && animelemtime(5)<0)||(anim = 211 && animelemtime(4)<2)
trigger5 = StateNo = 215 && (var(3) = [1,2]) && animelemtime(3)<2
trigger6 = StateNo = 220 && (var(3) = [1,2]) && (anim = 221 && animelemtime(4)<2)
trigger7 = StateNo = 225 && (var(3) = [1,2]) && animelemtime(4)<2
trigger8 = StateNo = 250 && (var(3) = [1,2]) && (anim = 251 && animelemtime(4)<2)
trigger9 = StateNo = 260 && (var(3) = [1,2]) && (anim = 261 && animelemtime(4)<0)
trigger10 = StateNo = 265 && (var(3) = [1,2]) && animelemtime(3)<2
trigger11 = StateNo = 270 && (var(3) = [1,2]) && (anim = 270 && animelemtime(4)<2)
trigger12 = StateNo = 300 && ((var(3) = [1,2])|| time < 1)
trigger13 = StateNo = 305 && (var(3) = [1,2]) && animelemtime(4)<2
trigger14 = StateNo = 310 && (var(3) = [1,2]) && animelemtime(4)<2
trigger15 = StateNo = 320 && ((var(3) = [1,2])|| time < 1)
trigger16 = StateNo = 325 && (var(3) = [1,2]) && animelemtime(4)<2
trigger17 = StateNo = 330 && (var(3) = [1,2]) && animelemtime(3)<2
trigger18 = StateNo = 1105 && time >= 1
ignorehitpause = 0

;---------------------------------------------------------------------------
;豪波動拳
[State -1]
type = ChangeState
value = 1200
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = !NumProj
triggerall = Command = "hadou1" || Command = "hadou2" || Command = "hadou3"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && ((var(3) = [1,2])|| time < 1)
trigger3 = StateNo = 205 && ((var(3) = [1,2])|| time < 1)
trigger4 = StateNo = 210 && var(3) = [1,2]
trigger4 = (anim = 210 && animelemtime(5)<0)||(anim = 211 && animelemtime(4)<2)
trigger5 = StateNo = 215 && (var(3) = [1,2]) && animelemtime(3)<2
trigger6 = StateNo = 220 && (var(3) = [1,2]) && (anim = 221 && animelemtime(4)<2)
trigger7 = StateNo = 225 && (var(3) = [1,2]) && animelemtime(4)<2
trigger8 = StateNo = 250 && (var(3) = [1,2]) && (anim = 251 && animelemtime(4)<2)
trigger9 = StateNo = 260 && (var(3) = [1,2]) && (anim = 261 && animelemtime(4)<0)
trigger10 = StateNo = 265 && (var(3) = [1,2]) && animelemtime(3)<2
trigger11 = StateNo = 270 && (var(3) = [1,2]) && (anim = 270 && animelemtime(4)<2)
trigger12 = StateNo = 300 && ((var(3) = [1,2])|| time < 1)
trigger13 = StateNo = 305 && (var(3) = [1,2]) && animelemtime(4)<2
trigger14 = StateNo = 310 && (var(3) = [1,2]) && animelemtime(4)<2
trigger15 = StateNo = 320 && ((var(3) = [1,2])|| time < 1)
trigger16 = StateNo = 325 && (var(3) = [1,2]) && animelemtime(4)<2
trigger17 = StateNo = 330 && (var(3) = [1,2]) && animelemtime(3)<2
ignorehitpause = 0

;---------------------------------------------------------------------------
;灼熱波動拳
[State -1]
type = ChangeState
value = 1300
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = !NumProj
triggerall = Command = "shakunetsu1" || Command = "shakunetsu2" || Command = "shakunetsu3"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && ((var(3) = [1,2])|| time < 1)
trigger3 = StateNo = 205 && ((var(3) = [1,2])|| time < 1)
trigger4 = StateNo = 210 && var(3) = [1,2]
trigger4 = (anim = 210 && animelemtime(5)<0)||(anim = 211 && animelemtime(4)<2)
trigger5 = StateNo = 215 && (var(3) = [1,2]) && animelemtime(3)<2
trigger6 = StateNo = 220 && (var(3) = [1,2]) && (anim = 221 && animelemtime(4)<2)
trigger7 = StateNo = 225 && (var(3) = [1,2]) && animelemtime(4)<2
trigger8 = StateNo = 250 && (var(3) = [1,2]) && (anim = 251 && animelemtime(4)<2)
trigger9 = StateNo = 260 && (var(3) = [1,2]) && (anim = 261 && animelemtime(4)<0)
trigger10 = StateNo = 265 && (var(3) = [1,2]) && animelemtime(3)<2
trigger11 = StateNo = 270 && (var(3) = [1,2]) && (anim = 270 && animelemtime(4)<2)
trigger12 = StateNo = 300 && ((var(3) = [1,2])|| time < 1)
trigger13 = StateNo = 305 && (var(3) = [1,2]) && animelemtime(4)<2
trigger14 = StateNo = 310 && (var(3) = [1,2]) && animelemtime(4)<2
trigger15 = StateNo = 320 && ((var(3) = [1,2])|| time < 1)
trigger16 = StateNo = 325 && (var(3) = [1,2]) && animelemtime(4)<2
trigger17 = StateNo = 330 && (var(3) = [1,2]) && animelemtime(3)<2
ignorehitpause = 0

;---------------------------------------------------------------------------
;空中竜巻斬空脚
[State -1]
type = ChangeState
value = ifelse(var(12)=20,1520,1500)
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = Command = "tatsumaki1" || Command = "tatsumaki2" || Command = "tatsumaki3"
triggerall = statetype = A
trigger1 = ctrl
ignorehitpause = 0

;---------------------------------------------------------------------------
;斬空波動拳
[State -1]
type = ChangeState
value = 1400
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = !NumProj
triggerall = Command = "hadou1" || Command = "hadou2" || Command = "hadou3"
triggerall = statetype = A
trigger1 = ctrl
;trigger2 = var(12) != 20
;trigger2 = StateNo = 350 && (var(3) = [1,2]) && movecontact <= 3
;trigger3 = var(12) != 20
;trigger3 = StateNo = 355 && (var(3) = [1,2]) && movecontact <= 3
;trigger4 = var(12) != 20
;trigger4 = StateNo = 360 && (var(3) = [1,2]) && movecontact <= 3
;trigger5 = var(12) != 20
;trigger5 = StateNo = 365 && (var(3) = [1,2]) && movecontact <= 3
;trigger6 = var(12) != 20
;trigger6 = StateNo = 370 && (var(3) = [1,2]) && movecontact <= 3
;trigger7 = var(12) != 20
;trigger7 = StateNo = 375 && (var(3) = [1,2]) && movecontact <= 3
;trigger8 = var(12) != 20
;trigger8 = StateNo = 400 && (var(3) = [1,2]) && movecontact <= 3
;trigger9 = var(12) != 20
;trigger9 = StateNo = 405 && (var(3) = [1,2]) && movecontact <= 3
;trigger10 = var(12) != 20
;trigger10 = StateNo = 410 && (var(3) = [1,2]) && movecontact <= 3
ignorehitpause = 0

;------------------------------------------------------------------------------
;---------------------------------------------------------------------------
;ステップ
[State -1]
type = ChangeState
value = 100
triggerall = !var(59)
triggerall = Var(12) = 20
trigger1 = command = "FF"
trigger1 = statetype = S
trigger1 = ctrl

[State -1]
type = ChangeState
value = 105
triggerall = !var(59)
triggerall = Var(12) = 20
trigger1 = command = "BB"
trigger1 = statetype = S
trigger1 = ctrl

;==============================================================================
; 特殊技
;==============================================================================
;------------------------------------------------------------------------------
;投げ技(EX)
[State -1]
type = ChangeState
value = 900
triggerall = !var(59)
triggerall = var(12) = 20
triggerall = roundstate = 2
triggerall = statetype = S
triggerall = ctrl || (stateno = 100 && AnimElemTime(3) >= 2)
triggerall = command = "holdfwd" || command = "holdback"
trigger1 = Command = "x+y" || Command = "x+z" || Command = "y+z"
trigger2 = Command = "a+b" || Command = "a+c" || Command = "b+c"
;trigger2 = enemynear,hitdefattr = SC, NT

;------------------------------------------------------------------------------
;投げ技
[State -1]
type = ChangeState
value = 901
triggerall = !var(59)
triggerall = var(12) != 20
triggerall = roundstate = 2
triggerall = statetype = S
triggerall = P2MoveType != H
triggerall = P2StateNo != [150,155]
triggerall = (p2statetype = S) || (p2statetype = C)
triggerall = p2bodydist X = [-15,ceil(33*const(size.xscale))]
triggerall = command = "holdfwd" || command = "holdback"
triggerall = ctrl
trigger1 = Command = "y" || Command = "z" || Command = "b" || Command = "c"
trigger2 = (var(11) = [2,3]) || (var(11) = [5,6])

;---------------------------------------------------------------------------
;前方転身
[State -1]
type = ChangeState
value = 90
triggerall = !var(59)
triggerall = Var(12) = 20
triggerall = roundstate = 2
triggerall = command = "zenten1" || command = "zenten2" || command = "zenten3"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && ((var(3) = [1,2])|| time < 1)
trigger3 = StateNo = 205 && ((var(3) = [1,2])|| time < 1)
trigger4 = StateNo = 210 && var(3) = [1,2]
trigger4 = (anim = 210 && animelemtime(5)<0)||(anim = 211 && animelemtime(4)<2)
trigger5 = StateNo = 215 && (var(3) = [1,2]) && animelemtime(3)<2
trigger6 = StateNo = 220 && (var(3) = [1,2]) && (anim = 221 && animelemtime(4)<2)
trigger7 = StateNo = 225 && (var(3) = [1,2]) && animelemtime(4)<2
trigger8 = StateNo = 250 && (var(3) = [1,2]) && (anim = 251 && animelemtime(4)<2)
trigger9 = StateNo = 260 && (var(3) = [1,2]) && (anim = 261 && animelemtime(4)<0)
trigger10 = StateNo = 265 && (var(3) = [1,2]) && animelemtime(3)<2
trigger11 = StateNo = 270 && (var(3) = [1,2]) && (anim = 270 && animelemtime(4)<2)
trigger12 = StateNo = 300 && ((var(3) = [1,2])|| time < 1)
trigger13 = StateNo = 305 && (var(3) = [1,2]) && animelemtime(4)<2
trigger14 = StateNo = 310 && (var(3) = [1,2]) && animelemtime(4)<2
trigger15 = StateNo = 320 && ((var(3) = [1,2])|| time < 1)
trigger16 = StateNo = 325 && (var(3) = [1,2]) && animelemtime(4)<2
trigger17 = StateNo = 330 && (var(3) = [1,2]) && animelemtime(3)<2
ignorehitpause = 0

;---------------------------------------------------------------------------
;回り込み
[State -1]
type = ChangeState
value = 95
triggerall = !var(59)
triggerall = Var(12) = 20
triggerall = roundstate = 2
triggerall = command = "holdfwd" && command = "recovery"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && time < 2
trigger4 = StateNo = 210 && time < 2
trigger6 = StateNo = 220 && time < 2
trigger8 = StateNo = 250 && time < 2
trigger9 = StateNo = 260 && time < 2
trigger11 = StateNo = 270 && time < 2
trigger12 = StateNo = 300 && time < 2
trigger13 = StateNo = 305 && time < 2
trigger14 = StateNo = 310 && time < 2
trigger15 = StateNo = 320 && time < 2
trigger16 = StateNo = 325 && time < 2
trigger17 = StateNo = 330 && time < 2
ignorehitpause = 0

;---------------------------------------------------------------------------
;頭蓋破殺
[State -1]
type = null;ChangeState
value = 460
triggerall = !var(59)
triggerall = Var(12) = 20
triggerall = roundstate = 2
triggerall = command = "y"
triggerall = command != "holddown" && Command = "holdfwd"
triggerall = statetype != A
trigger1 = ctrl
ignorehitpause = 0

;---------------------------------------------------------------------------
;旋風脚
[State -1]
type = ChangeState
value = 470
triggerall = !var(59)
triggerall = Var(12) = 20
triggerall = roundstate = 2
triggerall = command != "holddown" && Command = "holdfwd"
triggerall = statetype != A
triggerall = ctrl
trigger1 = command = "b"
trigger2 = var(11) = 2
ignorehitpause = 0

;---------------------------------------------------------------------------
;天魔空刃脚
[State -1]
type = ChangeState
value = 450
triggerall = !var(59)
triggerall = Var(12) = 20
triggerall = roundstate = 2
triggerall = command = "b"
triggerall = command = "holddown"
triggerall = statetype = A
triggerall = Pos Y <= -80
triggerall = Vel Y= [-2.5,2.5]
triggerall = Vel X > 0
trigger1 = ctrl
ignorehitpause = 0

;---------------------------------------------------------------------------
;GCステップ
[State -1]
type = ChangeState
value = 620
triggerall = !var(59)
triggerall = Var(12) = 20
triggerall = roundstate = 2
triggerall = Power >= 1000
triggerall = statetype != A
triggerall = Command = "FF"
trigger1 = stateno = 150
trigger2 = stateno = 151
trigger3 = stateno = 152
trigger4 = stateno = 153

;---------------------------------------------------------------------------
;ZEROカウンター
[State -1]
type = ChangeState
value = ifelse(statetype=C,635,630)
triggerall = !var(59)
triggerall = Var(12) = 20
triggerall = roundstate = 2
triggerall = Power >= 1000
triggerall = statetype != A
triggerall = Command = "z+c"
trigger1 = stateno = 150
trigger2 = stateno = 151 && time < 10
trigger3 = stateno = 152
trigger4 = stateno = 153 && time < 10
ignorehitpause = 1

;---------------------------------------------------------------------------
;パワー溜め
[State -1]
type = ChangeState
value = 610
triggerall = !var(59)
triggerall = Var(12) = 20
triggerall = roundstate = 2
triggerall = power < 3000
triggerall = (command = "chargec" && command = "chargez")
triggerall = statetype != A
trigger1 = ctrl
trigger2 = StateNo = 200 && time < 2
trigger3 = StateNo = 201 && time < 2
trigger4 = StateNo = 210 && time < 2
trigger5 = StateNo = 211 && time < 2
trigger6 = StateNo = 220 && time < 2
trigger7 = StateNo = 221 && time < 2
trigger8 = StateNo = 230 && time < 2
trigger9 = StateNo = 240 && time < 2
trigger10 = StateNo = 241 && time < 2
trigger11 = StateNo = 250 && time < 2
trigger12 = StateNo = 251 && time < 2
trigger13 = StateNo = 400 && time < 2
trigger14 = StateNo = 410 && time < 2
trigger15 = StateNo = 420 && time < 2
trigger16 = StateNo = 430 && time < 2
trigger17 = StateNo = 440 && time < 2
trigger18 = StateNo = 450 && time < 2
trigger19 = StateNo = 300 && time < 2

;---------------------------------------------------------------------------
;受身
[State -1]
type = ChangeState
value = 5201
persistent = 0
triggerall = !var(59)
triggerall = Var(12) = 20
triggerall = Alive
triggerall = StateNo = 5050 || StateNo = 5071
triggerall = Vel Y > 0
triggerall = Pos Y >= -5
trigger1 = command = "recovery"

;==============================================================================
; 通常攻撃技
;==============================================================================
;------------------------------------------------------------------------------

[State -1, 近距離立ち弱パンチ]
type = ChangeState
value = 205
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = var(12) != 20
triggerall = statetype != A
triggerall = command != "holddown"
triggerall = P2BodyDist X = [ceil(-28*const(size.xscale)),ceil(28*const(size.xscale))]
triggerall = ctrl
trigger1 = command = "x"
trigger1 = statetype = S
trigger2 = var(11) = 4
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 遠距離立ち弱パンチ]
type = ChangeState
value = 200
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command != "holddown"
triggerall = statetype != A
trigger1 = ctrl
trigger1 = command = "x"
trigger1 = statetype = S
trigger2 = command = "x"
trigger2 = stateno = 200
trigger2 = time > 4
;(var(3) && animelemtime(3) >= 0) || (!var(3) && animelemtime(2) > 1)
trigger3 = var(11) = 4
trigger3 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 近距離立ち中パンチ]
type = ChangeState
value = 215
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = var(12) != 20
triggerall = command != "holddown"
triggerall = statetype != A
triggerall = P2BodyDist X = [ceil(-31*const(size.xscale)),ceil(31*const(size.xscale))]
triggerall = ctrl
trigger1 = command = "y"
trigger1 = statetype = S
trigger2 = var(11) = 5
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 遠距離立ち中パンチ]
type = ChangeState
value = 210
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command != "holddown"
triggerall = statetype != A
triggerall = ctrl
trigger1 = command = "y"
trigger1 = statetype = S
trigger2 = var(11) = 5
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 近距離立ち強パンチ]
type = ChangeState
value = 225
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = var(12) != 20
triggerall = command != "holddown"
triggerall = statetype != A
triggerall = P2BodyDist X = [ceil(-33*const(size.xscale)),ceil(33*const(size.xscale))]
triggerall = ctrl
trigger1 = command = "z"
trigger1 = statetype = S
trigger2 = var(11) = 6
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 遠距離立ち強パンチ]
type = ChangeState
value = 220
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command != "holddown"
triggerall = statetype != A
triggerall = ctrl
trigger1 = command = "z"
trigger1 = statetype = S
trigger2 = var(11) = 6
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 近距離立ち弱キック]
type = ChangeState
value = 255
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = var(12) != 20
triggerall = command != "holddown"
triggerall = statetype != A
triggerall = P2BodyDist X = [ceil(-13*const(size.xscale)),ceil(13*const(size.xscale))]
trigger1 = ctrl
trigger1 = command = "a"
trigger1 = statetype = S
trigger2 = command = "a"
trigger2 = stateno = 255
trigger2 = time > 4
trigger3 = var(11) = 1
trigger3 = ctrl
;(var(3) && animelemtime(3) >= 0) || (!var(3) && animelemtime(2) > 1)
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 遠距離立ち弱キック]
type = ChangeState
value = 250
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command != "holddown"
triggerall = statetype != A
triggerall = ctrl
trigger1 = command = "a"
trigger1 = statetype = S
trigger2 = var(11) = 1
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 近距離立ち中キック]
type = ChangeState
value = 265
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = var(12) != 20
triggerall = command != "holddown"
triggerall = statetype != A
triggerall = P2BodyDist X = [ceil(-17*const(size.xscale)),ceil(17*const(size.xscale))]
triggerall = ctrl
trigger1 = command = "b"
trigger1 = statetype = S
trigger2 = var(11) = 2
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 遠距離立ち中キック]
type = ChangeState
value = 260
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command != "holddown"
triggerall = statetype != A
triggerall = ctrl
trigger1 = command = "b"
trigger1 = statetype = S
trigger2 = var(11) = 2
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 近距離立ち強キック]
type = ChangeState
value = 275
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = var(12) != 20
triggerall = command != "holddown"
triggerall = statetype != A
triggerall = P2BodyDist X = [ceil(-33*const(size.xscale)),ceil(33*const(size.xscale))]
triggerall = ctrl
trigger1 = command = "c"
trigger1 = statetype = S
trigger2 = var(11) = 3
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 遠距離立ち強キック]
type = ChangeState
value = 270
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command != "holddown"
triggerall = statetype != A
triggerall = ctrl
trigger1 = command = "c"
trigger1 = statetype = S
trigger2 = var(11) = 3
ignorehitpause = 0

;------------------------------------------------------------------------------

[State -1, 挑発]
type = null ;ChangeState
value = 195
triggerall = command = "start"
trigger1 = statetype != A
trigger1 = ctrl

;------------------------------------------------------------------------------
[State -1, しゃがみ弱パンチ]
type = ChangeState
value = 300
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "holddown"
triggerall = statetype != A
trigger1 = command = "x"
trigger1 = statetype = C
trigger1 = ctrl
trigger2 = var(11) = 4 || command = "x"
trigger2 = stateno = 300
trigger2 = animelemtime(3) >= 0;(var(3) && animelemtime(3) >= 0) || (!var(3) && animelemtime(2) > 1)
trigger3 = command = "x"
trigger3 = stateno = 320
trigger3 = time > 4;(var(3) && animelemtime(3) >= 0) || (!var(3) && animelemtime(2) > 1)
trigger4 = var(11) = 4
trigger4 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, しゃがみ中パンチ]
type = ChangeState
value = 305
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "holddown"
triggerall = ctrl
triggerall = statetype != A
trigger1 = command = "y"
trigger1 = statetype = C
trigger2 = var(11) = 5
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, しゃがみ強パンチ]
type = ChangeState
value = 310
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "holddown"
triggerall = ctrl
triggerall = statetype != A
trigger1 = command = "z"
trigger1 = statetype = C
trigger2 = var(11) = 6
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, しゃがみ弱キック]
type = ChangeState
value = 320
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "holddown"
triggerall = statetype != A
trigger1 = command = "a"
trigger1 = statetype = C
trigger1 = ctrl
trigger2 = command = "a"
trigger2 = stateno = 300
trigger2 = animelemtime(3) >= 0;(var(3) && animelemtime(3) >= 0) || (!var(3) && animelemtime(2) > 1)
trigger3 = var(11) = 1 || command = "a"
trigger3 = stateno = 320
trigger3 = time > ifelse(var(12)=20,5,4)
;(var(3) && animelemtime(3) >= 0) || (!var(3) && animelemtime(2) > 1)
trigger4 = var(11) = 1
trigger4 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, しゃがみ中キック]
type = ChangeState
value = 325
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "holddown"
triggerall = ctrl
triggerall = statetype != A
trigger1 = statetype = C
trigger1 = command = "b"
trigger2 = var(11) = 2
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, しゃがみ強キック]
type = ChangeState
value = 330
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "holddown"
triggerall = ctrl
triggerall = statetype != A
trigger1 = statetype = C
trigger1 = command = "c"
trigger2 = var(11) = 3
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, ジャンプ弱パンチ]
type = ChangeState
value = 350
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "x"
trigger1 = statetype = A
trigger1 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, ジャンプ中パンチ]
type = ChangeState
value = 355
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "y"
trigger1 = statetype = A
trigger1 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, ジャンプ強パンチ]
type = ChangeState
value = 360
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "z"
trigger1 = statetype = A
trigger1 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 斜めジャンプ弱キック]
type = ChangeState
value = 400
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "a"
triggerall = Vel X
trigger1 = statetype = A
trigger1 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 垂直ジャンプ弱キック]
type = ChangeState
value = 365
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "a"
trigger1 = statetype = A
trigger1 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 斜めジャンプ中キック]
type = ChangeState
value = 405
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "b"
triggerall = Vel X
trigger1 = statetype = A
trigger1 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 垂直ジャンプ中キック]
type = ChangeState
value = 370
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "b"
trigger1 = statetype = A
trigger1 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, 斜めジャンプ強キック]
type = ChangeState
value = 410
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "c"
triggerall = Vel X
trigger1 = statetype = A
trigger1 = ctrl
ignorehitpause = 0

;------------------------------------------------------------------------------
[State -1, ジャンプ強キック]
type = ChangeState
value = 375
triggerall = !var(59)
triggerall = roundstate = 2
triggerall = command = "c"
trigger1 = statetype = A
trigger1 = ctrl
ignorehitpause = 0
