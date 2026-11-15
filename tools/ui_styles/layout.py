"""KAI's own layout, measured from the game's screens (tools/test/scenarios/uicap_*: each box's pixels where the
interface differs from the same scene with it hidden). Every style draws into these same rectangles, so a style
changes how the game looks and never where anything is (the user, 2026-11-15: "fitment is critical", above all in
Double and Triple Battles). Rectangles are (x0, y0, x1, y1), x1 / y1 exclusive."""

# ---- battles
SINGLE_FOE = (13, 16, 113, 44)          # name, gender, Lv / HP bar
SINGLE_MINE = (127, 74, 230, 110)       # + HP numbers, EXP
DOUBLE_FOES = [(13, 5, 113, 29), (1, 30, 101, 54)]
DOUBLE_MINE = [(128, 62, 228, 86), (139, 87, 239, 111)]
TRIPLE_FOES = [(1, 1, 78, 21), (1, 22, 78, 42), (1, 43, 78, 63)]
TRIPLE_MINE = [(164, 48, 239, 68), (164, 69, 239, 89), (164, 90, 239, 110)]
TRIPLE_FOE_ICONS = [(80, 7), (80, 28), (80, 49)]        # KAI's type icons beside the cards (12 x 12), kept as they are
TRIPLE_MINE_ICONS = [(150, 54), (150, 75), (150, 96)]
BATTLE_MSG = (0, 112, 120, 160)         # "What will / TORCHIC do?"
BATTLE_MENU = (120, 112, 240, 160)      # FIGHT BAG / MOéMON RUN
MOVES = (0, 112, 160, 160)              # the four moves
MOVE_INFO = (160, 112, 240, 160)        # PP, TYPE/

# ---- the field
FIELD_MSG = (0, 112, 240, 160)
START = (160, 2, 238, 142)              # seven rows 16 apart from y 18

# ---- the party
PARTY_MAIN = (10, 26, 86, 74)           # the first POKéMON, picked
PARTY_SLOTS = [(88, 10 + 24 * i, 238, 34 + 24 * i) for i in range(5)]
PARTY_MSG = (2, 130, 182, 158)
PARTY_CANCEL = (184, 134, 238, 156)

# ---- the summary (INFO page)
SUM_HEADER = (0, 0, 240, 16)
SUM_LEFT = (0, 16, 80, 160)             # number, picture, name, ball, Lv
SUM_RIGHT = (80, 16, 240, 160)          # PROFILE, ABILITY, TRAINER MEMO

# ---- the BAG (KAI's eight pockets)
BAG_TITLE = (2, 2, 56, 18)
BAG_PIC = (44, 10, 100, 70)
BAG_POCKETS = (0, 72, 102, 86)
BAG_POCKET_NAME = (2, 88, 98, 106)
BAG_LIST = (104, 4, 238, 104)
BAG_DESC = (0, 108, 240, 160)
BAG_ICON = (4, 118, 34, 150)

# ---- OPTIONS (tabs)
OPT_TABS = (2, 2, 238, 30)
OPT_BODY = (2, 34, 238, 158)
OPT_ROW0, OPT_ROW_STEP = 46, 14
