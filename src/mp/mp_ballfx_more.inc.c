// Moemon Co-op: the ball effect pieces the KANTO, JOHTO and SINNOH achievements give (mp_ballunlock_ach.h):
// 35 send-outs, 35 returns and 20 particle sets. Included by mp_ballfx.c (its helpers: Spawn, Helper, Glow,
// Ring, BodyScale / Turn / Tint, Feet, Head...). Returns always head left (or straight up / down): the way home.

static void MoreEntranceFrame(struct FxBody *b);
static void MoreRecallFrame(struct FxBody *b);

// Their lengths in frames (0: the game's own, 16 to send out, 22 to return)
static u8 MoreLen(bool8 recall, u8 id)
{
    (void)recall;
    (void)id;
    return 0;
}

static void MoreEntranceFrame(struct FxBody *b)
{
    (void)b;
}

static void MoreRecallFrame(struct FxBody *b)
{
    (void)b;
}

static void MoreParticles(u8 part, u8 color, s16 mx, s16 my, s16 feet, s16 head, struct FxBody *b)
{
    (void)part; (void)color; (void)mx; (void)my; (void)feet; (void)head; (void)b;
}
