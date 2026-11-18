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
 *             the Defending have (not) the same number of Energy cards */
#define SCRIPT_TOKEN 48
/* Which of the player's Pokemon "it" is: the attacker (0), or a Pokemon whose power runs a script (tcg_powers.c) */
static unsigned sScriptSelf;
struct ScriptOp
{
    char cond[8], name[12];
    int arg[3];
    unsigned args;
};
/* The next op from *s (0: none left) */
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
        strncpy(op->cond, tok, sizeof(op->cond) - 1);
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
    strncpy(op->name, name, sizeof(op->name) - 1);
    return 1;
}
static int ScriptIs(const struct ScriptOp *op, const char *name) { return !strcmp(op->name, name); }
/* An op's phase: 0 coins and damage, 1 after the damage, 3 before the attack */
static int ScriptPhase(const struct ScriptOp *op)
{
    static const char *const before[] = {"c", "cu", "ce", "cdc", "csc", "add", "addh", "mulh", "set", "none", "zero",
                                         "addsc", "adddc", "adde", "addde", "addpast", "dse", "dsx", "nores",
                                         "noweak", "nowr"};
    unsigned i;
    if (ScriptIs(op, "drag"))
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
    if (!strcmp(cond, "at"))
        return !heads;
    if (!strncmp(cond, "ge", 2))
        return heads >= (unsigned)atoi(cond + 2);
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
        g->scriptCoins++;
        g->scriptHeads += TcgDuel_Coin(g, g->current) ? 1 : 0;
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
        f->poison = strstr(what, "psn20") ? 20 : 10;
    if (strstr(what, "brn"))
        f->burned = 1;
}
static unsigned ScriptEnergyOf(const struct TcgDuel *g, unsigned who, unsigned slot, int type)
{
    return type >= TCG_TYPES ? EnergyCards(g, who, slot) : EnergyOfType(g, who, slot, (unsigned)type);
}
static int ScriptRun(struct TcgDuel *g, const char *script, const struct TcgAttack *a, const struct TcgAction *action,
                     unsigned *damage, int after)
{
    struct TcgPlayer *p = &g->players[g->current], *op = &g->players[g->current ^ 1];
    struct TcgFieldCard *self = &p->field[sScriptSelf], *foe = &op->field[0];
    unsigned me = g->current, them = g->current ^ 1, s;
    int blocked = g->foeImmune == TCG_IMMUNE_YES;
    struct ScriptOp o;
    const char *at = script;
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
                 ScriptIs(&o, "psn20") || ScriptIs(&o, "brn"))
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
            TcgDuel_SelfDamage(g, x);
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
            if (!ValidTarget(g, them, action->target, 0))
                return TCG_INVALID;
            if (action->target || !blocked)
                TcgDuel_EffectDamage(g, them, action->target, x, 0);
        }
        else if (ScriptIs(&o, "osw"))
        {
            if (BenchCount(g, them) && !blocked)
                g->switchPending = 1;
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
        else if (ScriptIs(&o, "selfhand") || ScriptIs(&o, "benchdeck"))
        {
            /* it (or the Benched Pokemon chosen) and its cards back to the hand / into the deck */
            unsigned slot = ScriptIs(&o, "selfhand") ? sScriptSelf : action->target, k;
            struct TcgFieldCard *f;
            if (ScriptIs(&o, "benchdeck") && !ValidTarget(g, me, slot, 1))
                return TCG_INVALID;
            f = &p->field[slot];
            for (k = 0; k < f->stackSize + f->attachedSize; k++)
            {
                uint8_t card = k < f->stackSize ? f->stack[k] : f->attached[k - f->stackSize];
                if (slot == sScriptSelf)
                    p->hand[p->handSize++] = card;
                else
                    p->deck[p->deckSize++] = card;
            }
            memset(f, 0, sizeof(*f));
            if (slot != sScriptSelf)
                TcgDuel_Shuffle(g, me);
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
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
             * from the Bench onto it (emove 1), basic from the Bench onto the Active (emove 2) */
            unsigned from = TCG_FIELD_SIZE, to = !x ? action->target : x == 1 ? sScriptSelf : 0, k;
            int at = -1;
            const struct TcgCard *e;
            for (k = 0; k < TCG_FIELD_SIZE && at < 0; k++)
                if ((at = Picked(action, 0, p->field[k].attached, p->field[k].attachedSize)) >= 0)
                    from = k;
            if (at < 0 || !(e = CardAt(g, me, p->field[from].attached[at])) || e->kind != TCG_ENERGY)
                return TCG_INVALID;
            if (ScriptIs(&o, "ehand"))
                p->hand[p->handSize++] = TcgPile_Remove(p->field[from].attached, &p->field[from].attachedSize, (unsigned)at);
            else
            {
                if (to >= TCG_FIELD_SIZE || !p->field[to].stackSize || to == from || (x && !from) ||
                    (x == 1 && e->id != TCG_CARD_LIGHTNING_ENERGY) || (x == 2 && e->id > 6))
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
            /* the card picked from their hand (looked at) into their deck */
            int at = Picked(action, 0, op->hand, op->handSize);
            if (at >= 0)
            {
                op->deck[op->deckSize++] = TcgPile_Remove(op->hand, &op->handSize, (unsigned)at);
                TcgDuel_Shuffle(g, them);
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
            }
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
        if (ScriptIs(&o, "b1"))
            return 1;
        if (ScriptIs(&o, "ssw"))
            return 2;
        if (ScriptIs(&o, "any1"))
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
        if (ScriptIs(&o, "any1"))
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
        if (ScriptIs(&o, "srchp"))
            return TCG_APICK_DECK_BASIC;
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
        if (ScriptIs(&o, "dse"))
        {
            unsigned have = o.arg[0] >= TCG_TYPES ? EnergyCards(g, player, 0) : EnergyOfType(g, player, 0, (unsigned)o.arg[0]);
            if (!have || (o.arg[1] && have < (unsigned)o.arg[1]))
                return "Not enough Energy attached to discard.";
        }
    return NULL;
}
/* The AI's picks for a script (TcgDuel_AiAttackChoices; a target is chosen by tcg_ai.c from ScriptChoice) */
static void ScriptAi(const struct TcgDuel *g, struct TcgAction *a, const char *script)
{
    const struct TcgPlayer *p = &g->players[g->current];
    const struct TcgCard *self = TcgDuel_Attacker(g, g->current);
    struct ScriptOp o;
    const char *at = script;
    while (ScriptNext(&at, &o))
        if (ScriptIs(&o, "srchp"))
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
}
