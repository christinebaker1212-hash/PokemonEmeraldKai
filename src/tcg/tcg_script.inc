/* The regional sets' attack scripts (docs/TCG_REGIONS.md; included by tcg_effects.c for its helpers).
 *
 * An effect row of kind FX_SCRIPT carries a script (tools/tcg/regions/effects.py writes them from the cards'
 * words): ops separated by spaces, each "[cond.]name[:a[,b[,c]]]". Coins and damage are reckoned before the damage
 * (phase 0), the rest after it (phase 1), a "drag" before the attack's protection is settled (phase 3); the coins
 * flipped are kept for the later ops (TcgDuel.scriptCoins / scriptHeads). Types: TCG_* (0-8), 15 any.
 *
 * Coins (0):  c[:n] n coins (1) | cu until tails | ce:T[,max] one per Energy of type T on it | cdc / csc one per
 *             damage counter on the Defending Pokemon / on it
 * Damage (0): add:x | addh:x x per heads | mulh:x x times the heads | set:x base damage x | none (nothing at all) |
 *             zero (no damage, the rest happens) | addsc:x / adddc:x x per counter on it / on the Defending |
 *             adde:T,x[,max] x per Energy of type T on it | addde:x x per Energy card on the Defending |
 *             addpast:T,x,max x per Energy of type T past the cost (max more) | dse:T,n discard n of its Energy
 *             cards of type T (0 all) to use it
 * After (1):  par / slp / cnf / psn / psn20 / brn the Defending Pokemon Paralyzed, Asleep, Confused, Poisoned
 *             (20 a turn), Burned | spar / sslp / scnf / spsn on itself | rec:x x to itself | ctr:n n damage counters on it |
 *             heal:n n counters off it | healall:n off each of yours | bo:x / bm:x / bb:x x to each of their
 *             Benched / yours / both | b1:x x to the chosen one of their Benched (target) | any1:x x to the chosen one
 *             of theirs (target) | osw they switch | lure their chosen Benched one in (target) | ssw it switches with
 *             the chosen one of yours (target) | bar:n prevent damage (1) / all effects (2) next turn | guard:n
 *             TCG_GUARD_n | nort / noat the Defending can't retreat / attack next turn | lock this attack not
 *             next turn | selflock it can't attack next turn | dde one of the Defending's Energy discarded (picked)
 *             | dsa:T,n n of its Energy cards of type T discarded (0 all; as many as it has) | attd:T,n up to n Energy of type T from the discard pile onto it | srch:T,n n basic Energy of type T
 *             from the deck onto it | draw:n | notr no Trainers for them next turn
 *             | vuln:x attacks do x more to the Defending until the end of your next turn | srchp:T a basic Energy (of
 *             type T) picked from the deck onto it | nores / noweak / nowr no Resistance / Weakness / either (0) |
 *             dsx:T,n as dse, else the attack does nothing (0)
 * Conditions: h heads (some) | t tails (some) | ah all heads | at all tails | geN N heads or more | dpsn / dbrn /
 *             ddmg / dnodmg the Defending Pokemon Poisoned / Burned / damaged / not damaged | eqe / neqe it and
 *             the Defending have (not) the same number of Energy cards | oN the attack's option N was chosen
 * Options (0): opt:K[,T] a menu before the attack (ScriptOptions): K 0 NO / YES (YES only with Energy of type T on
 *             it, if T is given), 1 / 2 a Special Condition (5: with Burned / 4), 3 YOUR DECK / THEIR DECK, 5 NO /
 *             FIRE / FIGHTING (a basic Energy of that type on it), 6 how many Energy of type T on it (0..)
 *             (7 / 8: Asleep, Confused and Poisoned / Paralyzed) | cop a coin for each of that many | addop:x x for each of that many | addbd:x,d x times the Benched Pokemon theirs minus yours (d 1:
 *             yours minus theirs, none below 0) | addrc:x x per Colorless in the Defending's Retreat Cost | addpk:T,x
 *             x more if the Energy picked is of type T | milladdb:x,T,z both players mill x, z per Energy of type T
 * After (1):  chosc:K the Special Condition chosen (opt:1 / 2) | dso:T discard the chosen number of type T | order3
 *             the top 3 of the chosen deck in the order picked | b1e:T,x x per Energy of type T on it to the chosen
 *             Benched Pokemon | ectr counters on the chosen one of theirs, one per Energy card on it | botd:x x to
 *             each of their Benched of the Defending's type | luremay their chosen Benched (if any) switched in
 *             unless the Defending is Knocked Out | bbc:x a coin for each Benched Pokemon (both): x to it on heads
 *             | dte the picked Energy of the chosen one of theirs discarded | octrd a counter on each of theirs that has
 *             any | mvctr a counter from each of yours that has any onto the Defending | healt every counter off the
 *             chosen one of yours | bpick:x x to each of their Benched picked (none picked: the least HP left first,
 *             2) | m1:x x to the chosen one of your Benched | mulname:x x times your Pokemon with its name (set) |
 *             disdeck:T the Energy of type T in your discard pile shuffled into your deck | distop:w the card picked
 *             from your (0) / their (1) discard pile onto that deck | otop their top card: a Trainer discarded, else
 *             into their hand | swapdmg its and the Defending's counters swapped
 * Damage (0): adddis:T,x x per Energy of type T in your discard pile
 * Conditions: shurt an attack damaged it last turn | ddark the Defending has Dark in its name or is Darkness |
 *             dslpcnf it's Asleep or Confused | dnslp it isn't Asleep | dmoree it has more Energy than this one |
 *             devo it's an Evolved Pokemon
 * Coins (0):  cde one per Energy card on the Defending | coe one per Energy card on their Pokemon | cmbe one per
 *             Energy card on your Benched Pokemon
 * Damage (0): mulmb:x x times your Benched Pokemon (set) | addtypes:x x per type of basic Energy on it | dsxh:T
 *             discard as many Energy of type T as heads, else the attack does nothing
 * After (1):  allc:x a coin for each Pokemon in play (both, it too): x to it on heads | any1e:T,x,max x per Energy
 *             of type T on it (max) to the chosen one of theirs | dcure the Defending no longer Asleep or Confused |
 *             srchb:T a basic Energy of type T from the deck onto the chosen one of your Benched | healbb:n n off each
 *             Benched Pokemon (both) | healbe:T a counter per Energy of type T on it off the chosen one of your
 *             Benched | omill:x they discard their top x (0: one per heads) | recmb:x x to itself per Benched
 *             Pokemon of yours | healt:n (n counters, 0 all) | tailwind your Active's Retreat Cost 0 this turn | hdis:T
 *             the basic Energy of type T picked in the hand discarded | dcn:n n counters on the Defending | any1c:n n
 *             counters on the chosen one of theirs | dcself the Defending gets as many counters as it has | dcmine one
 *             counter on the Defending per damaged Pokemon of yours | ctrh:n n counters on it per heads | healen:n n
 *             off it per Energy card on it | healtyp:n n off it and each of yours of its type | bshield your Bench
 *             takes no damage from attacks until their next turn ends | spike the basic Energy picked from the deck
 *             onto the one of yours chosen
 * Coins (0):  cpk one per Pokemon in play (both) | Damage (0): mulpk:x x times your Pokemon in play (set) |
 *             addfam:x x per Benched Pokemon of yours of its evolution family
 * Before (3): dragmay[:1] their chosen Benched one in, if one was chosen (1: only one with no damage counters)
 * After (1):  weak:n the Defending's attacks on it do 10 / 20 less next turn (TCG_WEAKEN_n) | srchk:K the card picked
 *             from the deck (K 0 a Supporter, 1 a Technical Machine or Pokemon Tool, 2 a Trainer) into the hand |
 *             dbench:K the Pokemon picked from the deck (K 0 Baby or Basic, 1 Baby) onto the Bench | disbench the Baby
 *             or Basic picked from the discard pile onto the Bench | dishand the Pokemon or basic Energy picked from
 *             the discard pile into the hand | hatt:K,n,t up to n Energy picked from the hand (K 0 any, 1 basic, 2
 *             Grass) onto it (t 0) / the one of yours chosen (t 1: Grass Pokemon when K is 2) | any1:x,1 (only a
 *             damaged one)
 * Name groups (TcgFlow_InNameGroup): Coins (0) cnb:k / cnp:k one per Benched Pokemon of yours / Pokemon of yours in
 *             play in group k | Damage (0): addnb:x,k,max x per Benched one in group k (max, 0 any) | Before the damage
 *             (0): dbsame the Basics named as it picked from the deck onto the Bench | After (1): nsrch:k,max the cards
 *             in group k picked from the deck into the hand | dbown:o the Basic with owner o's name (TCG_OWNER_*) picked
 *             from the deck onto the Bench
 * After (1):  drain:k counters off it for the damage done (1 all, 2 half rounded up) | ocoinh:x a coin for each of
 *             theirs, x to it on heads, that much off it | psn30 | dte:1 / dte:2 a Special Energy / a Trainer of the
 *             chosen one of theirs into their deck | ohand their Active and its cards to their hand | ohbench the
 *             Basic picked from their hand onto their Bench, switched in | eto:16 any Energy card
 * Conditions: dlt the attack damaged the Defending Pokemon | ob / mb they / you have Benched Pokemon | enN it has
 *             Energy of type N | dfew the Defending has fewer Energy cards than it | oevo they have an Evolved
 *             Pokemon
 * Options (0): opt:9,T how many Energy cards of type T on all your Pokemon | dfld:T[,1] that many picked from your
 *             Pokemon in play discarded (1: a coin for each) | After (1): noconds:1 no Special Conditions as long as
 *             it's the Active Pokemon | odevb:1 into their deck, shuffled | odisbench the Basic picked from their discard
 *             pile onto their Bench with a damage counter | guard:9 30 or more prevented
 * After (1):  anchor:k the Defending Pokemon can't retreat and isn't switched out (TcgDuel_Anchored): through their
 *             next turn (0), while it's your Active (1), and through your next turn (2) | koslp:i it's Knocked Out if
 *             Asleep and it used attack i last turn | dheal:n n counters off the Defending (0 all) | oetype:x x to each
 *             of theirs with Energy of the type chosen (opt:10) | rec doubled by boost:i,0 on that attack
 * Blocks:     needlast:i it used attack i last turn (TcgFieldCard.lastAttack) | needhand:n n cards in the hand at most
 * After (1):  cure4:w your Active (0) / the Defending (1) no longer Asleep, Confused, Paralyzed or Poisoned | drawto:n
 *             draw until n in the hand | shiftto:T its type T for the turn | srchk:3 a Pokemon card | dbench:2 a Metal
 *             Basic | react:k[,n] when an attack damages it through their next turn (n 2: your next too): TCG_REACT_k
 *             (TcgPower_Damaged) | pursuit / attpsn the Defending: 10 if it retreats / Poisoned when Energy is attached
 *             to it, through their turn
 * Conditions: hN / tN the Nth coin came up heads / tails | Options (0): opt:11 the Defending Pokemon's attacks if it's
 *             a Basic Pokemon (else no menu) | After (1): amn that attack not next turn (Terrorize) | guard:10 Reflect
 *             Shield | guardk:K TCG_GUARD_K kept until it takes damage, is Benched or evolves (Shadow Images: 11)
 * Powers (1): pull the Grass Energy picked (up to 2) from your other Pokemon onto it | emove:4 between Magnemite /
 *             Magneton / Dark Magneton (name group 6) | ctrin a counter from the Unown chosen onto it | spike:T,K (an
 *             Energy of type T; K 1 onto an Unown, 2 onto a Benched one) | mshift Miracle Shift | recon Reconstruction |
 *             nobody every Poke-Body ignored this turn | order:n the top n of your deck in the order picked | oattd
 *             the Energy picked in their discard pile onto the Defending Pokemon | ohshuf Dark Moon
 * After (1):  boh:x x per heads to each of their Benched | oswitch they switch their Active with a Benched one of their
 *             choice (TcgDuel_OpponentSwitch) | gmove:T Baton Pass | mime Sleight of Hand | srchk:4 a Pokemon card not
 *             named as it | ddeh:n one of the Defending's Energy per heads, up to n picked | lockon Lock-on */
#define SCRIPT_TOKEN 48
/* Which of the player's Pokemon "it" is: the attacker (0), or a Pokemon whose power runs a script (tcg_powers.c) */
static unsigned sScriptSelf;
/* The attack's option (TcgAction.option) while a script runs */
static unsigned sScriptOption;
/* The damage the attack did to the Defending Pokemon (after Weakness and Resistance) while its after ops run */
static unsigned sScriptDealt;
struct ScriptOp
{
    char cond[8], name[12];
    int arg[3];
    unsigned args;
};
/* The next op from *s (0: none left) */
/* (a bounded copy: longer words are cut, as no op or condition is that long) */
static void ScriptCopy(char *to, size_t size, const char *from)
{
    size_t n = 0;
    while (from[n] && n + 1 < size)
        to[n] = from[n], n++;
    to[n] = 0;
}
static int ScriptNext(const char **s, struct ScriptOp *op)
{
    char tok[SCRIPT_TOKEN], *name, *colon, *dot;
    unsigned n = 0;
    while (**s == ' ')
        (*s)++;
    if (!**s)
        return 0;
    while (**s && **s != ' ' && n < sizeof(tok) - 1)
        tok[n++] = *(*s)++;
    tok[n] = 0;
    memset(op, 0, sizeof(*op));
    name = tok;
    if ((dot = strchr(tok, '.')) != NULL)
    {
        *dot = 0;
        ScriptCopy(op->cond, sizeof(op->cond), tok);
        name = dot + 1;
    }
    if ((colon = strchr(name, ':')) != NULL)
    {
        char *p = colon + 1;
        *colon = 0;
        while (*p && op->args < 3)
        {
            op->arg[op->args++] = (int)strtol(p, &p, 10);
            if (*p == ',')
                p++;
            else
                break;
        }
    }
    ScriptCopy(op->name, sizeof(op->name), name);
    return 1;
}
static int ScriptIs(const struct ScriptOp *op, const char *name) { return !strcmp(op->name, name); }
/* An op's phase: 0 coins and damage, 1 after the damage, 3 before the attack */
static int ScriptPhase(const struct ScriptOp *op)
{
    static const char *const before[] = {"c", "cu", "ce", "cdc", "csc", "add", "addh", "mulh", "set", "none", "zero",
                                         "addsc", "adddc", "adde", "addde", "addpast", "dse", "dsx", "nores",
                                         "noweak", "nowr", "halfhp", "subsc", "dbl", "cmine", "addmd", "milladd",
                                         "adds", "addoh", "eret", "dtrdisc", "opt", "cop", "addbd", "addrc",
                                         "addpk", "milladdb", "addop", "adddis",
                                         "mulname", "cde", "coe", "cmbe", "mulmb", "addtypes", "dsxh", "cpk",
                                         "mulpk", "addfam", "cnb", "cnp", "addnb", "dbsame", "dfld"};
    unsigned i;
    if (ScriptIs(op, "drag") || ScriptIs(op, "dragmay"))
        return 3;
    for (i = 0; i < sizeof(before) / sizeof(before[0]); i++)
        if (ScriptIs(op, before[i]))
            return 0;
    return 1;
}
static int ScriptCond(const struct TcgDuel *g, const char *cond)
{
    const struct TcgFieldCard *foe = &g->players[g->current ^ 1].field[0];
    unsigned heads = g->scriptHeads, tails = g->scriptCoins - g->scriptHeads;
    if (!cond[0])
        return 1;
    if (!strcmp(cond, "h"))
        return heads > 0;
    if (!strcmp(cond, "t"))
        return tails > 0;
    if (!strcmp(cond, "ah"))
        return g->scriptCoins && !tails;
    if ((cond[0] == 'h' || cond[0] == 't') && cond[1] >= '1' && cond[1] <= '9' && !cond[2])
    {
        /* the Nth coin (h2: the second came up heads, t2: it came up tails) */
        unsigned n = (unsigned)(cond[1] - '1');
        return n < g->scriptCoins && ((g->scriptBits >> n & 1) != 0) == (cond[0] == 'h');
    }
    if (!strcmp(cond, "at"))
        return !heads;
    if (!strncmp(cond, "ge", 2))
        return heads >= (unsigned)atoi(cond + 2);
    if (!strncmp(cond, "eq", 2) && cond[2] >= '0' && cond[2] <= '9')
        return heads == (unsigned)atoi(cond + 2);
    if (!strcmp(cond, "ob") || !strcmp(cond, "mb"))
    {
        /* they (ob) / you (mb) have Benched Pokemon */
        unsigned who = g->current ^ (cond[0] == 'o'), k;
        for (k = 1; k < TCG_FIELD_SIZE; k++)
            if (g->players[who].field[k].stackSize)
                return 1;
        return 0;
    }
    if (cond[0] == 'e' && cond[1] == 'n' && cond[2] >= '0' && cond[2] <= '9')
        return EnergyOfType(g, g->current, sScriptSelf, (unsigned)atoi(cond + 2)) > 0; /* it has Energy of type N */
    if (!strcmp(cond, "dlt"))
        return sScriptDealt > 0; /* (the attack damaged the Defending Pokemon) */
    if (!strcmp(cond, "nah"))
        return tails > 0; /* (not all heads) */
    if (cond[0] == 'o' && cond[1] >= '0' && cond[1] <= '9')
        return sScriptOption == (unsigned)atoi(cond + 1);
    /* pg5: their Prizes 5 or more; pzN: exactly N; eXY: it has basic Energy cards of types X and Y */
    if (!strcmp(cond, "dhp50"))
    {
        const struct TcgCard *d = TcgDuel_Field(g, g->current ^ 1, 0);
        return d && d->kind == TCG_POKEMON && TcgDuel_MaxHp(g, g->current ^ 1, 0) <= 50;
    }
    if (!strcmp(cond, "pg5"))
        return g->players[g->current ^ 1].prizeSize >= 5;
    if (cond[0] == 'p' && cond[1] == 'z')
        return g->players[g->current ^ 1].prizeSize == (unsigned)atoi(cond + 2);
    if (cond[0] == 'e' && cond[1] >= '0' && cond[1] <= '8' && cond[2] >= '0' && cond[2] <= '8' && !cond[3])
    {
        const struct TcgPlayer *p = &g->players[g->current];
        const struct TcgFieldCard *f = &p->field[sScriptSelf];
        unsigned k, have = 0;
        for (k = 0; k < f->attachedSize; k++)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[f->attached[k]]);
            if (c && c->kind == TCG_ENERGY && c->id <= 6)
                have |= 1u << c->type;
        }
        return (have >> (cond[1] - '0') & 1) && (have >> (cond[2] - '0') & 1);
    }
    if (!strcmp(cond, "ddark"))
    {
        const struct TcgCard *d = TcgDuel_Field(g, g->current ^ 1, 0);
        return d && (strstr(d->name, "Dark ") != NULL || TcgDuel_Type(g, g->current ^ 1, 0) == TCG_DARKNESS);
    }
    if (!strcmp(cond, "dslpcnf"))
        return foe->stackSize && (foe->condition == TCG_ASLEEP || foe->condition == TCG_CONFUSED);
    if (!strcmp(cond, "dnslp"))
        return !foe->stackSize || foe->condition != TCG_ASLEEP;
    if (!strcmp(cond, "oevo"))
    {
        /* they have an Evolved Pokemon in play */
        unsigned k;
        for (k = 0; k < TCG_FIELD_SIZE; k++)
            if (g->players[g->current ^ 1].field[k].stackSize >= 2)
                return 1;
        return 0;
    }
    if (!strcmp(cond, "dfew"))
        return EnergyCards(g, g->current ^ 1, 0) < EnergyCards(g, g->current, sScriptSelf);
    if (!strcmp(cond, "dmoree"))
        return EnergyCards(g, g->current ^ 1, 0) > EnergyCards(g, g->current, sScriptSelf);
    if (!strcmp(cond, "devo"))
        return foe->stackSize >= 2;
    if (!strcmp(cond, "shurt"))
        return g->players[g->current].field[sScriptSelf].mirrorDamage != 0;
    if (!strcmp(cond, "spsn"))
        return g->players[g->current].field[sScriptSelf].poison != 0;
    if (!strcmp(cond, "ob"))
    {
        unsigned s;
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            if (g->players[g->current ^ 1].field[s].stackSize)
                return 1;
        return 0;
    }
    if (!strcmp(cond, "dbaby") || !strcmp(cond, "dbasic"))
    {
        const struct TcgCard *d = TcgDuel_Field(g, g->current ^ 1, 0);
        return d && d->kind == TCG_POKEMON && (cond[1] == 'b' && cond[2] == 'a' && cond[3] == 'b' ? (d->flags & TCG_CF_BABY) != 0
                                                                                                  : !d->stage);
    }
    if (!strcmp(cond, "nat"))
        return heads > 0;
    if (!strncmp(cond, "dt", 2) && cond[2] >= '0' && cond[2] <= '9')
        return foe->stackSize && TcgDuel_Type(g, g->current ^ 1, 0) == (unsigned)atoi(cond + 2);
    if (!strncmp(cond, "sc", 2) && cond[2] >= '0' && cond[2] <= '9')
        return g->players[g->current].field[sScriptSelf].damage / 10 >= (unsigned)atoi(cond + 2);
    if (!strcmp(cond, "dpsn"))
        return foe->poison != 0;
    if (!strcmp(cond, "dbrn"))
        return foe->burned != 0;
    if (!strcmp(cond, "ddmg"))
        return foe->damage != 0;
    if (!strcmp(cond, "dnodmg"))
        return foe->damage == 0;
    if (!strcmp(cond, "eqe") || !strcmp(cond, "neqe"))
    {
        int same = EnergyCards(g, g->current, 0) == EnergyCards(g, g->current ^ 1, 0);
        return cond[0] == 'e' ? same : !same;
    }
    return 0;
}
static void ScriptFlip(struct TcgDuel *g, unsigned n)
{
    unsigned i;
    for (i = 0; i < n && g->scriptCoins < 255; i++)
    {
        unsigned heads = TcgDuel_Coin(g, g->current);
        if (heads && g->scriptCoins < 32)
            g->scriptBits |= 1u << g->scriptCoins;
        g->scriptCoins++;
        g->scriptHeads += heads ? 1 : 0;
    }
}
/* Special Conditions by name ("par", "slp+psn"...) onto f */
static void ScriptStatus(struct TcgFieldCard *f, const char *what)
{
    if (strstr(what, "slp"))
        f->condition = TCG_ASLEEP;
    if (strstr(what, "par"))
        f->condition = TCG_PARALYZED;
    if (strstr(what, "cnf"))
        f->condition = TCG_CONFUSED;
    if (strstr(what, "psn"))
        f->poison = strstr(what, "psn30") ? 30 : strstr(what, "psn20") ? 20 : 10;
    if (strstr(what, "brn"))
        f->burned = 1;
}
static unsigned ScriptEnergyOf(const struct TcgDuel *g, unsigned who, unsigned slot, int type)
{
    return type >= TCG_TYPES ? EnergyCards(g, who, slot) : EnergyOfType(g, who, slot, (unsigned)type);
}
/* The Basic an evolution line starts from (its name: for addfam) */
static const char *ScriptFamily(const struct TcgCard *c)
{
    unsigned depth, i;
    for (depth = 0; c && c->stage && depth < 3; depth++)
    {
        const struct TcgCard *from = NULL;
        for (i = 1; i <= TCG_CARD_COUNT && !from; i++)
        {
            const struct TcgCard *k = Tcg_CardAny(i);
            if (k && k->kind == TCG_POKEMON && k->stage + 1 == c->stage && !strcmp(k->name, c->evolvesFrom))
                from = k;
        }
        if (!from)
            return c->evolvesFrom;
        c = from;
    }
    return c ? c->name : "";
}
static int ScriptRun(struct TcgDuel *g, const char *script, const struct TcgAttack *a, const struct TcgAction *action,
                     unsigned *damage, int after)
{
    struct TcgPlayer *p = &g->players[g->current], *op = &g->players[g->current ^ 1];
    struct TcgFieldCard *self = &p->field[sScriptSelf], *foe = &op->field[0];
    unsigned me = g->current, them = g->current ^ 1, s;
    int blocked = g->foeImmune == TCG_IMMUNE_YES || (foe->barrier == 3 && after == 1);
    struct ScriptOp o;
    const char *at = script;
    sScriptOption = action ? action->option : 0;
    if (after == 1)
        sScriptDealt = damage ? *damage : 0;
    while (ScriptNext(&at, &o))
    {
        int x = o.arg[0], y = o.arg[1], z = o.arg[2];
        if (ScriptPhase(&o) != after)
            continue;
        if (!ScriptCond(g, o.cond))
            continue;
        (void)z;
        /* coins */
        if (ScriptIs(&o, "c"))
            ScriptFlip(g, x ? (unsigned)x : 1);
        else if (ScriptIs(&o, "cu"))
        {
            while (g->scriptCoins < 60 && TcgDuel_Coin(g, me))
                g->scriptCoins++, g->scriptHeads++;
            g->scriptCoins++;
        }
        else if (ScriptIs(&o, "ce"))
        {
            unsigned n = ScriptEnergyOf(g, me, sScriptSelf, x);
            ScriptFlip(g, o.args > 1 && n > (unsigned)y ? (unsigned)y : n);
        }
        else if (ScriptIs(&o, "cdc"))
            ScriptFlip(g, foe->damage / 10);
        else if (ScriptIs(&o, "csc"))
            ScriptFlip(g, self->damage / 10);
        else if (ScriptIs(&o, "opt"))
        {
            const char *labels[TCG_MAX_OPTIONS];
            uint8_t ok[TCG_MAX_OPTIONS];
            unsigned n = ScriptOptions(g, me, script, labels, ok);
            if (n ? sScriptOption >= n || !ok[sScriptOption] : sScriptOption != 0)
                return TCG_INVALID; /* (no menu: option 0) */
        }
        else if (ScriptIs(&o, "cop"))
            ScriptFlip(g, sScriptOption);
        else if (ScriptIs(&o, "addop"))
            *damage += (unsigned)x * sScriptOption;
        else if (ScriptIs(&o, "cde"))
            ScriptFlip(g, EnergyCards(g, them, 0));
        else if (ScriptIs(&o, "coe") || ScriptIs(&o, "cmbe"))
        {
            unsigned n = 0;
            for (s = ScriptIs(&o, "cmbe") ? 1 : 0; s < TCG_FIELD_SIZE; s++)
                n += EnergyCards(g, ScriptIs(&o, "coe") ? them : me, s);
            ScriptFlip(g, n);
        }
        else if (ScriptIs(&o, "cpk") || ScriptIs(&o, "mulpk"))
        {
            unsigned who, n = 0;
            for (who = 0; who < 2; who++)
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                    n += g->players[who].field[s].stackSize && (ScriptIs(&o, "cpk") || who == me);
            if (ScriptIs(&o, "cpk"))
                ScriptFlip(g, n);
            else
                *damage = (unsigned)x * n;
        }
        else if (ScriptIs(&o, "cnb") || ScriptIs(&o, "cnp") || ScriptIs(&o, "addnb"))
        {
            /* a coin for each of your Benched Pokemon (cnb) / your Pokemon in play (cnp) of name group x; addnb:x,k,max x
             * per Benched one of name group k (TcgFlow_InNameGroup; at most max, 0 any) */
            unsigned n = 0, group = ScriptIs(&o, "addnb") ? (unsigned)y : (unsigned)x;
            for (s = ScriptIs(&o, "cnp") ? 0 : 1; s < TCG_FIELD_SIZE; s++)
                n += TcgDuel_Field(g, me, s) && TcgFlow_InNameGroup(TcgDuel_Field(g, me, s)->name, group);
            if (ScriptIs(&o, "addnb"))
                *damage += (unsigned)x * (z && n > (unsigned)z ? (unsigned)z : n);
            else
                ScriptFlip(g, n);
        }
        else if (ScriptIs(&o, "dbsame"))
        {
            /* (before the damage) the Basic Pokemon picked from the deck named as it onto the Bench, as many as fit */
            const struct TcgCard *it = TcgDuel_Field(g, me, sScriptSelf);
            unsigned k, n = action ? action->selectionCount : 0;
            for (k = 0; k < n; k++)
            {
                int at = Picked(action, k, p->deck, p->deckSize);
                const struct TcgCard *c = at >= 0 ? CardAt(g, me, p->deck[at]) : NULL;
                unsigned room;
                if (!c || !it || c->kind != TCG_POKEMON || c->stage || strcmp(c->name, it->name))
                    return TCG_INVALID;
                for (room = 1; room < TCG_FIELD_SIZE && p->field[room].stackSize; room++)
                    ;
                if (room == TCG_FIELD_SIZE)
                    break;
                memset(&p->field[room], 0, sizeof(p->field[room]));
                p->field[room].stack[p->field[room].stackSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
                p->field[room].enteredTurn = g->turn;
                TcgDuel_Event(g, TCG_EV_PLAY, me, room, 0, c->id, 0);
            }
            if (n)
                TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "dfld"))
        {
            /* the option's number of Energy cards of type x (15 any) picked from any of your Pokemon, discarded; y: a
             * coin for each */
            unsigned want = sScriptOption, k, i, n = action ? action->selectionCount : 0;
            if (n != want) /* (as Burning Fire: all the picks the option asked for, else none) */
                n = 0;
            for (k = 0; k < n; k++)
            {
                int at = -1;
                for (i = 0; i < TCG_FIELD_SIZE && at < 0; i++)
                    if ((at = Picked(action, k, p->field[i].attached, p->field[i].attachedSize)) >= 0)
                    {
                        const struct TcgCard *c = CardAt(g, me, p->field[i].attached[at]);
                        if (c->kind != TCG_ENERGY || (x < TCG_TYPES && c->type != (unsigned)x))
                            return TCG_INVALID;
                    }
                if (at < 0)
                    return TCG_INVALID;
            }
            for (k = 0; k < n; k++)
                for (i = 0; i < TCG_FIELD_SIZE; i++)
                {
                    int at = Picked(action, k, p->field[i].attached, p->field[i].attachedSize);
                    if (at >= 0)
                    {
                        p->attachmentExpires[p->field[i].attached[at]] = 0;
                        p->discard[p->discardSize++] = TcgPile_Remove(p->field[i].attached, &p->field[i].attachedSize, (unsigned)at);
                        break;
                    }
                }
            if (y)
                ScriptFlip(g, n);
        }
        else if (ScriptIs(&o, "addfam"))
        {
            /* (its family: the Basic its evolution line starts from) */
            const struct TcgCard *it = TcgDuel_Field(g, me, sScriptSelf);
            unsigned n = 0;
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                n += s != sScriptSelf && it && TcgDuel_Field(g, me, s) && ScriptFamily(TcgDuel_Field(g, me, s)) == ScriptFamily(it);
            *damage += (unsigned)x * n;
        }
        else if (ScriptIs(&o, "mulmb"))
            *damage = (unsigned)x * BenchCount(g, me);
        else if (ScriptIs(&o, "addtypes"))
        {
            unsigned t, n = 0;
            for (t = 0; t < TCG_COLORLESS; t++)
            {
                unsigned k;
                for (k = 0; k < self->attachedSize; k++)
                    if (CardAt(g, me, self->attached[k])->id <= 6 && CardAt(g, me, self->attached[k])->type == t)
                        break;
                n += k < self->attachedSize;
            }
            *damage += (unsigned)x * n;
        }
        else if (ScriptIs(&o, "dsxh"))
        {
            if (ScriptEnergyOf(g, me, sScriptSelf, x) < g->scriptHeads)
            {
                *damage = 0;
                g->attackFailed = 1;
                return TCG_OK;
            }
            if (g->scriptHeads) /* (DiscardEnergy's 0 is every one of them) */
                DiscardEnergy(g, me, sScriptSelf, (unsigned)x, g->scriptHeads);
        }
        else if (ScriptIs(&o, "adddis"))
        {
            unsigned k, n = 0;
            for (k = 0; k < p->discardSize; k++)
                n += CardAt(g, me, p->discard[k])->kind == TCG_ENERGY && CardAt(g, me, p->discard[k])->type == (unsigned)x;
            *damage += (unsigned)y * n;
        }
        else if (ScriptIs(&o, "mulname"))
        {
            const struct TcgCard *it = TcgDuel_Field(g, me, sScriptSelf);
            unsigned n = 0;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                n += it && TcgDuel_Field(g, me, s) && strstr(TcgDuel_Field(g, me, s)->name, it->name) != NULL;
            *damage = (unsigned)x * n;
        }
        else if (ScriptIs(&o, "addbd"))
        {
            int mine = 0, theirs = 0;
            for (s = 1; s < TCG_FIELD_SIZE; s++)
            {
                mine += p->field[s].stackSize != 0;
                theirs += op->field[s].stackSize != 0;
            }
            if ((y ? mine - theirs : theirs - mine) > 0)
                *damage += (unsigned)x * (unsigned)(y ? mine - theirs : theirs - mine);
        }
        else if (ScriptIs(&o, "addrc"))
            *damage += foe->stackSize ? (unsigned)x * TcgDuel_RetreatCost(g, them) : 0;
        else if (ScriptIs(&o, "addpk"))
        {
            const struct TcgCard *e = action && action->selectionCount ? CardAt(g, me, action->selections[0]) : NULL;
            if (e && e->kind == TCG_ENERGY && e->type == (unsigned)x)
                *damage += (unsigned)y;
        }
        else if (ScriptIs(&o, "milladdb"))
        {
            /* each player discards the top x of their deck: z more for each Energy of type y among them */
            unsigned who, k, n = 0;
            for (who = 0; who < 2; who++)
            {
                struct TcgPlayer *q = &g->players[who];
                for (k = 0; k < (unsigned)x && q->deckSize; k++)
                {
                    const struct TcgCard *c = CardAt(g, who, q->deck[q->deckSize - 1]);
                    n += c->kind == TCG_ENERGY && c->type == (unsigned)y;
                    q->discard[q->discardSize++] = q->deck[--q->deckSize];
                }
                if (k)
                    TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, k, 0, 0);
            }
            *damage += (unsigned)z * n;
        }
        /* damage */
        else if (ScriptIs(&o, "add"))
            *damage += (unsigned)x;
        else if (ScriptIs(&o, "addh"))
            *damage += (unsigned)x * g->scriptHeads;
        else if (ScriptIs(&o, "mulh"))
            *damage = (unsigned)x * g->scriptHeads;
        else if (ScriptIs(&o, "set"))
            *damage = (unsigned)x;
        else if (ScriptIs(&o, "zero"))
            *damage = 0;
        else if (ScriptIs(&o, "none"))
        {
            *damage = 0;
            g->attackFailed = 1;
            return TCG_OK;
        }
        else if (ScriptIs(&o, "addsc"))
            *damage += (unsigned)x * (self->damage / 10);
        else if (ScriptIs(&o, "adddc"))
            *damage += (unsigned)x * (foe->damage / 10);
        else if (ScriptIs(&o, "adde"))
        {
            unsigned n = ScriptEnergyOf(g, me, sScriptSelf, x);
            if (o.args > 2 && n > (unsigned)z)
                n = (unsigned)z;
            *damage += (unsigned)y * n;
        }
        else if (ScriptIs(&o, "addde"))
            *damage += (unsigned)x * EnergyCards(g, them, 0);
        else if (ScriptIs(&o, "addpast"))
        {
            unsigned n = EnergyPastCost(g, a, x >= TCG_TYPES);
            if (n > (unsigned)z)
                n = (unsigned)z;
            *damage += (unsigned)y * n;
        }
        else if (ScriptIs(&o, "halfhp"))
        {
            /* half the Defending Pokemon's remaining HP (x: rounded up, else down, to 10) */
            const struct TcgCard *d = TcgDuel_Field(g, them, 0);
            unsigned left = d && TcgDuel_MaxHp(g, them, 0) > foe->damage ? TcgDuel_MaxHp(g, them, 0) - foe->damage : 0u;
            *damage = x ? (left / 2 + 9) / 10 * 10 : left / 2 / 10 * 10;
        }
        else if (ScriptIs(&o, "subsc"))
            *damage = *damage > (unsigned)x * (self->damage / 10) ? *damage - (unsigned)x * (self->damage / 10) : 0;
        else if (ScriptIs(&o, "dbl"))
            *damage *= 2;
        else if (ScriptIs(&o, "milladd"))
        {
            /* discard the top x of your deck: z more for each Energy of type y among them */
            unsigned k, n = 0;
            for (k = 0; k < (unsigned)x && p->deckSize; k++)
            {
                const struct TcgCard *c = CardAt(g, me, p->deck[p->deckSize - 1]);
                n += c->kind == TCG_ENERGY && c->type == (unsigned)y;
                p->discard[p->discardSize++] = p->deck[--p->deckSize];
            }
            if (k)
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, k, 0, 0);
            *damage += (unsigned)z * n;
        }
        else if (ScriptIs(&o, "adds"))
        {
            unsigned k, n = 0;
            for (k = 0; k < foe->attachedSize; k++)
                n += (CardAt(g, them, foe->attached[k])->flags & TCG_CF_SPECIAL) != 0;
            *damage += (unsigned)x * n;
        }
        else if (ScriptIs(&o, "addoh"))
        {
            unsigned k, n = 0;
            for (k = 0; k < op->handSize; k++)
                n += CardAt(g, them, op->hand[k])->kind == TCG_ENERGY;
            *damage += (unsigned)x * n;
        }
        else if (ScriptIs(&o, "eret"))
        {
            /* an Energy of type x from it back to the hand, to use it */
            unsigned k;
            int found = 0;
            for (k = 0; k < self->attachedSize && !found; k++)
                if (CardAt(g, me, self->attached[k])->kind == TCG_ENERGY && CardAt(g, me, self->attached[k])->type == (unsigned)x)
                {
                    p->hand[p->handSize++] = TcgPile_Remove(self->attached, &self->attachedSize, k);
                    found = 1;
                }
            if (!found)
                return TCG_INVALID;
        }
        else if (ScriptIs(&o, "dtrdisc"))
        {
            unsigned k;
            for (k = foe->attachedSize; k > 0; k--)
                if (CardAt(g, them, foe->attached[k - 1])->kind == TCG_TRAINER)
                {
                    op->attachmentExpires[foe->attached[k - 1]] = 0;
                    op->discard[op->discardSize++] = TcgPile_Remove(foe->attached, &foe->attachedSize, k - 1);
                }
        }
        else if (ScriptIs(&o, "cmine"))
        {
            unsigned n = 0;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                n += p->field[s].stackSize != 0;
            ScriptFlip(g, n);
        }
        else if (ScriptIs(&o, "addmd"))
        {
            unsigned n = 0;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                n += p->field[s].stackSize && p->field[s].damage;
            *damage += (unsigned)x * n;
        }
        else if (ScriptIs(&o, "nores"))
            g->noWeakRes = 2;
        else if (ScriptIs(&o, "noweak"))
            g->noWeakRes = 3;
        else if (ScriptIs(&o, "nowr"))
            g->noWeakRes = 1;
        else if (ScriptIs(&o, "dsx"))
        {
            /* discard them "or this attack does nothing" */
            unsigned have = ScriptEnergyOf(g, me, sScriptSelf, x);
            if (!have || (y && have < (unsigned)y))
            {
                *damage = 0;
                g->attackFailed = 1;
                return TCG_OK;
            }
            DiscardEnergy(g, me, sScriptSelf, x >= TCG_TYPES ? ANY_TYPE : (unsigned)x, (unsigned)y);
        }
        else if (ScriptIs(&o, "dse"))
        {
            /* the cost: (any type and a number: the cards picked, as Fire Spin) */
            if (x >= TCG_TYPES && y && action->selectionCount == (unsigned)y)
            {
                unsigned k;
                for (k = 0; k < (unsigned)y; k++)
                {
                    int i = Picked(action, k, self->attached, self->attachedSize);
                    if (i < 0 || CardAt(g, me, self->attached[i])->kind != TCG_ENERGY)
                        return TCG_INVALID;
                    p->discard[p->discardSize++] = TcgPile_Remove(self->attached, &self->attachedSize, (unsigned)i);
                }
            }
            else if (!DiscardEnergy(g, me, sScriptSelf, x >= TCG_TYPES ? ANY_TYPE : (unsigned)x, (unsigned)y))
                return TCG_INVALID;
        }
        /* after the damage */
        else if (ScriptIs(&o, "par") || ScriptIs(&o, "slp") || ScriptIs(&o, "cnf") || ScriptIs(&o, "psn") ||
                 ScriptIs(&o, "psn20") || ScriptIs(&o, "psn30") || ScriptIs(&o, "brn"))
        {
            if (!blocked && foe->stackSize)
                ScriptStatus(foe, o.name);
        }
        else if (ScriptIs(&o, "spar") || ScriptIs(&o, "sslp") || ScriptIs(&o, "scnf") || ScriptIs(&o, "spsn"))
        {
            if (self->stackSize)
                ScriptStatus(self, o.name + 1);
        }
        else if (ScriptIs(&o, "rec"))
        {
            /* (doubled with its base damage by a boost:i,0 on this attack: Focus Energy) */
            const struct TcgCard *it = TcgDuel_Field(g, me, sScriptSelf);
            if (it && a && self->boostTurn == g->turn && self->boostAttack && !self->boostValue &&
                a == &it->attacks[self->boostAttack - 1])
                x *= 2;
            TcgDuel_SelfDamage(g, x);
        }
        else if (ScriptIs(&o, "react"))
        {
            /* when an attack damages it through their next turn (y: through your next turn too): TCG_REACT_x */
            self->reactKind = (uint8_t)x;
            self->reactUntil = (uint16_t)(g->turn + (y ? (unsigned)y : 1u));
        }
        else if (ScriptIs(&o, "pursuit") || ScriptIs(&o, "attpsn"))
        {
            /* the Defending Pokemon: 10 if it retreats / Poisoned if an Energy is attached to it, through their turn */
            if (!blocked && foe->stackSize)
            {
                if (ScriptIs(&o, "pursuit"))
                    foe->pursuitUntil = (uint16_t)(g->turn + 1);
                else
                    foe->attachPoisonUntil = (uint16_t)(g->turn + 1);
            }
        }
        else if (ScriptIs(&o, "anchor"))
        {
            /* the Defending Pokemon can't retreat and no effect switches it out: through their next turn (anchor:0),
             * and only while it is your Active (1), through your next turn while it is (2) */
            if (!blocked && foe->stackSize)
            {
                foe->anchorUntil = (uint16_t)(g->turn + (x == 2 ? 2 : 1));
                foe->anchorBy = (uint8_t)(x ? self->stack[self->stackSize - 1] + 1 : 0);
            }
        }
        else if (ScriptIs(&o, "koslp"))
        {
            /* the Defending Pokemon Knocked Out if it's Asleep and it used its attack x last turn (Perish Song) */
            const struct TcgCard *d = TcgDuel_Field(g, them, 0);
            if (!blocked && d && foe->condition == TCG_ASLEEP && self->lastAttack == x + 1 &&
                self->lastAttackTurn + 2 == g->turn && foe->damage < TcgDuel_MaxHp(g, them, 0))
            {
                unsigned add = TcgDuel_MaxHp(g, them, 0) - foe->damage;
                foe->damage = (uint16_t)TcgDuel_MaxHp(g, them, 0);
                TcgDuel_Event(g, TCG_EV_DAMAGE, them, 0, add, 0, 0);
            }
        }
        else if (ScriptIs(&o, "drain"))
        {
            /* counters off it for the damage done to the Defending Pokemon: all of it (1) / half, rounded up (2) */
            if (sScriptDealt)
                Heal(g, me, sScriptSelf, x == 2 ? (sScriptDealt / 2 + 9) / 10 * 10 : sScriptDealt);
        }
        else if (ScriptIs(&o, "ocoinh"))
        {
            /* a coin for each of theirs: x to it on heads (no Weakness or Resistance), then that much off it */
            unsigned dealt = 0;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (op->field[s].stackSize && (s || !blocked))
                {
                    unsigned before = op->field[s].damage;
                    if (!TcgDuel_Coin(g, me))
                        continue;
                    TcgDuel_EffectDamage(g, them, s, x, 0);
                    dealt += op->field[s].damage > before ? op->field[s].damage - before : 0;
                }
            if (dealt)
                Heal(g, me, sScriptSelf, dealt);
        }
        else if (ScriptIs(&o, "ctr"))
        {
            if (self->stackSize)
            {
                self->damage += (uint16_t)(x * 10);
                TcgDuel_Event(g, TCG_EV_DAMAGE, me, sScriptSelf, (unsigned)x * 10, 0, TCG_DMG_SELF);
            }
        }
        else if (ScriptIs(&o, "heal"))
            Heal(g, me, sScriptSelf, (unsigned)x * 10);
        else if (ScriptIs(&o, "healall"))
        {
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (p->field[s].stackSize)
                    Heal(g, me, s, (unsigned)x * 10);
        }
        else if (ScriptIs(&o, "bo"))
            BenchDamage(g, them, x);
        else if (ScriptIs(&o, "bm"))
            BenchDamage(g, me, x);
        else if (ScriptIs(&o, "boh"))
            BenchDamage(g, them, x * (int)g->scriptHeads); /* (x per heads to each of their Benched) */
        else if (ScriptIs(&o, "oswitch"))
            TcgDuel_OpponentSwitch(g, them); /* (they choose who comes in, as a promotion) */
        else if (ScriptIs(&o, "gmove"))
        {
            /* Baton Pass: every Energy card of type x on it onto the Benched Pokemon of that type chosen, then they
             * switch (none of that type on the Bench: nothing) */
            const struct TcgCard *t = TcgDuel_Field(g, me, action ? action->target : 0);
            unsigned k, any = 0;
            for (k = 1; k < TCG_FIELD_SIZE; k++)
                any |= TcgDuel_Field(g, me, k) && TcgDuel_Type(g, me, k) == (unsigned)x;
            if (!any)
                continue;
            if (!action || !ValidTarget(g, me, action->target, 1) || !t || TcgDuel_Type(g, me, action->target) != (unsigned)x)
                return TCG_INVALID;
            for (k = self->attachedSize; k > 0; k--)
            {
                const struct TcgCard *e = CardAt(g, me, self->attached[k - 1]);
                if (e->kind == TCG_ENERGY && e->id <= 6 && e->type == (unsigned)x)
                {
                    struct TcgFieldCard *to = &p->field[action->target];
                    to->attached[to->attachedSize++] = TcgPile_Remove(self->attached, &self->attachedSize, k - 1);
                    TcgDuel_Event(g, TCG_EV_ATTACH, me, action->target, 0, e->id, 0);
                }
            }
            Switch(g, me, action->target);
        }
        else if (ScriptIs(&o, "mime"))
        {
            /* Sleight of Hand: the cards picked in the hand (up to 3) on top of the deck, then as many basic Energy
             * cards picked from the deck (at most) into the hand; the deck shuffled */
            unsigned k, fromHand = 0, fromDeck = 0;
            uint8_t take[TCG_DECK_SIZE];
            for (k = 0; action && k < action->selectionCount; k++)
            {
                int h = Picked(action, k, p->hand, p->handSize), d = Picked(action, k, p->deck, p->deckSize);
                if (h >= 0)
                {
                    if (++fromHand > 3)
                        return TCG_INVALID;
                }
                else if (d >= 0 && CardAt(g, me, p->deck[d])->kind == TCG_ENERGY && CardAt(g, me, p->deck[d])->id <= 6)
                    take[fromDeck++] = action->selections[k];
                else
                    return TCG_INVALID;
            }
            if (fromDeck > fromHand)
                return TCG_INVALID;
            for (k = 0; action && k < action->selectionCount; k++)
            {
                int h = Picked(action, k, p->hand, p->handSize);
                if (h >= 0)
                    p->deck[p->deckSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)h);
            }
            for (k = 0; k < fromDeck; k++)
            {
                unsigned j;
                for (j = 0; j < p->deckSize && p->deck[j] != take[k]; j++)
                    ;
                TcgDuel_Log(g, "Shown: %s.", CardAt(g, me, take[k])->name);
                p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, j);
            }
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "bb"))
        {
            BenchDamage(g, them, x);
            BenchDamage(g, me, x);
        }
        else if (ScriptIs(&o, "b1"))
        {
            if (BenchCount(g, them))
            {
                if (!ValidTarget(g, them, action->target, 1))
                    return TCG_INVALID;
                TcgDuel_EffectDamage(g, them, action->target, x, 0);
            }
        }
        else if (ScriptIs(&o, "any1"))
        {
            if (!ValidTarget(g, them, action->target, 0) || (y && !op->field[action->target].damage))
                return TCG_INVALID;
            if (action->target || !blocked)
                TcgDuel_EffectDamage(g, them, action->target, x, 0);
        }
        else if (ScriptIs(&o, "osw"))
        {
            if (BenchCount(g, them) && !blocked)
                g->switchPending = 1;
        }
        else if (ScriptIs(&o, "dragmay"))
        {
            unsigned t = action->target;
            if (t && BenchCount(g, them) && !blocked)
            {
                if (!ValidTarget(g, them, t, 1) || (x && op->field[t].damage))
                    return TCG_INVALID;
                Switch(g, them, t);
            }
        }
        else if (ScriptIs(&o, "weak"))
        {
            if (!blocked && foe->stackSize)
            {
                foe->weaken = (uint8_t)x;
                foe->effectBy = (uint8_t)(self->stack[self->stackSize - 1] + 1);
            }
        }
        else if (ScriptIs(&o, "nsrch"))
        {
            /* the cards of name group x picked from the deck into the hand (up to y, 0 any number) */
            unsigned k, n = action ? action->selectionCount : 0;
            for (k = 0; k < n; k++)
            {
                int at = Picked(action, k, p->deck, p->deckSize);
                if (at < 0 || !TcgFlow_InNameGroup(CardAt(g, me, p->deck[at])->name, (unsigned)x) || (y && k >= (unsigned)y))
                    return TCG_INVALID;
            }
            for (k = 0; k < n; k++)
            {
                int at = Picked(action, k, p->deck, p->deckSize);
                TcgDuel_Log(g, "Shown: %s.", CardAt(g, me, p->deck[at])->name);
                p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
            }
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "dbown"))
        {
            /* the Basic Pokemon with owner x's name (TCG_OWNER_*) picked from the deck onto the Bench */
            int at = action && action->selectionCount ? Picked(action, 0, p->deck, p->deckSize) : -1;
            if (at >= 0)
            {
                const struct TcgCard *c = CardAt(g, me, p->deck[at]);
                unsigned room;
                for (room = 1; room < TCG_FIELD_SIZE && p->field[room].stackSize; room++)
                    ;
                if (c->kind != TCG_POKEMON || c->stage || !strstr(c->name, TcgFlow_OwnerWord((unsigned)x)) ||
                    room == TCG_FIELD_SIZE)
                    return TCG_INVALID;
                memset(&p->field[room], 0, sizeof(p->field[room]));
                p->field[room].stack[p->field[room].stackSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
                p->field[room].enteredTurn = g->turn;
                TcgDuel_Event(g, TCG_EV_PLAY, me, room, 0, c->id, 0);
            }
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "srchk") || ScriptIs(&o, "dbench") || ScriptIs(&o, "disbench") || ScriptIs(&o, "dishand"))
        {
            int fromDeck = ScriptIs(&o, "srchk") || ScriptIs(&o, "dbench");
            int at = action && action->selectionCount ? Picked(action, 0, fromDeck ? p->deck : p->discard,
                                                               fromDeck ? p->deckSize : p->discardSize) : -1;
            if (at >= 0)
            {
                uint8_t card = fromDeck ? p->deck[at] : p->discard[at];
                const struct TcgCard *c = CardAt(g, me, card);
                int ok = ScriptIs(&o, "srchk")
                             ? (x == 3 ? c->kind == TCG_POKEMON /* (srchk:3 a Basic Pokemon or Evolution card) */
                                : x == 4 ? c->kind == TCG_POKEMON && TcgDuel_Field(g, me, sScriptSelf) &&
                                               strcmp(c->name, TcgDuel_Field(g, me, sScriptSelf)->name) /* (4: not named as it) */
                                       : c->kind == TCG_TRAINER &&
                                             (x == 0 ? (c->flags & TCG_CF_SUPPORTER) != 0
                                              : x == 1 ? (c->flags & (TCG_CF_TM | TCG_CF_TOOL)) != 0 : 1))
                         : ScriptIs(&o, "dishand") ? c->kind == TCG_POKEMON || (c->kind == TCG_ENERGY && c->id <= 6)
                         : x == 3 ? c->kind == TCG_POKEMON && !strcmp(c->evolvesFrom, "Mysterious Fossil") /* (dbench:3) */
                                                   : c->kind == TCG_POKEMON && !c->stage && (x != 1 || (c->flags & TCG_CF_BABY)) &&
                                                         (x != 2 || c->type == TCG_METAL); /* (dbench:2 a Metal Basic) */
                if (!ok)
                    return TCG_INVALID;
                if (ScriptIs(&o, "srchk") || ScriptIs(&o, "dishand"))
                {
                    p->hand[p->handSize++] = TcgPile_Remove(fromDeck ? p->deck : p->discard,
                                                            fromDeck ? &p->deckSize : &p->discardSize, (unsigned)at);
                    TcgDuel_Log(g, "Shown: %s.", c->name);
                }
                else
                {
                    unsigned k;
                    for (k = 1; k < TCG_FIELD_SIZE && p->field[k].stackSize; k++)
                        ;
                    if (k == TCG_FIELD_SIZE)
                        return TCG_INVALID;
                    memset(&p->field[k], 0, sizeof(p->field[k]));
                    p->field[k].stack[p->field[k].stackSize++] = TcgPile_Remove(fromDeck ? p->deck : p->discard,
                                                                                fromDeck ? &p->deckSize : &p->discardSize, (unsigned)at);
                    p->field[k].enteredTurn = g->turn;
                    TcgDuel_Event(g, TCG_EV_PLAY, me, k, 0, c->id, 0);
                }
            }
            if (fromDeck)
                TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "hatt"))
        {
            /* the Energy picked from the hand onto it / the one of yours chosen */
            unsigned to = z ? action->target : sScriptSelf, k;
            struct TcgFieldCard *t;
            if (!ValidTarget(g, me, to, 0) || (action->selectionCount > (unsigned)y))
                return TCG_INVALID;
            if (z && x == 2 && TcgDuel_Type(g, me, to) != TCG_GRASS)
                return TCG_INVALID;
            for (k = 0; k < action->selectionCount; k++)
            {
                int at = Picked(action, k, p->hand, p->handSize);
                const struct TcgCard *c = at >= 0 ? CardAt(g, me, p->hand[at]) : NULL;
                if (!c || c->kind != TCG_ENERGY || (x >= 1 && c->id > 6) || (x == 2 && c->id != TCG_CARD_GRASS_ENERGY))
                    return TCG_INVALID;
            }
            t = &p->field[to];
            for (k = 0; k < action->selectionCount; k++)
            {
                int at = Picked(action, k, p->hand, p->handSize);
                t->attached[t->attachedSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)at);
                TcgDuel_Event(g, TCG_EV_ATTACH, me, to, 0, p->cards[t->attached[t->attachedSize - 1]], 0);
            }
        }
        else if (ScriptIs(&o, "lure") || ScriptIs(&o, "drag"))
        {
            if (BenchCount(g, them) && !blocked)
            {
                if (!ValidTarget(g, them, action->target, 1))
                    return TCG_INVALID;
                Switch(g, them, action->target);
            }
        }
        else if (ScriptIs(&o, "ssw"))
        {
            if (BenchCount(g, me))
            {
                if (!ValidTarget(g, me, action->target, 1))
                    return TCG_INVALID;
                Switch(g, me, action->target);
            }
        }
        else if (ScriptIs(&o, "bar"))
            self->barrier = (uint8_t)(x ? x : 1);
        else if (ScriptIs(&o, "guard"))
            self->guard = (uint8_t)x;
        else if (ScriptIs(&o, "askch"))
        {
            /* a choice for a player (TCG_CHOICE_x): the opponent first, then you (askch:x,1: only you) */
            const struct TcgCard *it = TcgDuel_Field(g, me, sScriptSelf);
            TcgDuel_AskChoice(g, o.arg[1] ? me : them, (unsigned)x, it ? it->id : 0, o.arg[1] ? 1 : 0);
        }
        else if (ScriptIs(&o, "lockon"))
        {
            /* Lock-on: its Electric Bolt's tails are heads next turn, against this Defending Pokemon */
            if (foe->stackSize && !blocked)
            {
                self->lockOnUntil = (uint16_t)(g->turn + 2);
                self->lockOnFoe = foe->stack[foe->stackSize - 1];
            }
        }
        else if (ScriptIs(&o, "guardk"))
            self->guardKeep = (uint8_t)x; /* (until it takes damage, is Benched or evolves) */
        else if (ScriptIs(&o, "amn"))
        {
            /* Terrorize: the chosen attack of the Defending Pokemon (opt:11) can't be used next turn */
            const struct TcgCard *d = TcgDuel_Field(g, them, 0);
            if (!blocked && d && d->kind == TCG_POKEMON && !d->stage && sScriptOption < TCG_ATTACKS &&
                RealAttack(&d->attacks[sScriptOption]))
                Amnesia(g, sScriptOption);
        }
        else if (ScriptIs(&o, "nort"))
        {
            if (!blocked && foe->stackSize)
                foe->noRetreat = 1;
        }
        else if (ScriptIs(&o, "noat"))
        {
            if (!blocked && foe->stackSize)
                foe->noAttack = 1;
        }
        else if (ScriptIs(&o, "lock"))
        {
            self->lockTurn = (uint16_t)(g->turn + 2);
            self->lockAttack = (uint8_t)(action->attack + 1);
        }
        else if (ScriptIs(&o, "selflock"))
        {
            self->lockTurn = (uint16_t)(g->turn + 2);
            self->lockAttack = TCG_LOCK_ATTACKS;
        }
        else if (ScriptIs(&o, "dde"))
        {
            if (!blocked && foe->stackSize)
                DiscardFoeEnergyPicked(g, them, 0, 0, action);
        }
        else if (ScriptIs(&o, "attd"))
        {
            /* up to n Energy cards of type T from the discard pile onto it (all alike: the first ones) */
            unsigned k, n = 0;
            for (k = p->discardSize; k > 0 && n < (unsigned)y; k--)
            {
                const struct TcgCard *c = CardAt(g, me, p->discard[k - 1]);
                if (c->kind != TCG_ENERGY || c->id > 6 || (x < TCG_TYPES && c->type != (unsigned)x))
                    continue;
                self->attached[self->attachedSize++] = TcgPile_Remove(p->discard, &p->discardSize, k - 1);
                TcgDuel_Event(g, TCG_EV_ATTACH, me, sScriptSelf, 0, p->cards[self->attached[self->attachedSize - 1]], 0);
                n++;
            }
        }
        else if (ScriptIs(&o, "srch"))
        {
            unsigned k, n = 0;
            for (k = p->deckSize; k > 0 && n < (unsigned)y; k--)
            {
                const struct TcgCard *c = CardAt(g, me, p->deck[k - 1]);
                if (c->kind != TCG_ENERGY || c->id > 6 || (x < TCG_TYPES && c->type != (unsigned)x))
                    continue;
                self->attached[self->attachedSize++] = TcgPile_Remove(p->deck, &p->deckSize, k - 1);
                TcgDuel_Event(g, TCG_EV_ATTACH, me, sScriptSelf, 0, p->cards[self->attached[self->attachedSize - 1]], 0);
                n++;
            }
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "dsa"))
            DiscardEnergy(g, me, sScriptSelf, x >= TCG_TYPES ? ANY_TYPE : (unsigned)x, (unsigned)y);
        else if (ScriptIs(&o, "vuln"))
        {
            if (!blocked && foe->stackSize)
            {
                foe->vulnBonus = (uint8_t)x;
                foe->vulnUntil = (uint16_t)(g->turn + 2);
            }
        }
        else if (ScriptIs(&o, "srchp"))
        {
            /* the basic Energy card the player picked from the deck (none: none), onto it */
            int i = Picked(action, 0, p->deck, p->deckSize);
            if (i >= 0 && CardAt(g, me, p->deck[i])->kind == TCG_ENERGY && CardAt(g, me, p->deck[i])->id <= 6 &&
                (x >= TCG_TYPES || CardAt(g, me, p->deck[i])->type == (unsigned)x))
            {
                self->attached[self->attachedSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)i);
                TcgDuel_Event(g, TCG_EV_ATTACH, me, sScriptSelf, 0, p->cards[self->attached[self->attachedSize - 1]], 0);
            }
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "draw"))
            TcgDuel_Event(g, TCG_EV_DRAW, me, sScriptSelf, TcgDuel_Draw(g, me, (unsigned)x), 0, 0);
        else if (ScriptIs(&o, "notr"))
            op->noTrainers = 1;
        else if (ScriptIs(&o, "nosup"))
            op->noSupporters = 1;
        else if (ScriptIs(&o, "handdeck"))
        {
            while (p->handSize)
                p->deck[p->deckSize++] = p->hand[--p->handSize];
            TcgDuel_Shuffle(g, me);
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
        }
        else if (ScriptIs(&o, "dish") || ScriptIs(&o, "srchh"))
        {
            /* up to y basic Energy of type x (15 any) from the discard pile / the deck into the hand */
            uint8_t *pile = ScriptIs(&o, "dish") ? p->discard : p->deck;
            uint8_t *size = ScriptIs(&o, "dish") ? &p->discardSize : &p->deckSize;
            unsigned k, n = 0;
            for (k = *size; k > 0 && n < (unsigned)y; k--)
            {
                const struct TcgCard *c = CardAt(g, me, pile[k - 1]);
                if (c->kind != TCG_ENERGY || c->id > 6 || (x < TCG_TYPES && c->type != (unsigned)x))
                    continue;
                p->hand[p->handSize++] = TcgPile_Remove(pile, size, k - 1);
                n++;
            }
            if (ScriptIs(&o, "srchh"))
                TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "nop"))
            ;
        else if (ScriptIs(&o, "oall") || ScriptIs(&o, "powdmg") || ScriptIs(&o, "bnt") || ScriptIs(&o, "bot"))
        {
            /* x to each of theirs (oall); to each Pokemon in play with a power (powdmg); y to each Benched Pokemon not of
             * type x (bnt, both sides) / of type x (bot, theirs) */
            unsigned who;
            for (who = 0; who < 2; who++)
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                {
                    const struct TcgCard *c = TcgDuel_Field(g, who, s);
                    int hit = 0, amount = x;
                    if (!c || c->kind != TCG_POKEMON)
                        continue;
                    if (ScriptIs(&o, "oall"))
                        hit = who == them && (s || !blocked);
                    else if (ScriptIs(&o, "powdmg"))
                        hit = TcgPower_Id(c) != TCG_POWER_NONE && (who != them || s || !blocked);
                    else if (s)
                    {
                        amount = y;
                        hit = ScriptIs(&o, "bnt") ? TcgDuel_Type(g, who, s) != (unsigned)x
                                                  : who == them && TcgDuel_Type(g, who, s) == (unsigned)x;
                    }
                    if (hit)
                        TcgDuel_EffectDamage(g, who, s, amount, 0);
                }
        }
        else if (ScriptIs(&o, "wake"))
        {
            if (!blocked && foe->stackSize && foe->condition == TCG_ASLEEP)
            {
                foe->condition = TCG_HEALTHY;
                TcgDuel_Event(g, TCG_EV_STATUS, them, 0, TCG_HEALTHY, 0, foe->poison);
            }
        }
        else if (ScriptIs(&o, "rect"))
        {
            unsigned t = g->scriptCoins - g->scriptHeads;
            if (t)
                TcgDuel_SelfDamage(g, (unsigned)x * t);
        }
        else if (ScriptIs(&o, "rece"))
        {
            unsigned n = ScriptEnergyOf(g, me, sScriptSelf, x);
            if (n)
                TcgDuel_SelfDamage(g, (unsigned)y * n);
        }
        else if (ScriptIs(&o, "shuf"))
            TcgDuel_Shuffle(g, me);
        else if (ScriptIs(&o, "healh"))
            Heal(g, me, sScriptSelf, (unsigned)x * 10 * g->scriptHeads);
        else if (ScriptIs(&o, "cure4"))
        {
            /* your Active (0) / the Defending Pokemon (1) no longer Asleep, Confused, Paralyzed or Poisoned */
            unsigned who = x ? them : me;
            struct TcgFieldCard *f = &g->players[who].field[0];
            if (f->stackSize && (!x || !blocked))
            {
                f->condition = TCG_HEALTHY;
                f->poison = 0;
                TcgDuel_Event(g, TCG_EV_STATUS, who, 0, TCG_HEALTHY, 0, 0);
            }
        }
        else if (ScriptIs(&o, "drawto"))
        {
            /* cards drawn until the hand has x */
            if (p->handSize < (unsigned)x)
                TcgDuel_Event(g, TCG_EV_DRAW, me, sScriptSelf, TcgDuel_Draw(g, me, (unsigned)x - p->handSize), 0, 0);
        }
        else if (ScriptIs(&o, "shiftto"))
            self->shift = (uint8_t)(x + 1); /* (its type x for the rest of the turn) */
        else if (ScriptIs(&o, "healact"))
        {
            /* x counters off each Active Pokemon (both players') */
            unsigned w;
            for (w = 0; w < 2; w++)
                if (g->players[w].field[0].stackSize && g->players[w].field[0].damage)
                    Heal(g, w, 0, (unsigned)x * 10);
        }
        else if (ScriptIs(&o, "dheal"))
        {
            /* x counters off the Defending Pokemon (0: all of them) */
            if (!blocked && foe->stackSize && foe->damage)
                Heal(g, them, 0, x ? (unsigned)x * 10 : foe->damage);
        }
        else if (ScriptIs(&o, "oetype"))
        {
            /* x to each of theirs with an Energy card of the type chosen (opt:10) attached, no Weakness or Resistance */
            static const uint8_t types[] = {TCG_GRASS, TCG_FIRE, TCG_WATER, TCG_LIGHTNING, TCG_FIGHTING, TCG_PSYCHIC,
                                            TCG_DARKNESS, TCG_METAL};
            unsigned type = types[sScriptOption < sizeof(types) ? sScriptOption : 0];
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (op->field[s].stackSize && (s || !blocked) && EnergyOfType(g, them, s, type))
                    TcgDuel_EffectDamage(g, them, s, x, 0);
        }
        else if (ScriptIs(&o, "healallh") || ScriptIs(&o, "ohealall"))
        {
            unsigned who = ScriptIs(&o, "ohealall") ? them : me;
            unsigned amount = ScriptIs(&o, "healallh") ? (unsigned)x * 10 * g->scriptHeads : (unsigned)x * 10;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (g->players[who].field[s].stackSize)
                    Heal(g, who, s, amount);
        }
        else if (ScriptIs(&o, "nopow"))
            g->goopGasUntil = (uint16_t)(g->turn + (unsigned)x);
        else if (ScriptIs(&o, "any1h"))
        {
            if (!ValidTarget(g, them, action->target, 0))
                return TCG_INVALID;
            if (g->scriptHeads && (action->target || !blocked))
                TcgDuel_EffectDamage(g, them, action->target, x * (int)g->scriptHeads, 0);
        }
        else if (ScriptIs(&o, "smill"))
        {
            unsigned k;
            for (k = 0; k < (unsigned)x && p->deckSize; k++)
                p->discard[p->discardSize++] = p->deck[--p->deckSize];
            if (k)
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, k, 0, 0);
        }
        else if (ScriptIs(&o, "dctr"))
        {
            if (!blocked && foe->stackSize && g->scriptHeads)
            {
                foe->damage += (uint16_t)(x * 10 * (int)g->scriptHeads);
                TcgDuel_Event(g, TCG_EV_DAMAGE, them, 0, (unsigned)x * 10 * g->scriptHeads, 0, 0);
            }
        }
        else if (ScriptIs(&o, "dsat"))
            DiscardEnergy(g, me, sScriptSelf, (unsigned)x, g->scriptCoins - g->scriptHeads);
        else if (ScriptIs(&o, "otrdisc"))
        {
            for (s = 0; s < TCG_FIELD_SIZE; s++)
            {
                struct TcgFieldCard *f = &op->field[s];
                unsigned k;
                for (k = f->attachedSize; k > 0; k--)
                    if (CardAt(g, them, f->attached[k - 1])->kind == TCG_TRAINER)
                    {
                        op->attachmentExpires[f->attached[k - 1]] = 0;
                        op->discard[op->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, k - 1);
                    }
            }
        }
        else if (ScriptIs(&o, "needob") || ScriptIs(&o, "needsc") || ScriptIs(&o, "needeqe") || ScriptIs(&o, "needmb") ||
                 ScriptIs(&o, "needlast") || ScriptIs(&o, "needhand") ||
                 ScriptIs(&o, "needroom") || ScriptIs(&o, "neede"))
            ; /* (ScriptBlock) */
        else if (ScriptIs(&o, "dpowoff"))
        {
            if (!blocked && foe->stackSize)
                foe->powerOffUntil = (uint16_t)(g->turn + 1);
        }
        else if (ScriptIs(&o, "bbe"))
        {
            unsigned who;
            for (who = 0; who < 2; who++)
                for (s = 1; s < TCG_FIELD_SIZE; s++)
                    if (g->players[who].field[s].stackSize && EnergyCards(g, who, s))
                        TcgDuel_EffectDamage(g, who, s, x, 0);
        }
        else if (ScriptIs(&o, "nostad"))
        {
            unsigned who;
            for (who = 0; who < 2; who++)
                if (g->players[who].stadium)
                {
                    g->players[who].discard[g->players[who].discardSize++] = (uint8_t)(g->players[who].stadium - 1);
                    g->players[who].stadium = 0;
                }
        }
        else if (ScriptIs(&o, "ohrand"))
        {
            if (op->handSize)
            {
                op->deck[op->deckSize++] = TcgPile_Remove(op->hand, &op->handSize, Tcg_RandomBelow(&g->rng, op->handSize));
                TcgDuel_Shuffle(g, them);
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "to10"))
        {
            const struct TcgCard *d = TcgDuel_Field(g, them, 0);
            if (!blocked && d && d->kind == TCG_POKEMON && TcgDuel_MaxHp(g, them, 0) > foe->damage + 10u)
            {
                unsigned add = TcgDuel_MaxHp(g, them, 0) - 10u - foe->damage;
                foe->damage = (uint16_t)(TcgDuel_MaxHp(g, them, 0) - 10u);
                TcgDuel_Event(g, TCG_EV_DAMAGE, them, 0, add, 0, 0);
            }
        }
        else if (ScriptIs(&o, "dishh"))
        {
            unsigned k, n = 0;
            for (k = p->discardSize; k > 0 && n < g->scriptHeads; k--)
            {
                const struct TcgCard *c = CardAt(g, me, p->discard[k - 1]);
                if (c->kind == TCG_ENERGY && c->id <= 6 && (x >= TCG_TYPES || c->type == (unsigned)x))
                {
                    p->hand[p->handSize++] = TcgPile_Remove(p->discard, &p->discardSize, k - 1);
                    n++;
                }
            }
        }
        else if (ScriptIs(&o, "eto"))
        {
            /* an Energy of type x (15: any basic Energy) from it onto the Benched Pokemon chosen (none: discarded, but
             * kept with y 1) */
            unsigned k;
            for (k = 0; k < self->attachedSize; k++)
                if (CardAt(g, me, self->attached[k])->kind == TCG_ENERGY &&
                    (x >= 16 ? 1 /* (any Energy card) */
                     : x >= TCG_TYPES ? CardAt(g, me, self->attached[k])->id <= 6 : CardAt(g, me, self->attached[k])->type == (unsigned)x))
                    break;
            if (k < self->attachedSize)
            {
                if (BenchCount(g, me))
                {
                    struct TcgFieldCard *t;
                    if (!ValidTarget(g, me, action->target, 1))
                        return TCG_INVALID;
                    t = &p->field[action->target];
                    t->attached[t->attachedSize++] = TcgPile_Remove(self->attached, &self->attachedSize, k);
                    TcgDuel_Event(g, TCG_EV_ATTACH, me, action->target, 0, p->cards[t->attached[t->attachedSize - 1]], 0);
                }
                else if (!y)
                    p->discard[p->discardSize++] = TcgPile_Remove(self->attached, &self->attachedSize, k);
            }
        }
        else if (ScriptIs(&o, "b1h"))
        {
            if (BenchCount(g, them))
            {
                if (!ValidTarget(g, them, action->target, 1))
                    return TCG_INVALID;
                if (g->scriptHeads)
                    TcgDuel_EffectDamage(g, them, action->target, x * (int)g->scriptHeads, 0);
            }
        }
        else if (ScriptIs(&o, "benchself"))
        {
            /* a Basic of its name from the deck onto the Bench */
            const struct TcgCard *me_card = TcgDuel_Field(g, me, sScriptSelf);
            unsigned k, room;
            for (room = 1; room < TCG_FIELD_SIZE && p->field[room].stackSize; room++)
                ;
            for (k = 0; me_card && room < TCG_FIELD_SIZE && k < p->deckSize; k++)
            {
                const struct TcgCard *c = CardAt(g, me, p->deck[k]);
                if (c->kind == TCG_POKEMON && (x || !c->stage) && !strcmp(c->name, me_card->name)) /* (:1 even an Evolution card) */
                {
                    memset(&p->field[room], 0, sizeof(p->field[room]));
                    p->field[room].stack[p->field[room].stackSize++] = TcgPile_Remove(p->deck, &p->deckSize, k);
                    p->field[room].enteredTurn = g->turn;
                    TcgDuel_Event(g, TCG_EV_PLAY, me, room, 0, c->id, 0);
                    break;
                }
            }
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "allnt"))
        {
            unsigned who;
            for (who = 0; who < 2; who++)
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                    if (g->players[who].field[s].stackSize && TcgDuel_Type(g, who, s) != (unsigned)x &&
                        (who != them || s || !blocked))
                    {
                        if (who == me && s == sScriptSelf)
                            TcgDuel_SelfDamage(g, (unsigned)y);
                        else
                            TcgDuel_EffectDamage(g, who, s, y, 0);
                    }
        }
        else if (ScriptIs(&o, "oalle"))
        {
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (op->field[s].stackSize && EnergyCards(g, them, s) && (s || !blocked))
                    TcgDuel_EffectDamage(g, them, s, x * (int)EnergyCards(g, them, s), 0);
        }
        else if (ScriptIs(&o, "odraw"))
        {
            /* (they may: as many as leave their deck a card) */
            unsigned n = op->deckSize > 1 ? op->deckSize - 1u : 0;
            n = n < (unsigned)x ? n : (unsigned)x;
            if (n)
                TcgDuel_Event(g, TCG_EV_DRAW, them, 0, TcgDuel_Draw(g, them, n), 0, 0);
        }
        /* the regional powers' ops ("it": the Pokemon using the power) */
        else if (ScriptIs(&o, "curea"))
        {
            p->field[0].condition = TCG_HEALTHY;
            p->field[0].poison = p->field[0].burned = 0;
            TcgDuel_Event(g, TCG_EV_STATUS, me, 0, TCG_HEALTHY, 0, 0);
        }
        else if (ScriptIs(&o, "bco"))
        {
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                if (op->field[s].stackSize)
                {
                    op->field[s].damage += (uint16_t)(x * 10);
                    TcgDuel_Event(g, TCG_EV_DAMAGE, them, s, (unsigned)x * 10, 0, 0);
                }
        }
        else if (ScriptIs(&o, "hit"))
        {
            /* x to the Defending Pokemon, Weakness and Resistance applied */
            int amount = x;
            unsigned type = TcgDuel_Type(g, me, sScriptSelf);
            if (foe->stackSize)
            {
                if (TcgDuel_Weakness(g, them, 0) == type)
                    amount *= 2;
                if (TcgDuel_Resistance(g, them, 0) == type)
                    amount -= 30;
                if (amount > 0)
                {
                    foe->damage += (uint16_t)amount;
                    TcgDuel_Event(g, TCG_EV_DAMAGE, them, 0, (unsigned)amount, 0, 0);
                }
            }
        }
        else if (ScriptIs(&o, "endturn"))
            g->turnOver = 1;
        else if (ScriptIs(&o, "selfhand") || ScriptIs(&o, "benchdeck") || ScriptIs(&o, "selfdeck"))
        {
            /* it (or the Benched Pokemon chosen) and its cards back to the hand / into the deck (selfhand:1 only its
             * Energy cards to the hand, :2 only its basic Energy: the rest discarded) */
            unsigned slot = ScriptIs(&o, "benchdeck") ? action->target : sScriptSelf, k;
            int toHand = ScriptIs(&o, "selfhand");
            struct TcgFieldCard *f;
            if (ScriptIs(&o, "benchdeck") && !ValidTarget(g, me, slot, 1))
                return TCG_INVALID;
            f = &p->field[slot];
            for (k = 0; k < f->stackSize + f->attachedSize; k++)
            {
                uint8_t card = k < f->stackSize ? f->stack[k] : f->attached[k - f->stackSize];
                const struct TcgCard *c = CardAt(g, me, card);
                if (toHand && k >= f->stackSize && x && (c->kind != TCG_ENERGY || (x == 2 && c->id > 6)))
                    p->discard[p->discardSize++] = card;
                else if (toHand)
                    p->hand[p->handSize++] = card;
                else
                    p->deck[p->deckSize++] = card;
            }
            memset(f, 0, sizeof(*f));
            if (!toHand)
                TcgDuel_Shuffle(g, me);
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
        }
        else if (ScriptIs(&o, "odeck"))
        {
            /* their Active and its cards shuffled into their deck (not one this attack Knocked Out) */
            const struct TcgCard *t = TcgDuel_Field(g, them, 0);
            if (!blocked && t && foe->damage < TcgDuel_MaxHp(g, them, 0))
            {
                unsigned k;
                for (k = 0; k < foe->stackSize + foe->attachedSize; k++)
                    op->deck[op->deckSize++] = k < foe->stackSize ? foe->stack[k] : foe->attached[k - foe->stackSize];
                memset(foe, 0, sizeof(*foe));
                TcgDuel_Shuffle(g, them);
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "ohand"))
        {
            /* their Active and its cards back to their hand (not one this attack Knocked Out) */
            const struct TcgCard *t = TcgDuel_Field(g, them, 0);
            if (!blocked && t && foe->damage < TcgDuel_MaxHp(g, them, 0))
            {
                unsigned k;
                for (k = 0; k < foe->stackSize + foe->attachedSize; k++)
                {
                    uint8_t card = k < foe->stackSize ? foe->stack[k] : foe->attached[k - foe->stackSize];
                    op->attachmentExpires[card] = 0;
                    op->hand[op->handSize++] = card;
                }
                memset(foe, 0, sizeof(*foe));
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "odisbench"))
        {
            /* the Baby or Basic Pokemon picked from their discard pile onto their Bench, with a damage counter */
            int at = action && action->selectionCount ? Picked(action, 0, op->discard, op->discardSize) : -1;
            unsigned room;
            for (room = 1; room < TCG_FIELD_SIZE && op->field[room].stackSize; room++)
                ;
            if (at >= 0 && room < TCG_FIELD_SIZE)
            {
                const struct TcgCard *c = CardAt(g, them, op->discard[at]);
                if (c->kind != TCG_POKEMON || c->stage)
                    return TCG_INVALID;
                memset(&op->field[room], 0, sizeof(op->field[room]));
                op->field[room].stack[op->field[room].stackSize++] = TcgPile_Remove(op->discard, &op->discardSize, (unsigned)at);
                op->field[room].enteredTurn = g->turn;
                op->field[room].damage = 10;
                TcgDuel_Event(g, TCG_EV_PLAY, them, room, 0, c->id, 0);
            }
        }
        else if (ScriptIs(&o, "ohbench"))
        {
            /* the Baby or Basic Pokemon picked from their hand onto their Bench, then switched in */
            int at = action && action->selectionCount ? Picked(action, 0, op->hand, op->handSize) : -1;
            unsigned room;
            for (room = 1; room < TCG_FIELD_SIZE && op->field[room].stackSize; room++)
                ;
            if (at >= 0 && room < TCG_FIELD_SIZE)
            {
                const struct TcgCard *c = CardAt(g, them, op->hand[at]);
                if (c->kind != TCG_POKEMON || c->stage)
                    return TCG_INVALID;
                memset(&op->field[room], 0, sizeof(op->field[room]));
                op->field[room].stack[op->field[room].stackSize++] = TcgPile_Remove(op->hand, &op->handSize, (unsigned)at);
                op->field[room].enteredTurn = g->turn;
                TcgDuel_Event(g, TCG_EV_PLAY, them, room, 0, c->id, 0);
                if (!blocked)
                    Switch(g, them, room);
            }
        }
        else if (ScriptIs(&o, "plunge"))
        {
            /* the Active's Energy onto it, then it switches in */
            unsigned k;
            for (k = p->field[0].attachedSize; k > 0; k--)
                if (CardAt(g, me, p->field[0].attached[k - 1])->kind == TCG_ENERGY)
                    self->attached[self->attachedSize++] = TcgPile_Remove(p->field[0].attached, &p->field[0].attachedSize, k - 1);
            if (sScriptSelf)
                Switch(g, me, sScriptSelf);
        }
        else if (ScriptIs(&o, "ehand") || ScriptIs(&o, "emove"))
        {
            /* the Energy card picked, from one of yours: to the hand (ehand); onto the one chosen (emove 0), Lightning
             * from the Bench onto it (emove 1), basic from the Bench onto the Active (emove 2), basic from one onto the
             * other chosen (emove 3: an attack's, nothing picked when there was nothing to move) */
            unsigned from = TCG_FIELD_SIZE, to = !x || x == 3 || x == 4 ? action->target : x == 1 ? sScriptSelf : 0, k;
            int at = -1;
            const struct TcgCard *e;
            for (k = 0; k < TCG_FIELD_SIZE && at < 0; k++)
                if ((at = Picked(action, 0, p->field[k].attached, p->field[k].attachedSize)) >= 0)
                    from = k;
            if (x == 3 && (!action || !action->selectionCount))
                continue;
            /* (emove 4, Electromagnetic Power: from one Magnemite, Magneton or Dark Magneton to another) */
            if (x == 4 && (from >= TCG_FIELD_SIZE || to >= TCG_FIELD_SIZE || !TcgDuel_Field(g, me, from) || !TcgDuel_Field(g, me, to) ||
                           !TcgFlow_InNameGroup(TcgDuel_Field(g, me, from)->name, 6) ||
                           !TcgFlow_InNameGroup(TcgDuel_Field(g, me, to)->name, 6)))
                return TCG_INVALID;
            if (at < 0 || !(e = CardAt(g, me, p->field[from].attached[at])) || e->kind != TCG_ENERGY)
                return TCG_INVALID;
            if (ScriptIs(&o, "ehand"))
                p->hand[p->handSize++] = TcgPile_Remove(p->field[from].attached, &p->field[from].attachedSize, (unsigned)at);
            else
            {
                if (to >= TCG_FIELD_SIZE || !p->field[to].stackSize || to == from || ((x == 1 || x == 2) && !from) ||
                    (x == 1 && e->id != TCG_CARD_LIGHTNING_ENERGY) || ((x == 2 || x == 3) && e->id > 6))
                    return TCG_INVALID;
                p->field[to].attached[p->field[to].attachedSize++] =
                    TcgPile_Remove(p->field[from].attached, &p->field[from].attachedSize, (unsigned)at);
                TcgDuel_Event(g, TCG_EV_ATTACH, me, to, 0, e->id, 0);
            }
        }
        else if (ScriptIs(&o, "wmove"))
        {
            /* a Water Energy from the Active onto the Benched Pokemon chosen */
            unsigned k;
            if (!ValidTarget(g, me, action->target, 1))
                return TCG_INVALID;
            for (k = 0; k < p->field[0].attachedSize; k++)
                if (CardAt(g, me, p->field[0].attached[k])->id == TCG_CARD_WATER_ENERGY)
                {
                    struct TcgFieldCard *t = &p->field[action->target];
                    t->attached[t->attachedSize++] = TcgPile_Remove(p->field[0].attached, &p->field[0].attachedSize, k);
                    TcgDuel_Event(g, TCG_EV_ATTACH, me, action->target, 0, TCG_CARD_WATER_ENERGY, 0);
                    break;
                }
        }
        else if (ScriptIs(&o, "attdto") || ScriptIs(&o, "handto"))
        {
            /* a basic Energy of type x from the discard pile (attdto) / the one picked in the hand (handto) onto the
             * Pokemon chosen (handto: Benched) */
            struct TcgFieldCard *t;
            int at = -1;
            unsigned k;
            if (!ValidTarget(g, me, action->target, ScriptIs(&o, "handto")))
                return TCG_INVALID;
            t = &p->field[action->target];
            if (ScriptIs(&o, "handto"))
            {
                at = Picked(action, 0, p->hand, p->handSize);
                if (at < 0 || CardAt(g, me, p->hand[at])->id > 6)
                    return TCG_INVALID;
                t->attached[t->attachedSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)at);
            }
            else
            {
                for (k = p->discardSize; k > 0 && at < 0; k--)
                    if (CardAt(g, me, p->discard[k - 1])->id <= 6 && CardAt(g, me, p->discard[k - 1])->type == (unsigned)x)
                        at = (int)k - 1;
                if (at < 0)
                    continue;
                t->attached[t->attachedSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)at);
            }
            TcgDuel_Event(g, TCG_EV_ATTACH, me, action->target, 0, p->cards[t->attached[t->attachedSize - 1]], 0);
        }
        else if (ScriptIs(&o, "ohdeck"))
        {
            /* the card picked from their hand (looked at) into their deck (x 1: a Trainer, 2: an Energy card) */
            int at = Picked(action, 0, op->hand, op->handSize);
            if (at >= 0 && ((x == 1 && CardAt(g, them, op->hand[at])->kind != TCG_TRAINER) ||
                            (x == 2 && CardAt(g, them, op->hand[at])->kind != TCG_ENERGY)))
                return TCG_INVALID;
            if (at < 0 && x)
            {
                /* (after a coin, never looked at before it: the one that costs them most, a Supporter / Special
                 * Energy first) */
                unsigned k;
                for (k = 0; k < op->handSize; k++)
                {
                    const struct TcgCard *c = CardAt(g, them, op->hand[k]);
                    if (c->kind != (x == 1 ? TCG_TRAINER : TCG_ENERGY))
                        continue;
                    if (at < 0 || (x == 1 ? (c->flags & TCG_CF_SUPPORTER) != 0 : c->id > 6))
                        at = (int)k;
                }
                if (at >= 0)
                    TcgDuel_Log(g, "Shuffled into their deck: %s.", CardAt(g, them, op->hand[at])->name);
            }
            if (at >= 0)
            {
                op->deck[op->deckSize++] = TcgPile_Remove(op->hand, &op->handSize, (unsigned)at);
                TcgDuel_Shuffle(g, them);
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "ohtmtool"))
        {
            /* every Technical Machine and Pokemon Tool in their hand into their deck */
            unsigned k, n = 0;
            for (k = op->handSize; k > 0; k--)
                if (CardAt(g, them, op->hand[k - 1])->kind == TCG_TRAINER &&
                    (CardAt(g, them, op->hand[k - 1])->flags & (TCG_CF_TM | TCG_CF_TOOL)))
                    op->deck[op->deckSize++] = TcgPile_Remove(op->hand, &op->handSize, k - 1), n++;
            TcgDuel_Log(g, "%u Technical Machine and Tool cards shuffled into their deck.", n);
            if (n)
            {
                TcgDuel_Shuffle(g, them);
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "attdn"))
        {
            /* a Metal (x 0) / Darkness (1) Energy card from the discard pile onto it */
            unsigned k;
            for (k = p->discardSize; k > 0; k--)
            {
                const struct TcgCard *c = CardAt(g, me, p->discard[k - 1]);
                if (c->kind == TCG_ENERGY && c->id > 6 && strstr(c->name, x ? "Darkness" : "Metal"))
                {
                    self->attached[self->attachedSize++] = TcgPile_Remove(p->discard, &p->discardSize, k - 1);
                    TcgDuel_Event(g, TCG_EV_ATTACH, me, sScriptSelf, 0, p->cards[self->attached[self->attachedSize - 1]], 0);
                    break;
                }
            }
        }
        else if (ScriptIs(&o, "chosc"))
        {
            /* the Special Condition chosen (opt:1 with Burned, opt:2 without) */
            static const char *const five[5] = {"slp", "brn", "cnf", "par", "psn"}, *const four[4] = {"slp", "cnf", "par", "psn"},
                                     *const sev[3] = {"slp", "cnf", "psn"}, *const eight[3] = {"slp", "cnf", "par"};
            const char *const *list = x == 1 ? five : x == 7 ? sev : x == 8 ? eight : four;
            if (!blocked && foe->stackSize && sScriptOption < (x == 1 ? 5u : x >= 7 ? 3u : 4u))
            {
                ScriptStatus(foe, list[sScriptOption]);
                TcgDuel_Event(g, TCG_EV_STATUS, them, 0, foe->condition, 0, foe->poison);
            }
        }
        else if (ScriptIs(&o, "dso"))
        {
            /* the chosen number of its Energy cards of type x discarded */
            unsigned k, n = sScriptOption;
            for (k = self->attachedSize; k > 0 && n; k--)
            {
                const struct TcgCard *e = CardAt(g, me, self->attached[k - 1]);
                if (e->kind == TCG_ENERGY && (x >= TCG_TYPES || e->type == (unsigned)x))
                {
                    p->discard[p->discardSize++] = TcgPile_Remove(self->attached, &self->attachedSize, k - 1);
                    n--;
                }
            }
        }
        else if (ScriptIs(&o, "order3"))
        {
            /* the top cards of the deck chosen (up to 3) in the order picked, the first on top */
            unsigned whose = me ^ (sScriptOption ? 1u : 0u), top, i, k;
            struct TcgPlayer *d = &g->players[whose];
            top = d->deckSize < 3 ? d->deckSize : 3;
            if (action && action->selectionCount)
            {
                if (action->selectionCount != top)
                    return TCG_INVALID;
                for (i = 0; i < top; i++)
                {
                    for (k = 0; k < top && d->deck[d->deckSize - 1 - k] != action->selections[i]; k++)
                        ;
                    if (k == top)
                        return TCG_INVALID;
                    for (k = 0; k < i; k++)
                        if (action->selections[k] == action->selections[i])
                            return TCG_INVALID;
                }
                for (i = 0; i < top; i++)
                    d->deck[d->deckSize - 1 - i] = action->selections[i];
            }
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, whose, 0, 0, 0, 0);
        }
        else if (ScriptIs(&o, "b1e"))
        {
            if (ValidTarget(g, them, action->target, 1))
                TcgDuel_EffectDamage(g, them, action->target, y * (int)ScriptEnergyOf(g, me, sScriptSelf, x), 0);
        }
        else if (ScriptIs(&o, "ectr"))
        {
            unsigned t = action->target;
            unsigned n = ValidTarget(g, them, t, 0) && (t || !blocked) ? EnergyCards(g, them, t) : 0;
            if (n)
            {
                op->field[t].damage += (uint16_t)(10 * n);
                TcgDuel_Event(g, TCG_EV_DAMAGE, them, t, 10 * n, 0, 0);
            }
        }
        else if (ScriptIs(&o, "botd"))
        {
            unsigned type = foe->stackSize ? TcgDuel_Type(g, them, 0) : TCG_NONE;
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                if (op->field[s].stackSize && TcgDuel_Type(g, them, s) == type)
                    TcgDuel_EffectDamage(g, them, s, x, 0);
        }
        else if (ScriptIs(&o, "luremay"))
        {
            unsigned t = action->target;
            const struct TcgCard *d = TcgDuel_Field(g, them, 0);
            if (t && ValidTarget(g, them, t, 1) && d && foe->damage < TcgDuel_MaxHp(g, them, 0) && !blocked)
            {
                struct TcgFieldCard tmp = op->field[0];
                op->field[0] = op->field[t];
                op->field[t] = tmp;
                TcgDuel_Event(g, TCG_EV_SWITCH, them, t, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "bbc"))
        {
            unsigned who;
            for (who = 0; who < 2; who++)
                for (s = 1; s < TCG_FIELD_SIZE; s++)
                    if (g->players[who].field[s].stackSize && TcgDuel_Coin(g, me))
                        TcgDuel_EffectDamage(g, who, s, x, 0);
        }
        else if (ScriptIs(&o, "dte"))
        {
            unsigned t = action->target;
            int at = ValidTarget(g, them, t, 0) && (t || !blocked)
                         ? Picked(action, 0, op->field[t].attached, op->field[t].attachedSize) : -1;
            /* (dte: an Energy discarded; dte:1 a Special Energy, dte:2 a Trainer, shuffled into their deck) */
            const struct TcgCard *c = at >= 0 ? CardAt(g, them, op->field[t].attached[at]) : NULL;
            if (c && (x == 2 ? c->kind == TCG_TRAINER : c->kind == TCG_ENERGY && (x != 1 || c->id > 6)))
            {
                op->attachmentExpires[op->field[t].attached[at]] = 0;
                if (x)
                {
                    op->deck[op->deckSize++] = TcgPile_Remove(op->field[t].attached, &op->field[t].attachedSize, (unsigned)at);
                    TcgDuel_Shuffle(g, them);
                    TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
                }
                else
                    op->discard[op->discardSize++] = TcgPile_Remove(op->field[t].attached, &op->field[t].attachedSize, (unsigned)at);
            }
        }
        else if (ScriptIs(&o, "octrd") || ScriptIs(&o, "mvctr"))
        {
            for (s = 0; s < TCG_FIELD_SIZE; s++)
            {
                struct TcgFieldCard *f = ScriptIs(&o, "octrd") ? &op->field[s] : &p->field[s];
                if (!f->stackSize || f->damage < 10 || (ScriptIs(&o, "octrd") && !s && blocked))
                    continue;
                if (ScriptIs(&o, "octrd"))
                {
                    f->damage += 10;
                    TcgDuel_Event(g, TCG_EV_DAMAGE, them, s, 10, 0, 0);
                }
                else if (foe->stackSize && !blocked)
                {
                    f->damage -= 10;
                    TcgDuel_Event(g, TCG_EV_HEAL, me, s, 10, 0, 0);
                    foe->damage += 10;
                    TcgDuel_Event(g, TCG_EV_DAMAGE, them, 0, 10, 0, 0);
                }
            }
        }
        else if (ScriptIs(&o, "healt"))
        {
            if (!ValidTarget(g, me, action->target, 0))
                return TCG_INVALID;
            if (p->field[action->target].damage)
            {
                unsigned h = x && 10u * (unsigned)x < p->field[action->target].damage ? 10u * (unsigned)x
                                                                                      : p->field[action->target].damage;
                TcgDuel_Event(g, TCG_EV_HEAL, me, action->target, h, 0, 0);
                p->field[action->target].damage -= (uint16_t)h;
            }
        }
        else if (ScriptIs(&o, "bpick"))
        {
            unsigned hit = 0, used[TCG_FIELD_SIZE] = {0}, k;
            for (k = 0; action && k < action->selectionCount && hit < 2; k++)
                for (s = 1; s < TCG_FIELD_SIZE; s++)
                    if (op->field[s].stackSize && !used[s] && op->field[s].stack[op->field[s].stackSize - 1] == action->selections[k])
                    {
                        used[s] = 1, hit++;
                        TcgDuel_EffectDamage(g, them, s, x, 0);
                        break;
                    }
            while (hit < 2)
            {
                unsigned best = TCG_NONE;
                for (s = 1; s < TCG_FIELD_SIZE; s++)
                    if (op->field[s].stackSize && !used[s] && (best == TCG_NONE || HpLeft(g, them, s) < HpLeft(g, them, best)))
                        best = s;
                if (best == TCG_NONE)
                    break;
                used[best] = 1, hit++;
                TcgDuel_EffectDamage(g, them, best, x, 0);
            }
        }
        else if (ScriptIs(&o, "m1"))
        {
            if (BenchCount(g, me))
            {
                if (!ValidTarget(g, me, action->target, 1))
                    return TCG_INVALID;
                TcgDuel_EffectDamage(g, me, action->target, x, 0);
            }
        }
        else if (ScriptIs(&o, "disdeck"))
        {
            unsigned k, n = 0;
            for (k = p->discardSize; k > 0; k--)
                if (CardAt(g, me, p->discard[k - 1])->kind == TCG_ENERGY && CardAt(g, me, p->discard[k - 1])->type == (unsigned)x)
                {
                    p->deck[p->deckSize++] = TcgPile_Remove(p->discard, &p->discardSize, k - 1);
                    n++;
                }
            if (n)
            {
                TcgDuel_Shuffle(g, me);
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "distop"))
        {
            struct TcgPlayer *q = x ? op : p;
            int at = action ? Picked(action, 0, q->discard, q->discardSize) : -1;
            if (at >= 0)
            {
                q->deck[q->deckSize++] = TcgPile_Remove(q->discard, &q->discardSize, (unsigned)at);
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, x ? them : me, 0, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "otop"))
        {
            if (op->deckSize)
            {
                const struct TcgCard *c = CardAt(g, them, op->deck[op->deckSize - 1]);
                TcgDuel_Log(g, "Their top card: %s.", c->name);
                if (c->kind == TCG_TRAINER)
                    op->discard[op->discardSize++] = op->deck[--op->deckSize];
                else if (!x)
                    op->hand[op->handSize++] = op->deck[--op->deckSize];
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "evdeck"))
        {
            /* the card picked from the deck that evolves from it: onto it (x 0: evolving it) or into the hand (1) */
            const struct TcgCard *it = TcgDuel_Field(g, me, sScriptSelf);
            int at = Picked(action, 0, p->deck, p->deckSize);
            if (at >= 0 && !EvolvesFrom(CardAt(g, me, p->deck[at]), it))
                return TCG_INVALID;
            if (at >= 0 && !x)
                Evolve(g, me, sScriptSelf, (unsigned)at);
            else if (at >= 0)
            {
                TcgDuel_Log(g, "Shown: %s.", CardAt(g, me, p->deck[at])->name);
                p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
            }
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "srche"))
        {
            /* the basic Energy cards picked from the deck into the hand: up to x (0: one per heads) */
            unsigned k, max = x ? (unsigned)x : g->scriptHeads, n = action ? action->selectionCount : 0;
            for (k = 0; k < n; k++)
            {
                int at = Picked(action, k, p->deck, p->deckSize);
                if (at < 0 || CardAt(g, me, p->deck[at])->kind != TCG_ENERGY || CardAt(g, me, p->deck[at])->id > 6)
                    return TCG_INVALID;
            }
            for (k = 0; k < n && k < max; k++)
            {
                int at = Picked(action, k, p->deck, p->deckSize);
                TcgDuel_Log(g, "Shown: %s.", CardAt(g, me, p->deck[at])->name);
                p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
            }
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "top3h"))
        {
            /* one of the top 3 of the deck into the hand, the rest shuffled in */
            int at = Picked(action, 0, p->deck, p->deckSize);
            if (at >= 0 && (unsigned)at + 3 < p->deckSize)
                return TCG_INVALID;
            if (at >= 0)
                p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "hdraw"))
        {
            /* up to x cards picked from the hand discarded, then as many drawn */
            unsigned k, n = action ? action->selectionCount : 0;
            if (n > (unsigned)x)
                return TCG_INVALID;
            for (k = 0; k < n; k++)
                if (Picked(action, k, p->hand, p->handSize) < 0)
                    return TCG_INVALID;
            for (k = 0; k < n; k++)
                p->discard[p->discardSize++] = TcgPile_Remove(p->hand, &p->handSize,
                                                              (unsigned)Picked(action, k, p->hand, p->handSize));
            if (n)
                TcgDuel_Event(g, TCG_EV_DRAW, me, sScriptSelf, TcgDuel_Draw(g, me, n), 0, 0);
        }
        else if (ScriptIs(&o, "stuck") || ScriptIs(&o, "blindall") || ScriptIs(&o, "noatall"))
        {
            /* until it's Benched or evolves: can't retreat (stuck:2 while it's the Defending's opponent), a coin to
             * attack, can't attack */
            if (!blocked && foe->stackSize)
            {
                if (ScriptIs(&o, "stuck"))
                {
                    foe->stuck = (uint8_t)(x == 2 ? 2 : 1);
                    foe->stuckBy = (uint8_t)(self->stack[self->stackSize - 1] + 1);
                }
                else if (ScriptIs(&o, "blindall"))
                    foe->blindAll = 1;
                else
                    foe->noAttackAll = 1;
            }
        }
        else if (ScriptIs(&o, "dedeck"))
        {
            /* up to x of the Defending's Energy cards (the ones picked, else the first) shuffled into their deck */
            unsigned k, n = 0;
            for (k = 0; k < (unsigned)x && !blocked && foe->stackSize; k++)
            {
                int at = Picked(action, k, foe->attached, foe->attachedSize);
                uint8_t list[TCG_DECK_SIZE];
                unsigned have = Attached(g, them, 0, TCG_ENERGY, list);
                if (!have)
                    break;
                if (at < 0 || CardAt(g, them, foe->attached[at])->kind != TCG_ENERGY)
                    at = list[0];
                op->deck[op->deckSize++] = TcgPile_Remove(foe->attached, &foe->attachedSize, (unsigned)at);
                n++;
            }
            if (n)
            {
                TcgDuel_Shuffle(g, them);
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "odevb"))
        {
            /* the chosen Evolved Pokemon of theirs: its top card to the bottom of their deck (devolving it) */
            struct TcgFieldCard *f = &op->field[action->target];
            unsigned k;
            if (!ValidTarget(g, them, action->target, 0) || f->stackSize < 2)
            {
                for (k = 0; k < TCG_FIELD_SIZE && op->field[k].stackSize < 2; k++)
                    ;
                if (k < TCG_FIELD_SIZE)
                    return TCG_INVALID;
            }
            else if (action->target || !blocked)
            {
                if (x) /* (odevb:1 shuffled into their deck) */
                {
                    op->deck[op->deckSize++] = f->stack[--f->stackSize];
                    TcgDuel_Shuffle(g, them);
                }
                else
                {
                    for (k = op->deckSize; k > 0; k--)
                        op->deck[k] = op->deck[k - 1];
                    op->deck[0] = f->stack[--f->stackSize];
                    op->deckSize++;
                }
                f->condition = TCG_HEALTHY;
                f->poison = f->burned = 0;
                f->noAttack = f->noRetreat = f->weaken = f->guard = f->charged = f->effectBy = 0;
                f->stuck = f->blindAll = f->noAttackAll = 0;
                TcgDuel_Event(g, TCG_EV_DEVOLVE, them, action->target, 0, op->cards[f->stack[f->stackSize - 1]], 0);
                TcgDuel_Log(g, "%s devolved.", CardAt(g, them, f->stack[f->stackSize - 1])->name);
            }
        }
        else if (ScriptIs(&o, "evactive"))
        {
            /* the card picked from the deck that evolves from the Active, onto it */
            int at = Picked(action, 0, p->deck, p->deckSize);
            if (at >= 0 && !EvolvesFrom(CardAt(g, me, p->deck[at]), TcgDuel_Field(g, me, 0)))
                return TCG_INVALID;
            if (at >= 0)
                Evolve(g, me, 0, (unsigned)at);
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "burnall"))
        {
            /* every one of yours: its Energy all Fire for the rest of the turn (Energy Burn's way) */
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (p->field[s].stackSize)
                    p->field[s].burn = 1;
        }
        else if (ScriptIs(&o, "shufboth"))
        {
            TcgDuel_Shuffle(g, me);
            TcgDuel_Shuffle(g, them);
        }
        else if (ScriptIs(&o, "hredraw"))
        {
            /* the hand shuffled into the deck, then as many cards drawn */
            unsigned n = p->handSize;
            while (p->handSize)
                p->deck[p->deckSize++] = p->hand[--p->handSize];
            TcgDuel_Shuffle(g, me);
            TcgDuel_Event(g, TCG_EV_DRAW, me, sScriptSelf, TcgDuel_Draw(g, me, n), 0, 0);
        }
        else if (ScriptIs(&o, "acthand"))
        {
            /* the Active and its cards back to the hand */
            struct TcgFieldCard *f = &p->field[0];
            unsigned k;
            for (k = 0; k < f->stackSize + f->attachedSize; k++)
                p->hand[p->handSize++] = k < f->stackSize ? f->stack[k] : f->attached[k - f->stackSize];
            memset(f, 0, sizeof(*f));
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
        }
        else if (ScriptIs(&o, "distr"))
        {
            /* the Trainer picked from the discard pile into the hand */
            int at = Picked(action, 0, p->discard, p->discardSize);
            if (at >= 0 && CardAt(g, me, p->discard[at])->kind != TCG_TRAINER)
                return TCG_INVALID;
            if (at >= 0)
                p->hand[p->handSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)at);
        }
        else if (ScriptIs(&o, "barunown"))
        {
            /* every one of yours with Unown in its name: no effect of attacks, damage too, until your next turn */
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (TcgDuel_Field(g, me, s) && strstr(TcgDuel_Field(g, me, s)->name, "Unown"))
                    p->field[s].barrier = 2;
        }
        else if (ScriptIs(&o, "evbench"))
        {
            /* the card picked from the deck that evolves from the Benched Pokemon of yours chosen, onto it */
            int at = Picked(action, 0, p->deck, p->deckSize);
            if (at >= 0 && (!ValidTarget(g, me, action->target, 1) ||
                            !EvolvesFrom(CardAt(g, me, p->deck[at]), TcgDuel_Field(g, me, action->target))))
                return TCG_INVALID;
            if (at >= 0)
                Evolve(g, me, action->target, (unsigned)at);
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "evhand"))
        {
            /* the card picked from the hand that evolves from it, onto it (evolving it) */
            int at = Picked(action, 0, p->hand, p->handSize);
            if (at >= 0 && !EvolvesFrom(CardAt(g, me, p->hand[at]), TcgDuel_Field(g, me, sScriptSelf)))
                return TCG_INVALID;
            if (at >= 0)
            {
                /* (Evolve takes from the deck: the card put on top of it first) */
                p->deck[p->deckSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)at);
                Evolve(g, me, sScriptSelf, p->deckSize - 1);
            }
        }
        else if (ScriptIs(&o, "dised") || ScriptIs(&o, "distrdeck"))
        {
            /* the basic Energy (dised) / Trainer (distrdeck) picked from the discard pile shuffled into the deck */
            int at = Picked(action, 0, p->discard, p->discardSize);
            const struct TcgCard *c = at >= 0 ? CardAt(g, me, p->discard[at]) : NULL;
            if (c && (ScriptIs(&o, "dised") ? c->kind != TCG_ENERGY || c->id > 6 : c->kind != TCG_TRAINER))
                return TCG_INVALID;
            if (c)
            {
                TcgDuel_Log(g, "Shown: %s.", c->name);
                p->deck[p->deckSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)at);
                TcgDuel_Shuffle(g, me);
            }
        }
        else if (ScriptIs(&o, "srchnm"))
        {
            /* a basic Energy of type x from the deck onto it for each of yours in play with its name */
            const struct TcgCard *it = TcgDuel_Field(g, me, sScriptSelf);
            unsigned n = 0, k, got = 0;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                n += it && TcgDuel_Field(g, me, s) && !strcmp(TcgDuel_Field(g, me, s)->name, it->name);
            for (k = p->deckSize; k > 0 && got < n; k--)
            {
                const struct TcgCard *c = CardAt(g, me, p->deck[k - 1]);
                if (c->kind != TCG_ENERGY || c->id > 6 || c->type != (unsigned)x)
                    continue;
                self->attached[self->attachedSize++] = TcgPile_Remove(p->deck, &p->deckSize, k - 1);
                TcgDuel_Event(g, TCG_EV_ATTACH, me, sScriptSelf, 0, p->cards[self->attached[self->attachedSize - 1]], 0);
                got++;
            }
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "healte"))
        {
            /* x counters off the one of yours chosen with Energy on it */
            struct TcgFieldCard *t = &p->field[action->target];
            unsigned h;
            if (!ValidTarget(g, me, action->target, 0) || !EnergyCards(g, me, action->target))
                return TCG_INVALID;
            h = 10u * (unsigned)x < t->damage ? 10u * (unsigned)x : t->damage;
            if (h)
            {
                t->damage -= (uint16_t)h;
                TcgDuel_Event(g, TCG_EV_HEAL, me, action->target, h, 0, 0);
            }
        }
        else if (ScriptIs(&o, "healbd"))
        {
            /* every counter off the Benched one of yours chosen, and its Energy cards discarded */
            struct TcgFieldCard *t = &p->field[action->target];
            unsigned k;
            if (!BenchCount(g, me))
                ;
            else if (!ValidTarget(g, me, action->target, 1))
                return TCG_INVALID;
            else
            {
                if (t->damage)
                    TcgDuel_Event(g, TCG_EV_HEAL, me, action->target, t->damage, 0, 0);
                t->damage = 0;
                for (k = t->attachedSize; k > 0; k--)
                    if (CardAt(g, me, t->attached[k - 1])->kind == TCG_ENERGY)
                        p->discard[p->discardSize++] = TcgPile_Remove(t->attached, &t->attachedSize, k - 1);
            }
        }
        else if (ScriptIs(&o, "noatt"))
        {
            if (!blocked)
                op->noAttachActive = (uint16_t)(g->turn + 1);
        }
        else if (ScriptIs(&o, "noevo"))
        {
            if (!blocked && foe->stackSize)
                foe->noEvolveUntil = (uint16_t)(g->turn + 1);
        }
        else if (ScriptIs(&o, "deall"))
        {
            /* an Energy card of each of theirs that has any discarded (a Special one first) */
            for (s = 0; s < TCG_FIELD_SIZE; s++)
            {
                struct TcgFieldCard *t = &op->field[s];
                uint8_t list[TCG_DECK_SIZE];
                unsigned have = t->stackSize && (s || !blocked) ? Attached(g, them, s, TCG_ENERGY, list) : 0, k, pick;
                if (!have)
                    continue;
                for (k = 0, pick = list[0]; k < have; k++)
                    if (CardAt(g, them, t->attached[list[k]])->id > 6)
                        pick = list[k];
                op->discard[op->discardSize++] = TcgPile_Remove(t->attached, &t->attachedSize, pick);
            }
        }
        else if (ScriptIs(&o, "noconds"))
            self->condImmune = x ? 0xFFFF /* (as long as it's the Active Pokemon) */ : (uint16_t)(g->turn + 1);
        else if (ScriptIs(&o, "rt1"))
        {
            if (!blocked && foe->stackSize)
            {
                foe->retreatPlus = (uint8_t)(x ? x : 1);
                foe->retreatUntil = (uint16_t)(g->turn + 1);
            }
        }
        else if (ScriptIs(&o, "hpset"))
        {
            /* its maximum HP is now x, and its attack y does z base damage, until it's Benched (Giant Growth) */
            self->hpSet = (uint8_t)x;
            self->boostTurn = 0xFFFF;
            self->boostAttack = (uint8_t)(y + 1);
            self->boostValue = (uint8_t)z;
        }
        else if (ScriptIs(&o, "boost"))
        {
            /* next turn its attack x does y base damage (0: double) */
            self->boostTurn = (uint16_t)(g->turn + 2);
            self->boostAttack = (uint8_t)(x + 1);
            self->boostValue = (uint8_t)y;
        }
        else if (ScriptIs(&o, "dspe") || ScriptIs(&o, "ddeh"))
        {
            /* a Special Energy of the Defending's picked (dspe), or one of its Energy per heads (ddeh) discarded */
            unsigned k, n = ScriptIs(&o, "dspe") ? 1 : g->scriptHeads;
            for (k = 0; k < n && !blocked && foe->stackSize; k++)
            {
                int at = Picked(action, k, foe->attached, foe->attachedSize);
                uint8_t list[TCG_DECK_SIZE];
                unsigned i, have = Attached(g, them, 0, TCG_ENERGY, list), pick;
                if (ScriptIs(&o, "dspe"))
                {
                    for (i = 0, pick = TCG_DECK_SIZE; i < have; i++)
                        if (CardAt(g, them, foe->attached[list[i]])->id > 6)
                            pick = list[i];
                    if (pick == TCG_DECK_SIZE)
                        break;
                    if (at >= 0 && CardAt(g, them, foe->attached[at])->kind == TCG_ENERGY &&
                        CardAt(g, them, foe->attached[at])->id > 6)
                        pick = (unsigned)at;
                }
                else
                {
                    if (!have)
                        break;
                    pick = at >= 0 && CardAt(g, them, foe->attached[at])->kind == TCG_ENERGY ? (unsigned)at : list[0];
                }
                op->discard[op->discardSize++] = TcgPile_Remove(foe->attached, &foe->attachedSize, pick);
            }
        }
        else if (ScriptIs(&o, "swapdmg"))
        {
            if (foe->stackSize && !blocked)
            {
                uint16_t mine = self->damage;
                self->damage = foe->damage;
                foe->damage = mine;
                TcgDuel_Event(g, TCG_EV_DAMAGE, them, 0, 0, 0, 0);
                TcgDuel_Event(g, TCG_EV_DAMAGE, me, sScriptSelf, 0, 0, 0);
            }
        }
        else if (ScriptIs(&o, "moon"))
        {
            /* the hand card picked onto the deck, then the basic Energy picked from the deck into the hand */
            int at = action ? Picked(action, 0, p->hand, p->handSize) : -1, from = -1;
            if (at < 0)
                return TCG_INVALID;
            if (action->selectionCount > 1)
            {
                from = Picked(action, 1, p->deck, p->deckSize);
                if (from < 0 || CardAt(g, me, p->deck[from])->id > 6)
                    return TCG_INVALID;
                TcgDuel_Log(g, "Shown: %s.", CardAt(g, me, p->deck[from])->name);
                p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)from);
            }
            at = Picked(action, 0, p->hand, p->handSize);
            p->deck[p->deckSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)at);
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "gaze"))
        {
            if (!ValidTarget(g, them, action->target, 1))
                return TCG_INVALID;
            op->field[action->target].powerOffUntil = g->turn;
        }
        else if (ScriptIs(&o, "dcn") || ScriptIs(&o, "dcself") || ScriptIs(&o, "dcmine") || ScriptIs(&o, "any1c"))
        {
            unsigned t = ScriptIs(&o, "any1c") ? action->target : 0, n = (unsigned)x;
            if (ScriptIs(&o, "dcself"))
                n = self->damage / 10;
            if (ScriptIs(&o, "dcmine"))
                for (n = 0, s = 0; s < TCG_FIELD_SIZE; s++)
                    n += p->field[s].stackSize && p->field[s].damage;
            if (ValidTarget(g, them, t, 0) && (t || !blocked) && n)
            {
                op->field[t].damage += (uint16_t)(10 * n);
                TcgDuel_Event(g, TCG_EV_DAMAGE, them, t, 10 * n, 0, 0);
            }
        }
        else if (ScriptIs(&o, "ctrh"))
        {
            if (self->stackSize && g->scriptHeads)
            {
                self->damage += (uint16_t)(10 * x * (int)g->scriptHeads);
                TcgDuel_Event(g, TCG_EV_DAMAGE, me, sScriptSelf, 10u * (unsigned)x * g->scriptHeads, 0, 0);
            }
        }
        else if (ScriptIs(&o, "healen") || ScriptIs(&o, "healtyp"))
        {
            unsigned type = TcgDuel_Type(g, me, sScriptSelf);
            for (s = 0; s < TCG_FIELD_SIZE; s++)
            {
                struct TcgFieldCard *f = &p->field[s];
                unsigned h;
                if (!f->stackSize || !f->damage || (s != sScriptSelf && (ScriptIs(&o, "healen") || TcgDuel_Type(g, me, s) != type)))
                    continue;
                h = 10u * (unsigned)x * (ScriptIs(&o, "healen") ? EnergyCards(g, me, s) : 1u);
                h = h < f->damage ? h : f->damage;
                if (h)
                {
                    f->damage -= (uint16_t)h;
                    TcgDuel_Event(g, TCG_EV_HEAL, me, s, h, 0, 0);
                }
            }
        }
        else if (ScriptIs(&o, "bshield"))
            p->benchShieldUntil = (uint16_t)(g->turn + 1);
        else if (ScriptIs(&o, "spike"))
        {
            int at = action && action->selectionCount ? Picked(action, 0, p->deck, p->deckSize) : -1;
            if (at >= 0)
            {
                struct TcgFieldCard *t;
                const struct TcgCard *e = CardAt(g, me, p->deck[at]), *to = TcgDuel_Field(g, me, action->target);
                /* (spike:T,K: an Energy of type T; K 1 onto a Pokemon with Unown in its name, 2 onto a Benched one) */
                if (e->kind != TCG_ENERGY || e->id > 6 || !ValidTarget(g, me, action->target, o.args > 1 && o.arg[1] == 2) ||
                    (o.args && o.arg[0] != 15 && e->type != (unsigned)o.arg[0]) ||
                    (o.args > 1 && o.arg[1] == 1 && (!to || !strstr(to->name, "Unown"))))
                    return TCG_INVALID;
                t = &p->field[action->target];
                t->attached[t->attachedSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
                TcgDuel_Event(g, TCG_EV_ATTACH, me, action->target, 0, p->cards[t->attached[t->attachedSize - 1]], 0);
            }
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "tailwind"))
            p->turnBuffs |= TCG_BUFF_TAILWIND;
        else if (ScriptIs(&o, "pull"))
        {
            /* Soak Up: the Grass Energy cards picked (up to 2) from your other Pokemon onto it */
            unsigned k, s;
            if (!action || !action->selectionCount || action->selectionCount > 2)
                return TCG_INVALID;
            for (k = 0; k < action->selectionCount; k++)
            {
                int at = -1;
                for (s = 0; s < TCG_FIELD_SIZE && at < 0; s++)
                    if (s != sScriptSelf)
                        for (at = (int)p->field[s].attachedSize - 1; at >= 0 && p->field[s].attached[at] != action->selections[k]; at--)
                            ;
                if (at < 0 || CardAt(g, me, p->field[s - 1].attached[at])->id != TCG_CARD_GRASS_ENERGY)
                    return TCG_INVALID;
                self->attached[self->attachedSize++] = TcgPile_Remove(p->field[s - 1].attached, &p->field[s - 1].attachedSize, (unsigned)at);
                TcgDuel_Event(g, TCG_EV_ATTACH, me, sScriptSelf, 0, TCG_CARD_GRASS_ENERGY, 0);
            }
        }
        else if (ScriptIs(&o, "ctrin"))
        {
            /* [Bear]: a damage counter from the Unown chosen onto it */
            const struct TcgCard *from = TcgDuel_Field(g, me, action ? action->target : 0);
            if (!action || action->target == sScriptSelf || !from || !strstr(from->name, "Unown") ||
                !p->field[action->target].damage || self->damage + 10u >= TcgDuel_MaxHp(g, me, sScriptSelf))
                return TCG_INVALID;
            p->field[action->target].damage -= 10;
            self->damage += 10;
            TcgDuel_Event(g, TCG_EV_HEAL, me, action->target, 10, 0, 0);
            TcgDuel_Event(g, TCG_EV_DAMAGE, me, sScriptSelf, 10, 0, 0);
        }
        else if (ScriptIs(&o, "mshift"))
        {
            /* Miracle Shift: the basic Energy picked on one of yours discarded, then the one picked from the discard
             * pile (none: that same card) attached to that Pokemon */
            unsigned s;
            int at = -1, back;
            for (s = 0; s < TCG_FIELD_SIZE && at < 0; s++)
                at = Picked(action, 0, p->field[s].attached, p->field[s].attachedSize);
            if (at < 0 || CardAt(g, me, p->field[s - 1].attached[at])->id > 6)
                return TCG_INVALID;
            s--;
            p->discard[p->discardSize++] = TcgPile_Remove(p->field[s].attached, &p->field[s].attachedSize, (unsigned)at);
            back = action->selectionCount > 1 ? Picked(action, 1, p->discard, p->discardSize) : (int)p->discardSize - 1;
            if (back < 0 || CardAt(g, me, p->discard[back])->kind != TCG_ENERGY || CardAt(g, me, p->discard[back])->id > 6)
                return TCG_INVALID;
            p->field[s].attached[p->field[s].attachedSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)back);
            TcgDuel_Event(g, TCG_EV_ATTACH, me, s, 0, p->cards[p->field[s].attached[p->field[s].attachedSize - 1]], 0);
        }
        else if (ScriptIs(&o, "recon"))
        {
            /* Reconstruction: the Omanyte or Kabuto picked from the deck into the hand, then the basic Energy picked in
             * the hand into the deck */
            int at = action && action->selectionCount > 1 ? Picked(action, 0, p->deck, p->deckSize) : -1, e;
            const struct TcgCard *c = at >= 0 ? CardAt(g, me, p->deck[at]) : NULL;
            if (at >= 0)
            {
                if (!c || c->kind != TCG_POKEMON || (strcmp(c->name, "Omanyte") && strcmp(c->name, "Kabuto")))
                    return TCG_INVALID;
                TcgDuel_Log(g, "Shown: %s.", c->name);
                p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
            }
            e = action ? Picked(action, action->selectionCount - 1, p->hand, p->handSize) : -1;
            if (e < 0 || CardAt(g, me, p->hand[e])->kind != TCG_ENERGY || CardAt(g, me, p->hand[e])->id > 6)
                return TCG_INVALID;
            p->deck[p->deckSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)e);
            TcgDuel_Shuffle(g, me);
        }
        else if (ScriptIs(&o, "order"))
        {
            /* Terraforming: the top x of your deck in the order picked, the first on top */
            unsigned n = p->deckSize < (unsigned)x ? p->deckSize : (unsigned)x, k, j;
            if (!action || action->selectionCount != n)
                return TCG_INVALID;
            for (k = 0; k < n; k++)
            {
                for (j = 0; j < n && p->deck[p->deckSize - 1 - j] != action->selections[k]; j++)
                    ;
                if (j == n)
                    return TCG_INVALID;
                for (j = 0; j < k; j++)
                    if (action->selections[j] == action->selections[k])
                        return TCG_INVALID;
            }
            for (k = 0; k < n; k++)
                p->deck[p->deckSize - 1 - k] = action->selections[k];
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
        }
        else if (ScriptIs(&o, "oattd"))
        {
            /* Strange Tentacles: the Energy picked in their discard pile onto the Defending Pokemon */
            int at = Picked(action, 0, op->discard, op->discardSize);
            if (at < 0 || CardAt(g, them, op->discard[at])->kind != TCG_ENERGY || !foe->stackSize)
                return TCG_INVALID;
            foe->attached[foe->attachedSize++] = TcgPile_Remove(op->discard, &op->discardSize, (unsigned)at);
            TcgDuel_Event(g, TCG_EV_ATTACH, them, 0, 0, op->cards[foe->attached[foe->attachedSize - 1]], 0);
        }
        else if (ScriptIs(&o, "ohshuf"))
        {
            /* Dark Moon: the cards picked in their hand (up to one per Darkness Energy on it) into their deck, then they
             * draw as many */
            unsigned k, n = action ? action->selectionCount : 0, dark = 0;
            for (k = 0; k < self->attachedSize; k++)
                dark += CardAt(g, me, self->attached[k])->kind == TCG_ENERGY && CardAt(g, me, self->attached[k])->type == TCG_DARKNESS;
            if (n > dark)
                return TCG_INVALID;
            for (k = 0; k < n; k++)
            {
                int at = Picked(action, k, op->hand, op->handSize);
                if (at < 0)
                    return TCG_INVALID;
                op->deck[op->deckSize++] = TcgPile_Remove(op->hand, &op->handSize, (unsigned)at);
            }
            if (n)
            {
                TcgDuel_Shuffle(g, them);
                for (k = 0; k < n && op->deckSize; k++)
                    op->hand[op->handSize++] = op->deck[--op->deckSize];
                TcgDuel_Event(g, TCG_EV_DRAW, them, 0, k, 0, 0);
            }
        }
        else if (ScriptIs(&o, "nobody"))
            g->bodiesOffTurn = g->turn; /* (Ancient Wind: every Poke-Body ignored this turn) */
        else if (ScriptIs(&o, "hdis"))
        {
            /* the card picked in the hand (an Energy of type x) discarded */
            int at = action ? Picked(action, 0, p->hand, p->handSize) : -1;
            const struct TcgCard *c = at >= 0 ? CardAt(g, me, p->hand[at]) : NULL;
            if (!c || c->kind != TCG_ENERGY || c->id > 6 || c->type != (unsigned)x)
                return TCG_INVALID;
            p->discard[p->discardSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)at);
        }
        else if (ScriptIs(&o, "allc"))
        {
            unsigned who;
            for (who = 0; who < 2; who++)
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                    if (g->players[who].field[s].stackSize && TcgDuel_Coin(g, me) && (who != them || s || !blocked))
                        TcgDuel_EffectDamage(g, who, s, x, 0);
        }
        else if (ScriptIs(&o, "any1e"))
        {
            unsigned t = action->target, n = ScriptEnergyOf(g, me, sScriptSelf, x);
            if (o.args > 2 && (int)n * y > z)
                n = (unsigned)(z / y);
            if (ValidTarget(g, them, t, 0) && (t || !blocked) && n)
                TcgDuel_EffectDamage(g, them, t, y * (int)n, 0);
        }
        else if (ScriptIs(&o, "dcure"))
        {
            if (!blocked && foe->stackSize && (foe->condition == TCG_ASLEEP || foe->condition == TCG_CONFUSED))
            {
                foe->condition = TCG_HEALTHY;
                TcgDuel_Event(g, TCG_EV_STATUS, them, 0, TCG_HEALTHY, 0, foe->poison);
            }
        }
        else if (ScriptIs(&o, "srchb"))
        {
            unsigned k;
            if (BenchCount(g, me))
            {
                if (!ValidTarget(g, me, action->target, 1))
                    return TCG_INVALID;
                for (k = p->deckSize; k > 0; k--)
                {
                    const struct TcgCard *c = CardAt(g, me, p->deck[k - 1]);
                    if (c->kind == TCG_ENERGY && c->id <= 6 && (x >= TCG_TYPES || c->type == (unsigned)x))
                    {
                        struct TcgFieldCard *t = &p->field[action->target];
                        t->attached[t->attachedSize++] = TcgPile_Remove(p->deck, &p->deckSize, k - 1);
                        TcgDuel_Event(g, TCG_EV_ATTACH, me, action->target, 0, c->id, 0);
                        break;
                    }
                }
                TcgDuel_Shuffle(g, me);
            }
        }
        else if (ScriptIs(&o, "healbb"))
        {
            unsigned who;
            for (who = 0; who < 2; who++)
                for (s = 1; s < TCG_FIELD_SIZE; s++)
                    if (g->players[who].field[s].stackSize && g->players[who].field[s].damage)
                    {
                        unsigned h = g->players[who].field[s].damage < 10u * (unsigned)x ? g->players[who].field[s].damage : 10u * (unsigned)x;
                        g->players[who].field[s].damage -= (uint16_t)h;
                        TcgDuel_Event(g, TCG_EV_HEAL, who, s, h, 0, 0);
                    }
        }
        else if (ScriptIs(&o, "healbe"))
        {
            if (BenchCount(g, me))
            {
                struct TcgFieldCard *t;
                unsigned h;
                if (!ValidTarget(g, me, action->target, 1))
                    return TCG_INVALID;
                t = &p->field[action->target];
                h = 10u * ScriptEnergyOf(g, me, sScriptSelf, x);
                h = h < t->damage ? h : t->damage;
                if (h)
                {
                    t->damage -= (uint16_t)h;
                    TcgDuel_Event(g, TCG_EV_HEAL, me, action->target, h, 0, 0);
                }
            }
        }
        else if (ScriptIs(&o, "omill"))
        {
            unsigned k, n = x ? (unsigned)x : g->scriptHeads;
            for (k = 0; k < n && op->deckSize; k++)
                op->discard[op->discardSize++] = op->deck[--op->deckSize];
            if (k)
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, k, 0, 0);
        }
        else if (ScriptIs(&o, "recmb"))
        {
            if (BenchCount(g, me))
                TcgDuel_SelfDamage(g, x * (int)BenchCount(g, me));
        }
        else if (ScriptIs(&o, "mill"))
        {
            /* each player discards the top x cards of their deck */
            unsigned who, k;
            for (who = 0; who < 2; who++)
            {
                struct TcgPlayer *q = &g->players[who];
                for (k = 0; k < (unsigned)x && q->deckSize; k++)
                    q->discard[q->discardSize++] = q->deck[--q->deckSize];
                if (k)
                    TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, k, 0, 0);
            }
        }
    }
    return TCG_OK;
}
/* Whom a script aims at (TcgDuel_AttackChoice: 1 their Bench, 2 your Bench, 3 any of theirs) */
static unsigned ScriptChoice(const char *script, int *required)
{
    struct ScriptOp o;
    const char *at = script;
    while (ScriptNext(&at, &o))
    {
        if (ScriptIs(&o, "lure") || ScriptIs(&o, "drag"))
        {
            if (required)
                *required = 1;
            return 1;
        }
        if (ScriptIs(&o, "dragmay"))
            return o.arg[0] ? 5 : 1;
        if (ScriptIs(&o, "odevb"))
        {
            if (required)
                *required = 1;
            return 7;
        }
        if (ScriptIs(&o, "healte"))
        {
            if (required)
                *required = 1;
            return 8;
        }
        if (ScriptIs(&o, "evbench") || ScriptIs(&o, "healbd"))
            return 2;
        if (ScriptIs(&o, "gmove"))
            return 9; /* (one of your Benched Grass Pokemon, if any) */
        if (ScriptIs(&o, "hatt") && o.arg[2])
        {
            if (required)
                *required = 1;
            return 4;
        }
        if (ScriptIs(&o, "b1"))
            return 1;
        if (ScriptIs(&o, "ssw") || ScriptIs(&o, "eto") || ScriptIs(&o, "m1") || ScriptIs(&o, "srchb") ||
            ScriptIs(&o, "healbe") || ScriptIs(&o, "benchdeck"))
            return 2;
        if (ScriptIs(&o, "healt"))
        {
            if (required)
                *required = 1;
            return 4;
        }
        if (ScriptIs(&o, "b1h") || ScriptIs(&o, "b1e") || ScriptIs(&o, "luremay"))
            return 1;
        if (ScriptIs(&o, "any1") && o.arg[1])
        {
            if (required)
                *required = 1;
            return 6;
        }
        if (ScriptIs(&o, "any1") || ScriptIs(&o, "any1h") || ScriptIs(&o, "ectr") || ScriptIs(&o, "dte") ||
            ScriptIs(&o, "any1e") || ScriptIs(&o, "any1c"))
        {
            if (required)
                *required = 1;
            return 3;
        }
    }
    return 0;
}
static const char *ScriptPrompt(const char *script)
{
    struct ScriptOp o;
    const char *at = script;
    while (ScriptNext(&at, &o))
    {
        if (ScriptIs(&o, "lure") || ScriptIs(&o, "drag"))
            return o.cond[0] ? "If heads, bring out which of theirs?" : "Bring out which of their Benched Pokemon?";
        if (ScriptIs(&o, "b1"))
            return o.cond[0] ? "If heads, hit which Benched Pokemon?" : "Hit which of their Benched Pokemon?";
        if (ScriptIs(&o, "ssw"))
            return "Switch with which of yours?";
        if (ScriptIs(&o, "gmove"))
            return "Pass the Grass Energy to which Benched Grass Pokemon?";
        if (ScriptIs(&o, "m1"))
            return o.cond[0] ? "If tails, which of yours takes it?" : "Which of your Benched Pokemon takes it?";
        if (ScriptIs(&o, "healt"))
            return o.cond[0] ? "If heads, heal which of your Pokemon?" : "Heal which of your Pokemon?";
        if (ScriptIs(&o, "srchb"))
            return o.cond[0] ? "If heads, attach the Energy to which of yours?" : "Attach the Energy to which of yours?";
        if (ScriptIs(&o, "healbe"))
            return "Heal which of your Benched Pokemon?";
        if (ScriptIs(&o, "benchdeck"))
            return "Shuffle which of your Benched Pokemon into your deck?";
        if (ScriptIs(&o, "dte"))
            return o.arg[0] == 2 ? "Take a Trainer from which of theirs?"
                   : o.arg[0] ? "If heads, take a Special Energy from which of theirs?" : "If heads, discard Energy from which of theirs?";
        if (ScriptIs(&o, "eto"))
            return "Move the Energy to which of your Benched Pokemon?";
        if (ScriptIs(&o, "b1h") || ScriptIs(&o, "b1e"))
            return "Hit which of their Benched Pokemon?";
        if (ScriptIs(&o, "luremay"))
            return "Then switch in which of theirs? (B: none)";
        if (ScriptIs(&o, "dragmay"))
            return o.arg[0] ? "Bring out which undamaged one of theirs? (B: none)" : "Bring out which of theirs? (B: none)";
        if (ScriptIs(&o, "hatt"))
            return "Attach the Energy to which of your Pokemon?";
        if (ScriptIs(&o, "ectr"))
            return "Put counters on which of their Pokemon?";
        if (ScriptIs(&o, "any1") || ScriptIs(&o, "any1h"))
            return o.cond[0] ? "If heads, hit which of their Pokemon?" : "Aim at which of their Pokemon?";
    }
    return NULL;
}
/* What a script's player picks (TcgDuel_AttackPick) */
static unsigned ScriptPick(const char *script, unsigned *count)
{
    struct ScriptOp o;
    const char *at = script;
    while (ScriptNext(&at, &o))
    {
        if (ScriptIs(&o, "dde"))
            return TCG_APICK_FOE_ENERGY;
        if (ScriptIs(&o, "ddeh") || ScriptIs(&o, "dspe") || ScriptIs(&o, "dedeck"))
        {
            *count = ScriptIs(&o, "dspe") ? 100 : ScriptIs(&o, "dedeck") || (o.args && ScriptIs(&o, "ddeh")) ? (unsigned)o.arg[0] : 2;
            return TCG_APICK_FOE_ENERGY;
        }
        if (ScriptIs(&o, "evdeck"))
            return TCG_APICK_EVOLUTION;
        if (ScriptIs(&o, "evbench") || ScriptIs(&o, "dised") || ScriptIs(&o, "distrdeck") || ScriptIs(&o, "evhand"))
        {
            *count = ScriptIs(&o, "evbench") ? 55u : ScriptIs(&o, "dised") ? 56u : ScriptIs(&o, "distrdeck") ? 57u : 58u;
            return TCG_APICK_SEARCH;
        }
        if (ScriptIs(&o, "ohdeck") && o.arg[0] && !o.cond[0])
        {
            *count = 52u + (unsigned)o.arg[0];
            return TCG_APICK_SEARCH;
        }
        if (ScriptIs(&o, "srche") || ScriptIs(&o, "top3h") || ScriptIs(&o, "hdraw"))
        {
            *count = ScriptIs(&o, "srche") ? 50u | (unsigned)(o.arg[1] ? o.arg[1] : o.arg[0] ? o.arg[0] : 3) << 8
                     : ScriptIs(&o, "top3h") ? 51u : 52u | (unsigned)o.arg[0] << 8;
            return TCG_APICK_SEARCH;
        }
        if (ScriptIs(&o, "srchp"))
            return TCG_APICK_DECK_BASIC;
        if (ScriptIs(&o, "emove") && o.arg[0] == 3)
            return TCG_APICK_FIELD_MOVE;
        if (ScriptIs(&o, "dte"))
        {
            *count = (unsigned)o.arg[0];
            return TCG_APICK_TARGET_ENERGY;
        }
        if (ScriptIs(&o, "spike"))
            return TCG_APICK_DECK_ENERGY;
        if (ScriptIs(&o, "dfld"))
        {
            *count = (unsigned)o.arg[0];
            return TCG_APICK_FIELD_ENERGY;
        }
        if (ScriptIs(&o, "odisbench"))
        {
            *count = 82;
            return TCG_APICK_SEARCH;
        }
        if (ScriptIs(&o, "ohbench"))
        {
            *count = 81;
            return TCG_APICK_SEARCH;
        }
        if (ScriptIs(&o, "nsrch") || ScriptIs(&o, "dbown") || ScriptIs(&o, "dbsame"))
        {
            *count = ScriptIs(&o, "nsrch") ? (60u + (unsigned)o.arg[0]) | (unsigned)(o.arg[1] ? o.arg[1] : 60) << 8
                     : ScriptIs(&o, "dbown") ? 70u + (unsigned)o.arg[0] : 80u | 5u << 8;
            return TCG_APICK_SEARCH;
        }
        if (ScriptIs(&o, "mime"))
        {
            *count = 59u | 3u << 8; /* (cards from the hand, then basic Energy from the deck) */
            return TCG_APICK_SEARCH;
        }
        if (ScriptIs(&o, "srchk") || ScriptIs(&o, "dbench") || ScriptIs(&o, "disbench") || ScriptIs(&o, "dishand"))
        {
            *count = ScriptIs(&o, "srchk") ? 10u + (unsigned)o.arg[0] : ScriptIs(&o, "dbench") ? 20u + (unsigned)o.arg[0]
                     : ScriptIs(&o, "disbench") ? 30u : 40u;
            return TCG_APICK_SEARCH;
        }
        if (ScriptIs(&o, "hatt"))
        {
            *count = (unsigned)o.arg[1] | (unsigned)o.arg[0] << 8;
            return TCG_APICK_HAND_ENERGY;
        }
        if (ScriptIs(&o, "bpick"))
        {
            *count = 2;
            return TCG_APICK_FOE_BENCH;
        }
        if (ScriptIs(&o, "distop"))
        {
            *count = (unsigned)o.arg[0];
            return TCG_APICK_DISCARD_CARD;
        }
        if (ScriptIs(&o, "dse") && o.arg[0] >= TCG_TYPES && o.arg[1])
        {
            *count = (unsigned)o.arg[1];
            return TCG_APICK_OWN_ENERGY;
        }
    }
    return TCG_APICK_NONE;
}
/* Why a script's attack can't be used now (NULL: it can) */
static const char *ScriptBlock(const struct TcgDuel *g, unsigned player, const char *script)
{
    struct ScriptOp o;
    const char *at = script;
    while (ScriptNext(&at, &o))
    {
        if (ScriptIs(&o, "dse"))
        {
            unsigned have = o.arg[0] >= TCG_TYPES ? EnergyCards(g, player, 0) : EnergyOfType(g, player, 0, (unsigned)o.arg[0]);
            if (!have || (o.arg[1] && have < (unsigned)o.arg[1]))
                return "Not enough Energy attached to discard.";
        }
        if (ScriptIs(&o, "needob"))
        {
            unsigned s;
            for (s = 1; s < TCG_FIELD_SIZE && !g->players[player ^ 1].field[s].stackSize; s++)
                ;
            if (s == TCG_FIELD_SIZE)
                return "They have no Benched Pokemon.";
        }
        if (ScriptIs(&o, "any1") && o.arg[1])
        {
            unsigned s;
            for (s = 0; s < TCG_FIELD_SIZE && !(g->players[player ^ 1].field[s].stackSize && g->players[player ^ 1].field[s].damage); s++)
                ;
            if (s == TCG_FIELD_SIZE)
                return "None of their Pokemon has damage counters.";
        }
        if (ScriptIs(&o, "needsc") && g->players[player].field[0].damage / 10 < (unsigned)o.arg[0])
            return "It needs more damage counters on it.";
        if (ScriptIs(&o, "needeqe") && EnergyCards(g, player, 0) != EnergyCards(g, player ^ 1, 0))
            return "It and the Defending Pokemon need the same number of Energy cards.";
        if (ScriptIs(&o, "needmb") || ScriptIs(&o, "needroom"))
        {
            unsigned s, n = 0;
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                n += g->players[player].field[s].stackSize != 0;
            if (ScriptIs(&o, "needmb") && !n)
                return "You have no Benched Pokemon.";
            if (ScriptIs(&o, "needroom") && n == TCG_FIELD_SIZE - 1)
                return "Your Bench is full.";
        }
        if (ScriptIs(&o, "needhand") && g->players[player].handSize > (unsigned)o.arg[0])
            return "You have too many cards in your hand.";
        if (ScriptIs(&o, "needlast") && (g->players[player].field[0].lastAttack != o.arg[0] + 1 ||
                                         g->players[player].field[0].lastAttackTurn + 2 != g->turn))
            return "It didn't use the attack this one needs last turn.";
        if ((ScriptIs(&o, "neede") || ScriptIs(&o, "eret")) && !EnergyOfType(g, player, 0, (unsigned)o.arg[0]))
            return "It doesn't have the Energy this attack needs.";
    }
    return NULL;
}
/* The AI's picks for a script (TcgDuel_AiAttackChoices; a target is chosen by tcg_ai.c from ScriptChoice) */
/* A script's menu before the attack (opt:K, TcgDuel_AttackOptions): how many choices */
static unsigned ScriptOptions(const struct TcgDuel *g, unsigned player, const char *script, const char *labels[],
                              uint8_t ok[])
{
    static const char *const counts[TCG_MAX_OPTIONS] = {"NONE", "1", "2", "3", "4", "5", "6", "7"};
    static const char *const conds[5] = {"ASLEEP", "BURNED", "CONFUSED", "PARALYZED", "POISONED"};
    struct ScriptOp o;
    const char *at = script;
    unsigned i, n;
    while (ScriptNext(&at, &o))
    {
        if (!ScriptIs(&o, "opt"))
            continue;
        switch (o.arg[0])
        {
        case 0:
            labels[0] = "NO", ok[0] = 1;
            labels[1] = "YES", ok[1] = o.args < 2 || ScriptEnergyOf(g, player, 0, o.arg[1]) != 0;
            return 2;
        case 1:
        case 2:
            for (i = n = 0; i < 5; i++)
                if (o.arg[0] == 1 || i != 1)
                    labels[n] = conds[i], ok[n++] = 1;
            return n;
        case 7: /* Asleep, Confused or Poisoned */
        case 8: /* Asleep, Confused or Paralyzed */
            labels[0] = "ASLEEP", labels[1] = "CONFUSED", labels[2] = o.arg[0] == 7 ? "POISONED" : "PARALYZED";
            ok[0] = ok[1] = ok[2] = 1;
            return 3;
        case 3:
            labels[0] = "YOUR DECK", ok[0] = g->players[player].deckSize != 0;
            labels[1] = "THEIR DECK", ok[1] = g->players[player ^ 1].deckSize != 0;
            return 2;
        case 5:
            labels[0] = "NO", ok[0] = 1;
            labels[1] = "FIRE", ok[1] = EnergyOfType(g, player, 0, TCG_FIRE) != 0;
            labels[2] = "FIGHTING", ok[2] = EnergyOfType(g, player, 0, TCG_FIGHTING) != 0;
            return 3;
        case 11: /* the Defending Pokemon's attacks, if it's a Basic Pokemon (else no menu) */
        {
            const struct TcgCard *d = TcgDuel_Field(g, player ^ 1, 0);
            if (!d || d->kind != TCG_POKEMON || d->stage || !(RealAttack(&d->attacks[0]) || RealAttack(&d->attacks[1])))
                return 0;
            for (i = 0; i < AttackSlots(d); i++)
            {
                labels[i] = d->attacks[i].name[0] ? d->attacks[i].name : "---";
                ok[i] = RealAttack(&d->attacks[i]);
            }
            return i;
        }
        case 10: /* an Energy type other than Colorless */
        {
            static const char *const types[] = {"GRASS", "FIRE", "WATER", "LIGHTNING", "FIGHTING", "PSYCHIC", "DARKNESS",
                                                "METAL"};
            for (i = 0; i < 8; i++)
                labels[i] = types[i], ok[i] = 1;
            return 8;
        }
        case 9: /* how many Energy cards of type T on all your Pokemon */
            for (i = n = 0; i < TCG_FIELD_SIZE; i++)
                if (g->players[player].field[i].stackSize)
                    n += ScriptEnergyOf(g, player, i, o.args > 1 ? o.arg[1] : TCG_TYPES);
            for (i = 0; i <= n && i < TCG_MAX_OPTIONS; i++)
                labels[i] = counts[i], ok[i] = 1;
            return i;
        case 6:
            n = ScriptEnergyOf(g, player, 0, o.args > 1 ? o.arg[1] : TCG_TYPES);
            for (i = 0; i <= n && i < TCG_MAX_OPTIONS; i++)
                labels[i] = counts[i], ok[i] = 1;
            return i;
        }
    }
    return 0;
}
/* Does a script ask for the top 3 of a deck in order (order3: TCG_ASK_ORDER_TOP3) */
static int ScriptOrders(const char *script)
{
    struct ScriptOp o;
    const char *at = script;
    while (ScriptNext(&at, &o))
        if (ScriptIs(&o, "order3"))
            return 1;
    return 0;
}
static void ScriptAi(const struct TcgDuel *g, struct TcgAction *a, const char *script)
{
    const struct TcgPlayer *p = &g->players[g->current];
    const struct TcgCard *self = TcgDuel_Attacker(g, g->current);
    struct ScriptOp o;
    const char *at = script;
    {
        /* options: the last one that may be chosen (YES, FIGHTING, every Energy...); Paralyzed of the Conditions;
         * their deck only looked at */
        const char *labels[TCG_MAX_OPTIONS];
        uint8_t ok[TCG_MAX_OPTIONS];
        unsigned n = ScriptOptions(g, g->current, script, labels, ok), i;
        a->option = 0;
        for (i = 0; i < n; i++)
            if (ok[i])
                a->option = (uint8_t)i;
        for (i = 0; i < n; i++)
            if (!strcmp(labels[i], "PARALYZED"))
                a->option = (uint8_t)i;
        if (n == 2 && !strcmp(labels[0], "YOUR DECK"))
            a->option = ok[1] ? 1 : 0, a->selectionCount = 0;
    }
    while (ScriptNext(&at, &o))
        if (ScriptIs(&o, "dte"))
        {
            /* the first of theirs with Energy (a Special Energy / a Trainer: dte:1 / 2), its first such card */
            unsigned s, k;
            a->selectionCount = 0;
            for (s = 0; s < TCG_FIELD_SIZE && !a->selectionCount; s++)
                for (k = 0; k < g->players[g->current ^ 1].field[s].attachedSize; k++)
                    if (o.arg[0] == 2 ? CardAt(g, g->current ^ 1, g->players[g->current ^ 1].field[s].attached[k])->kind == TCG_TRAINER
                        : CardAt(g, g->current ^ 1, g->players[g->current ^ 1].field[s].attached[k])->kind == TCG_ENERGY &&
                              (!o.arg[0] || CardAt(g, g->current ^ 1, g->players[g->current ^ 1].field[s].attached[k])->id > 6))
                    {
                        a->target = (uint8_t)s;
                        a->selections[a->selectionCount++] = g->players[g->current ^ 1].field[s].attached[k];
                        break;
                    }
        }
        else if (ScriptIs(&o, "healt"))
        {
            unsigned s, best = 0;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (p->field[s].stackSize && p->field[s].damage > p->field[best].damage)
                    best = s;
            a->target = (uint8_t)best;
        }
        else if (ScriptIs(&o, "srchb") || ScriptIs(&o, "healbe") || ScriptIs(&o, "benchdeck"))
        {
            /* the most damaged of the Bench (healbe), else the first */
            unsigned s, best = 0;
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                if (p->field[s].stackSize && (!best || p->field[s].damage > p->field[best].damage))
                    best = s;
            a->target = (uint8_t)best;
        }
        else if (ScriptIs(&o, "any1e") || ScriptIs(&o, "any1c"))
            a->target = 0;
        else if (ScriptIs(&o, "dragmay"))
            a->target = 0; /* (the AI leaves the Defending Pokemon there) */
        else if (ScriptIs(&o, "any1") && o.arg[1])
        {
            unsigned s;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (g->players[g->current ^ 1].field[s].stackSize && g->players[g->current ^ 1].field[s].damage)
                {
                    a->target = (uint8_t)s;
                    break;
                }
        }
        else if (ScriptIs(&o, "spike"))
        {
            /* a basic Energy of the Active's type onto it, else any */
            unsigned i;
            int pick = -1;
            for (i = 0; i < p->deckSize; i++)
            {
                const struct TcgCard *c = CardAt(g, g->current, p->deck[i]);
                if (c->kind == TCG_ENERGY && c->id <= 6 && (pick < 0 || (self && c->type == self->type)))
                    pick = (int)i;
            }
            a->selectionCount = 0;
            a->target = 0;
            if (pick >= 0)
                a->selections[a->selectionCount++] = p->deck[pick];
        }
        else if (ScriptIs(&o, "m1"))
        {
            /* the Benched Pokemon with the most HP left */
            unsigned s, best = 0;
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                if (p->field[s].stackSize && (!best || HpLeft(g, g->current, s) > HpLeft(g, g->current, best)))
                    best = s;
            a->target = (uint8_t)best;
        }
        else if (ScriptIs(&o, "emove") && o.arg[0] == 3)
        {
            /* a basic Energy from a Benched Pokemon onto the Active */
            unsigned s, k;
            a->selectionCount = 0;
            for (s = 1; s < TCG_FIELD_SIZE && !a->selectionCount; s++)
                for (k = 0; k < p->field[s].attachedSize; k++)
                    if (CardAt(g, g->current, p->field[s].attached[k])->id <= 6)
                    {
                        a->selections[a->selectionCount++] = p->field[s].attached[k];
                        a->target = 0;
                        break;
                    }
        }
        else if (ScriptIs(&o, "srchp"))
        {
            /* a basic Energy of its own type, else any it may take */
            int pick = -1;
            unsigned i;
            for (i = 0; i < p->deckSize; i++)
            {
                const struct TcgCard *c = CardAt(g, g->current, p->deck[i]);
                if (c->kind != TCG_ENERGY || c->id > 6 || (o.arg[0] < TCG_TYPES && c->type != (unsigned)o.arg[0]))
                    continue;
                if (pick < 0 || (self && c->type == self->type))
                    pick = (int)i;
            }
            a->selectionCount = 0;
            if (pick >= 0)
                a->selections[a->selectionCount++] = p->deck[pick];
        }
        else if (ScriptIs(&o, "evdeck"))
        {
            /* the evolution with the most HP */
            int pick = -1;
            unsigned i;
            for (i = 0; i < p->deckSize; i++)
                if (EvolvesFrom(CardAt(g, g->current, p->deck[i]), self) &&
                    (pick < 0 || CardAt(g, g->current, p->deck[i])->hp > CardAt(g, g->current, p->deck[pick])->hp))
                    pick = (int)i;
            a->selectionCount = 0;
            if (pick >= 0)
                a->selections[a->selectionCount++] = p->deck[pick];
        }
        else if (ScriptIs(&o, "srche"))
        {
            /* basic Energy, its own type first */
            unsigned i, pass, max = o.arg[1] ? (unsigned)o.arg[1] : o.arg[0] ? (unsigned)o.arg[0] : 3;
            a->selectionCount = 0;
            for (pass = 0; pass < 2; pass++)
                for (i = 0; i < p->deckSize && a->selectionCount < max; i++)
                {
                    const struct TcgCard *c = CardAt(g, g->current, p->deck[i]);
                    unsigned k, dup = 0;
                    for (k = 0; k < a->selectionCount; k++)
                        dup |= a->selections[k] == p->deck[i];
                    if (!dup && c->kind == TCG_ENERGY && c->id <= 6 && (pass || (self && c->type == self->type)))
                        a->selections[a->selectionCount++] = p->deck[i];
                }
        }
        else if (ScriptIs(&o, "dfld"))
        {
            /* the Energy of that type on its Benched Pokemon (the Active's kept), at least one */
            unsigned s, k;
            a->selectionCount = 0;
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                for (k = 0; k < p->field[s].attachedSize && a->selectionCount < TCG_MAX_OPTIONS - 1; k++)
                {
                    const struct TcgCard *c = CardAt(g, g->current, p->field[s].attached[k]);
                    if (c->kind == TCG_ENERGY && (o.arg[0] >= TCG_TYPES || c->type == (unsigned)o.arg[0]))
                        a->selections[a->selectionCount++] = p->field[s].attached[k];
                }
            for (k = 0; k < p->field[0].attachedSize && !a->selectionCount; k++)
            {
                const struct TcgCard *c = CardAt(g, g->current, p->field[0].attached[k]);
                if (c->kind == TCG_ENERGY && (o.arg[0] >= TCG_TYPES || c->type == (unsigned)o.arg[0]))
                    a->selections[a->selectionCount++] = p->field[0].attached[k];
            }
            a->option = a->selectionCount;
        }
        else if (ScriptIs(&o, "odisbench"))
        {
            /* their Basic with the least HP */
            const struct TcgPlayer *q = &g->players[g->current ^ 1];
            unsigned i;
            a->selectionCount = 0;
            for (i = 0; i < q->discardSize; i++)
            {
                const struct TcgCard *c = CardAt(g, g->current ^ 1, q->discard[i]);
                if (c->kind == TCG_POKEMON && !c->stage &&
                    (!a->selectionCount || c->hp < CardAt(g, g->current ^ 1, a->selections[0])->hp))
                    a->selections[0] = q->discard[i], a->selectionCount = 1;
            }
        }
        else if (ScriptIs(&o, "ohbench"))
        {
            /* their Basic with the least HP */
            const struct TcgPlayer *q = &g->players[g->current ^ 1];
            unsigned i;
            a->selectionCount = 0;
            for (i = 0; i < q->handSize; i++)
            {
                const struct TcgCard *c = CardAt(g, g->current ^ 1, q->hand[i]);
                if (c->kind == TCG_POKEMON && !c->stage &&
                    (!a->selectionCount || c->hp < CardAt(g, g->current ^ 1, a->selections[0])->hp))
                    a->selections[0] = q->hand[i], a->selectionCount = 1;
            }
        }
        else if (ScriptIs(&o, "nsrch") || ScriptIs(&o, "dbown") || ScriptIs(&o, "dbsame"))
        {
            /* every card that matches, as many as may be taken (dbown: the Basic with the most HP; dbsame: as many as
             * the Bench has room for) */
            unsigned i, room = 0, max;
            for (i = 1; i < TCG_FIELD_SIZE; i++)
                room += !p->field[i].stackSize;
            max = ScriptIs(&o, "nsrch") ? (o.arg[1] ? (unsigned)o.arg[1] : 60u) : ScriptIs(&o, "dbown") ? (room ? 1u : 0u) : room;
            a->selectionCount = 0;
            for (i = 0; i < p->deckSize && a->selectionCount < max; i++)
            {
                const struct TcgCard *c = CardAt(g, g->current, p->deck[i]);
                int ok = ScriptIs(&o, "nsrch") ? TcgFlow_InNameGroup(c->name, (unsigned)o.arg[0])
                         : c->kind == TCG_POKEMON && !c->stage &&
                               (ScriptIs(&o, "dbown") ? strstr(c->name, TcgFlow_OwnerWord((unsigned)o.arg[0])) != NULL
                                                      : self && !strcmp(c->name, self->name));
                if (!ok)
                    continue;
                if (ScriptIs(&o, "dbown") && a->selectionCount &&
                    c->hp > CardAt(g, g->current, a->selections[0])->hp)
                    a->selections[0] = p->deck[i];
                else if (!ScriptIs(&o, "dbown") || !a->selectionCount)
                    a->selections[a->selectionCount++] = p->deck[i];
            }
        }
        else if (ScriptIs(&o, "top3h"))
        {
            /* the top card */
            a->selectionCount = 0;
            if (p->deckSize)
                a->selections[a->selectionCount++] = p->deck[p->deckSize - 1];
        }
        else if (ScriptIs(&o, "hdraw") || ScriptIs(&o, "dised") || ScriptIs(&o, "distrdeck"))
            a->selectionCount = 0;
        else if (ScriptIs(&o, "healte"))
        {
            /* the most damaged of yours with Energy */
            unsigned s, best = 0;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (p->field[s].stackSize && EnergyCards(g, g->current, s) &&
                    (!EnergyCards(g, g->current, best) || p->field[s].damage > p->field[best].damage))
                    best = s;
            a->target = (uint8_t)best;
        }
        else if (ScriptIs(&o, "healbd") || ScriptIs(&o, "evbench"))
        {
            /* healbd: the most damaged Benched one; evbench: the first Benched one with an evolution in the deck */
            unsigned s, k, best = 0;
            a->selectionCount = 0;
            for (s = 1; s < TCG_FIELD_SIZE; s++)
            {
                if (!p->field[s].stackSize)
                    continue;
                if (ScriptIs(&o, "healbd"))
                {
                    if (!best || p->field[s].damage > p->field[best].damage)
                        best = s;
                    continue;
                }
                for (k = 0; k < p->deckSize; k++)
                    if (EvolvesFrom(CardAt(g, g->current, p->deck[k]), TcgDuel_Field(g, g->current, s)))
                        break;
                if (k < p->deckSize && !best)
                {
                    best = s;
                    a->selections[a->selectionCount++] = p->deck[k];
                }
            }
            if (!best)
                for (s = 1; s < TCG_FIELD_SIZE && !best; s++)
                    if (p->field[s].stackSize)
                        best = s;
            a->target = (uint8_t)best;
        }
        else if (ScriptIs(&o, "evhand"))
        {
            unsigned k;
            a->selectionCount = 0;
            for (k = 0; k < p->handSize; k++)
                if (EvolvesFrom(CardAt(g, g->current, p->hand[k]), self))
                {
                    a->selections[a->selectionCount++] = p->hand[k];
                    break;
                }
        }
        else if (ScriptIs(&o, "odevb"))
        {
            unsigned s;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (g->players[g->current ^ 1].field[s].stackSize >= 2)
                {
                    a->target = (uint8_t)s;
                    break;
                }
        }
        else if (ScriptIs(&o, "ohdeck") && o.arg[0] && !o.cond[0])
        {
            /* (their hand is looked at: its first Trainer / Energy card) */
            const struct TcgPlayer *op = &g->players[g->current ^ 1];
            unsigned i;
            a->selectionCount = 0;
            for (i = 0; i < op->handSize && !a->selectionCount; i++)
                if (CardAt(g, g->current ^ 1, op->hand[i])->kind == (o.arg[0] == 1 ? TCG_TRAINER : TCG_ENERGY))
                    a->selections[a->selectionCount++] = op->hand[i];
        }
}
