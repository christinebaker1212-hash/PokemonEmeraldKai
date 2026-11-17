// Moemon Co-op: the ball effect pieces the KANTO, JOHTO and SINNOH achievements give (mp_ballunlock_ach.h):
// 35 send-outs, 35 returns and 20 particle sets. Included by mp_ballfx.c (its helpers: Spawn, Helper, Glow,
// Ring, BodyScale / Turn / Tint, Feet, Head...). Returns always head left (or straight up / down): the way home.
//
// Art: the game's own move animations where they fit (rocks, the SUBSTITUTE doll, eyes, bells, vines, flames,
// eggs, the bent spoon, the moon...) and pieces drawn for these (tools/moemon_ballfx/gen.py more()).

static void MoreEntranceFrame(struct FxBody *b);
static void MoreRecallFrame(struct FxBody *b);

// Their lengths in frames (0: the game's own, 16 to send out, 22 to return)
static u8 MoreLen(bool8 recall, u8 id)
{
    if (!recall)
    {
        switch (id)
        {
        case MPFX_FORM_BOULDER:    return 26;
        case MPFX_FORM_WATERFALL:  return 26;
        case MPFX_FORM_SPARKCAGE:  return 24;
        case MPFX_FORM_BLOSSOM:    return 28;
        case MPFX_FORM_DECOY:      return 26;
        case MPFX_FORM_MINDSEYE:   return 28;
        case MPFX_FORM_MAGMA:      return 28;
        case MPFX_FORM_QUAKE:      return 26;
        case MPFX_FORM_BIRDS:      return 30;
        case MPFX_FORM_BARRIER:    return 28;
        case MPFX_FORM_PINKTRAIL:  return 30;
        case MPFX_FORM_WARPPANEL:  return 28;
        }
        return id >= MPFX_FORM_BOULDER ? 26 : 0;
    }
    return id >= MPFX_RECALL_ROCKTOMB ? 28 : 0;
}

// ---------------------------------------------------------------------------
// Little helpers

// Sizes a part so its longer side is `px` pixels on the battle field (smaller on the overworld), whichever picture is
// drawn: a DS piece or its stand-in from the game's own art. Every piece is sized this way, so it reads the same in
// Single, Double and Triple Battles beside a Moemon of 64 pixels.
// Where things falling from the sky start: a little above its head, never from the top of the screen, so nothing
// crosses the other battlers' HP boxes in Double and Triple Battles
static s16 SkyY(struct FxBody *b)
{
    s16 y = Head(b) - Dist(16);
    return y < 0 ? 0 : y;
}

static void Size(struct FxPart *p, u16 px)
{
    u16 w = 16, h = 16, m;
    if (p == NULL)
        return;
    ArtSize(p->art, &w, &h);
    m = w > h ? w : h;
    p->scale = FieldScale(px * 256 / (m ? m : 16));
}

// Where the body is drawn now (its middle, with the effect's own offsets)
static s16 BodyX(struct FxBody *b) { return MonX(b) + gSprites[b->mon].x2 - b->baseX2; }
static s16 BodyY(struct FxBody *b) { return MonY(b) + gSprites[b->mon].y2 - b->baseY2; }

// A puff of dust (the game's own speed dust), drifting a little to one side
static struct FxPart *Puff(s16 x, s16 y, s16 vx, u16 scale)
{
    struct FxPart *p = Spawn(FXA_BITS_PUFF0, 0, x, y);
    if (p)
    {
        p->nFrames = 3;
        p->frameTime = 3;
        p->flags = PF_ONCE;
        p->vx = vx;
        p->drag = 20;
        p->life = 12;
        p->fadeOut = 6;
        p->scale = scale;
    }
    return p;
}

// n pieces flung out from (x, y) all around (speed in 1/256 px a frame), falling if gravity, px pixels big
static void Fling(u16 art, u8 frames, u8 color, s16 x, s16 y, u8 n, s16 speed, u8 life, s16 gravity, u8 spin, u8 px)
{
    u8 i;
    struct FxPart *p;
    for (i = 0; i < Budget(n); i++)
    {
        u8 a = i * 256 / Budget(n) + RndU(256 / Budget(n));
        if (!(p = Spawn(art + (frames > 1 ? RndU(frames) : 0), color, x, y)))
            break;
        p->vx = Cos(a, speed + RndU(speed / 2));
        p->vy = Sin(a, speed + RndU(speed / 2)) - (gravity ? speed / 2 : 0);
        p->ay = gravity;
        p->life = life + RndU(life / 3);
        p->fadeOut = life / 3;
        p->dRot = spin ? (s8)(Rnd(spin)) : 0;
        Size(p, px);
    }
}

// A piece that eases to (tx, ty) by frame `arrive` and stays there until `life`
static struct FxPart *SettleAt(u16 art, u8 color, s16 x, s16 y, s16 tx, s16 ty, u8 arrive, u8 life)
{
    struct FxPart *p = Spawn(art, color, x, y);
    if (p)
    {
        p->move = MOVE_SETTLE;
        p->tx = tx;
        p->ty = ty;
        p->dRadius = arrive;
        p->life = life;
    }
    return p;
}

// ---------------------------------------------------------------------------
// Send-outs

static void MoreEntranceFrame(struct FxBody *b)
{
    struct Sprite *s = &gSprites[b->mon];
    u8 t = b->t;
    u16 energy = sEnergyRgb[b->color];
    u16 light = MixRgb(energy, RGB_WHITE, 6);
    s16 mx = MonX(b), my = MonY(b), feet = Feet(b), head = Head(b);
    struct FxPart *p;
    s32 r, k;
    u8 i;

    (void)energy;
    switch (b->id)
    {
    case MPFX_FORM_BOULDER:
    {
        // rocks fly in from all around and pile into a boulder over it; the boulder shudders, cracks and falls
        // away in pieces, the Moemon standing in the dust
        static const s8 sPile[10][2] = {{-14, 14}, {0, 16}, {14, 14}, {-18, 0}, {18, 0}, {-12, -14}, {12, -14},
                                        {0, -20}, {-4, 2}, {6, -4}};
        if (t == 0)
            s->invisible = TRUE;
        for (i = 0; i < 10; i++)
        {
            // one at a time, from outside the screen's corner of their side
            if (t == i && (p = SettleAt(FXD_PL_ROCK0 + i % 5, 0, mx + (sPile[i][0] < 0 ? -Dist(90) : Dist(90)),
                                        my - Dist(70) + Rnd(Dist(30)), mx + Dist(sPile[i][0]), my + Dist(sPile[i][1]),
                                        6, 18 - i)))
            {
                Size(p, 18 + RndU(8));
                p->rot = RndU(256);
                p->dRot = 0;
            }
            if (t == i + 6)
            {
                Puff(mx + Dist(sPile[i][0]), my + Dist(sPile[i][1]), 0, 120);
                PlaySE(i & 1 ? SE_M_ROCK_THROW : 0);
            }
        }
        if (t >= 12 && t < 18)
        {
            // the boulder shudders (all of it: the pieces are drawn where they settled; the body shakes inside)
            s->invisible = TRUE;
            if (t == 12)
                for (i = 0; i < 10; i++)
                    if ((p = Spawn(FXD_PL_ROCK0 + i % 5, 0, mx + Dist(sPile[i][0]), my + Dist(sPile[i][1]))))
                    {
                        Size(p, 20);
                        p->life = 6;
                        p->rot = i * 37;
                        p->sway = 1;
                        p->angle = i * 50;
                    }
        }
        if (t == 18)
        {
            // cracks open: the pieces tumble down and bounce, dust rolls out
            s->invisible = FALSE;
            for (i = 0; i < 10; i++)
                if ((p = Spawn(i & 1 ? FXD_BW_BOULDER : FXD_PL_ROCK0 + i % 5, 0, mx + Dist(sPile[i][0]), my + Dist(sPile[i][1]))))
                {
                    p->move = MOVE_BOUNCE;
                    p->vx = sPile[i][0] * 0x18;
                    p->vy = -0x180 - RndU(0x100);
                    p->ay = 0x48;
                    p->ty = feet - 2;
                    p->dRot = sPile[i][0] < 0 ? -9 : 9;
                    Size(p, 16 + RndU(6));
                    p->life = 22;
                    p->fadeOut = 6;
                }
            for (i = 0; i < Budget(6); i++)
                if ((p = Spawn(FXD_BW_CLOUD0 + i % 5, 0, mx + Rnd(Dist(24)), feet - RndU(Dist(10)))))
                {
                    p->tint = RGB(24, 20, 14);
                    p->vx = (i & 1 ? 1 : -1) * (0x80 + RndU(0x80));
                    p->vy = -0x30;
                    p->drag = 12;
                    p->life = 18;
                    p->fadeOut = 10;
                    p->alpha = 12;
                    Size(p, 24);
                    p->dScale = p->scale / 40;
                }
            Ring(MP_BALL_BROWN, TRUE, mx, feet - 2, 10, 120, 30, 70);
            PlaySE(SE_M_ROCK_THROW);
        }
        if (t >= 18)
        {
            BodyTint(b, 10 - 10 * Ramp(t, 18, 25) / 256, RGB(18, 13, 8));
            s->x2 = b->baseX2;
        }
        else
        {
            BodyTint(b, 10, RGB(18, 13, 8));
            s->x2 = b->baseX2 + (t >= 12 ? ((t & 1) ? 1 : -1) : 0);
        }
        break;
    }
    case MPFX_FORM_WATERFALL:
        // a stream of water pours down onto its spot from the top of the screen; it forms in the falling water,
        // see-through at first, and the stream thins away into a last few drops and a splash
        if (t < 20)
        {
            u8 n = t < 14 ? 3 : 1;
            for (i = 0; i < Budget(n); i++)
            {
                if (!(p = Spawn(FXD_BW_WATERFALL, 0, mx + Rnd(Dist(t < 14 ? 6 : 3)), SkyY(b))))
                    break;
                p->stretchX = t < 14 ? 256 : 128;   // the stream thins away
                p->fadeIn = 2;   // (pouring out of the air just above it)
                p->vy = 0x700;
                p->life = (feet - (p->y >> 8)) * 256 / 0x700;
                p->alpha = 12;
                Size(p, 24);
                p->stretchY = 512;   // tall: a falling stream
            }
            if (t % 2 == 0)
                for (i = 0; i < Budget(2); i++)
                    if ((p = Spawn(FXD_PL_DROP, 0, mx + Rnd(Dist(10)), feet - 4)))
                    {
                        p->vx = Rnd(0x180);
                        p->vy = -0x200 - RndU(0x100);
                        p->ay = 0x50;
                        p->life = 12;
                        p->fadeOut = 4;
                        Size(p, 5 + RndU(3));
                    }
        }
        if (t == 20)
        {
            PartSplash(mx, feet);
            Ring(MP_BALL_CYAN, TRUE, mx, feet - 2, 10, 120, 34, 64);
        }
        b->alpha = 3 + 13 * Ramp(t, 4, 20) / 256;
        BodyTint(b, 12 - 12 * Ramp(t, 10, 24) / 256, RGB(18, 26, 31));
        s->y2 = b->baseY2 + (t < 20 ? Sin((t * 40) & 0xFF, 1) : 0);   // the water's weight on it
        break;
    case MPFX_FORM_SPARKCAGE:
    {
        // electric orbs fly out of the ball and circle its spot; bolts jump between them, a crackling cage it
        // flickers into being inside; the cage bursts with a flash and it's free
        s16 rad = Dist(30);
        if (t == 0)
            for (i = 0; i < Budget(6); i++)
                if ((p = Spawn(FXD_BW_ORB, 0, b->bx, b->by)))
                {
                    p->tint = RGB(31, 31, 8);
                    p->move = MOVE_SPIRAL;
                    p->tx = mx;
                    p->ty = my;
                    p->angle = (i * 256 / Budget(6)) << 8;
                    p->radius = 0;
                    p->dRadius = (rad << 8) / 6;
                    p->dAngle = 0x900;
                    p->life = 16;
                    Size(p, 10);
                    p->blend = BLEND_GLOW;
                }
        if (t == 6)
        {
            // they keep circling, at the cage's size
            u16 j;
            for (j = 0; j < sPartHigh; j++)
                if (sParts[j].used && sParts[j].art == FXD_BW_ORB && sParts[j].move == MOVE_SPIRAL && sParts[j].tx == mx)
                    sParts[j].dRadius = 0;
        }
        if (t >= 5 && t < 16)
        {
            // bolts between the orbs (round the cage) and across it
            for (i = 0; i < 2; i++)
            {
                u8 a = (t * 43 + i * 128) & 0xFF;
                if ((p = Spawn(FXD_BW_BOLT0 + RndU(4), 0, mx + Cos(a, rad * 3 / 4), my + Sin(a, rad * 3 / 4))))
                {
                    p->tint = RGB(31, 31, 14);
                    p->life = 3;
                    p->rot = a + 64;
                    p->blend = BLEND_GLOW;
                    Size(p, 14);
                }
            }
            if (t % 3 == 0 && (p = Spawn(FXD_PL_BOLT0 + RndU(4), 0, mx + Rnd(Dist(10)), my + Rnd(Dist(14)))))
            {
                p->tint = RGB(31, 30, 18);
                p->life = 3;
                p->blend = BLEND_GLOW;
                Size(p, 16);
            }
        }
        if (t == 16)
        {
            ScreenFlash(3, 10);
            Fling(FXD_BW_ORB, 1, 0, mx, my, 10, 0x300, 10, 0, 0, 8);   // (white: lightning-bright)
            Fling(FXD_BW_SPARKLE, 1, 0, mx, my, 6, 0x200, 8, 0, 0, 12);
            Ring(MP_BALL_YELLOW, TRUE, mx, my, 10, 150, 40, 256);
            PlaySE(SE_M_THUNDERBOLT2);
        }
        b->alpha = t < 16 ? ((t & 2) ? 6 : 11) : 16;
        BodyTint(b, t < 16 ? ((t & 1) ? 16 : 11) : 12 - 12 * Ramp(t, 16, 23) / 256, RGB(31, 30, 12));
        break;
    }
    case MPFX_FORM_BLOSSOM:
        // petals swirl in from all around and close into a bud around its spot; the bud glows and blooms open,
        // the petals scattering and fluttering down
        if (t == 0)
            s->invisible = TRUE;
        if (t < 10)
        {
            for (i = 0; i < Budget(2); i++)
                if ((p = Spawn(FXD_PL_PETAL0 + RndU(8), 0, mx, my)))
                {
                    p->move = MOVE_SPIRAL;
                    p->tx = mx;
                    p->ty = my;
                    p->angle = ((t * 40 + i * 128) & 0xFF) << 8;
                    p->radius = Dist(70) << 8;
                    p->dRadius = -(Dist(56) << 8) / (14 - t);
                    p->dAngle = 0xC00;
                    p->dRot = 12;
                    p->life = 14 - t;
                    Size(p, 10);
                }
        }
        if (t >= 10 && t < 20)
        {
            // the closed bud: petals turning slowly round it, pink light growing inside
            if (t == 10)
                for (i = 0; i < Budget(8); i++)
                    if ((p = Spawn(FXD_PL_PETAL0 + i, 0, mx, my)))
                    {
                        p->move = MOVE_ORBIT;
                        p->rot = i * 32;
                        p->tx = mx;
                        p->ty = my + Dist(6);
                        p->angle = (i * 32) << 8;
                        p->radius = Dist(16) << 8;
                        p->dAngle = 0x300;
                        p->flags = PF_FAR_DIM;
                        p->underSprite = b->mon;
                        p->life = 10;
                        Size(p, 14);
                    }
            if (t == 12 && (p = Glow(MP_BALL_PINK, mx, my, TRUE, 10, 80, 18)))
                p->alpha = 9;
        }
        if (t == 20)
        {
            // blooms: the petals fly open and flutter down
            s->invisible = FALSE;
            for (i = 0; i < Budget(14); i++)
                if ((p = Spawn(FXD_PL_PETAL0 + i % 8, 0, mx + Cos(i * 18, Dist(12)), my + Sin(i * 18, Dist(8)))))
                {
                    p->move = MOVE_FLUTTER;
                    p->dRot = (i & 1) ? 6 : -6;
                    p->vx = Cos(i * 18, 0x220);
                    p->vy = Sin(i * 18, 0x160) - 0x180;
                    p->ay = 0x18;
                    p->sway = 2;
                    p->angle = i * 40;
                    p->life = 30;
                    p->fadeOut = 10;
                    Size(p, 10);
                    p->flags = PF_NOCLIP;
                }
            Ring(MP_BALL_PINK, TRUE, mx, my, 10, 120, 36, 256);
            PlaySE(SE_M_PETAL_DANCE);
        }
        r = t < 20 ? 140 : 140 + 116 * Ease(Ramp(t, 20, 25)) / 256 + (t == 25 ? 10 : 0);
        BodyScale(b, r, r, TRUE);
        BodyTint(b, 12 - 12 * Ramp(t, 20, 27) / 256, RGB(31, 22, 26));
        break;
    case MPFX_FORM_DECOY:
    {
        // a SUBSTITUTE doll pops up on its spot and stands there wobbling; a puff of smoke, and the doll was it
        struct FxPart *doll = PartAt(b->helper[0]);
        if (t == 0)
        {
            s->invisible = TRUE;
            doll = Helper(b, 0, (b->flags & FLAG_FRONT) ? FXD_BW_DOLL : FXA_MORE_DOLLBACK, 0, mx, my);
        }
        if (doll && t < 16)
        {
            // pops up, then wobbles side to side on its spot
            s32 pop = t < 5 ? 40 + 236 * Ease(Ramp(t, 0, 4)) / 256 : 256 + (t == 5 ? 20 : 0);
            Size(doll, ((b->flags & FLAG_FRONT) ? 60 : 104) * pop / 256);   // (the game's own back doll is small in its picture)
            doll->stretchY = t < 3 ? 300 : 256;
            doll->rot = t >= 6 ? (u8)Sin((t - 6) * 26, 10) : 0;
            doll->x = mx << 8;
            doll->y = (my + Dist(4) - (t < 5 ? Sin(Ramp(t, 0, 4) / 2, Dist(10)) : 0)) << 8;
        }
        if (t == 2)
            Puff(mx, feet - 2, 0, 160);
        if (t == 16)
        {
            Kill(&b->helper[0]);
            PartSmoke(mx, my, 10, 26, 12);
            s->invisible = FALSE;
            PlaySE(SE_M_HAZE);
        }
        if (t >= 16)
        {
            r = 300 - 44 * Ease(Ramp(t, 16, 21)) / 256;
            BodyScale(b, r, r, TRUE);
            BodyTint(b, 10 - 10 * Ramp(t, 16, 24) / 256, RGB_WHITE);
        }
        break;
    }
    case MPFX_FORM_MINDSEYE:
    {
        // an eye opens above its spot and stares; rings of psychic light ripple out from it and the Moemon wavers
        // into being inside them; the eye closes and fades
        s16 ey = head - Dist(18);
        if (t == 0 && (p = Helper(b, 0, FXD_BW_EYE0, 0, mx, ey)))
        {
            p->nFrames = 4;
            p->frameTime = 3;
            p->flags = PF_ONCE;
            p->fadeIn = 3;
            Size(p, 26);
        }
        if (t == 22 && (p = PartAt(b->helper[0])))
        {
            p->life = p->age + 6;
            p->fadeOut = 6;
            p->dScale = -12;
        }
        if (t >= 8 && t < 22 && t % 3 == 2)
        {
            if ((p = Spawn(FXD_BW_OVALS, 0, mx, my)))
            {
                p->tint = RGB(31, 14, 28);
                p->blend = BLEND_GLOW;
                p->alpha = 8;
                p->life = 12;
                p->fadeOut = 8;
                Size(p, 16);
                p->dScale = p->scale / 6;
            }
            if ((p = Ring(MP_BALL_PURPLE, FALSE, mx, ey, 8, 30, 20, 256)))
                p->alpha = 8;
        }
        b->alpha = t < 8 ? 0 : 2 + 14 * Ramp(t, 8, 22) / 256;
        s->invisible = t < 8;
        {
            s32 amp = t < 22 ? 40 * (256 - Ramp(t, 8, 22)) / 256 : 0;
            BodyScale(b, 256 + Sin((t * 48) & 0xFF, amp), 256 - Sin((t * 48) & 0xFF, amp / 2), TRUE);
        }
        BodyTint(b, 12 - 12 * Ramp(t, 14, 26) / 256, RGB(28, 16, 31));
        if (t == 8)
            PlaySE(SE_M_PSYBEAM);
        break;
    }
    case MPFX_FORM_MAGMA:
        // the ground cracks and glows under its spot; lava bursts up in a plume and it rises out of the glow,
        // red-hot, cooling to its own colors as smoke curls away
        if (t == 0)
        {
            if ((p = Glow(MP_BALL_ORANGE, mx, feet, TRUE, 26, 100, 4)))
                p->stretchY = 60, p->fadeOut = 8;
            Fling(FXD_BW_PEBBLE, 1, 0, mx, feet, 6, 0x120, 10, 0x30, 12, 6);
        }
        if (t >= 4 && t < 16 && t % 2 == 0)
            for (i = 0; i < Budget(2); i++)
                if ((p = Spawn(FXD_BW_BLAST0, 0, mx + Rnd(Dist(10)), feet - Dist(8))))
                {
                    p->nFrames = 5;
                    p->frameTime = 2;
                    p->vy = -0x300 - RndU(0x100);
                    p->life = 10;
                    p->fadeOut = 4;
                    Size(p, 14 + RndU(8));
                }
        if (t >= 6 && t % 2 == 0 && (p = Spawn(FXD_BW_FIRE0, 0, mx + Rnd(Dist(16)), feet - RndU(Dist(20)))))
        {
            p->nFrames = 2;
            p->frameTime = 3;
            Size(p, 12);
            p->vy = -0x140;
            p->vx = Rnd(0x80);
            p->life = 14;
            p->fadeOut = 6;
        }
        if (t == 4)
            PlaySE(SE_M_FLAME_WHEEL);
        if (t == 22)
            for (i = 0; i < Budget(4); i++)
                if ((p = Spawn(FXD_BW_SMOKE0 + i, 0, mx + Rnd(Dist(14)), head + Rnd(Dist(6)))))
                {
                    p->tint = RGB(16, 15, 15);
                    p->vy = -0xA0;
                    p->vx = Rnd(0x40);
                    p->life = 20;
                    p->fadeIn = 3;
                    p->fadeOut = 12;
                    p->alpha = 10;
                    Size(p, 16);
                    p->dScale = p->scale / 40;
                }
        // rising out of the ground (cut off at it), red hot, then cooling: orange, a dull red, its colors
        s->mpClipOn = TRUE;
        b->clipped = TRUE;
        s->mpClipY = feet + 1;
        s->y2 = b->baseY2 + (BodyHeight(b) * 3 / 4) * (256 - Ease(Ramp(t, 4, 16))) / 256;
        if (t < 16)
            BodyTint(b, 16, RGB(31, 18, 4));
        else
            BodyTint(b, 16 - 16 * Ramp(t, 16, 27) / 256, t < 21 ? RGB(24, 8, 4) : RGB(10, 6, 6));
        break;
    case MPFX_FORM_QUAKE:
        // the ground shakes and dirt jumps up; it drops in from above and lands with a thud that kicks up dust
        // and pebbles, a shockwave running along the ground
        if (t < 10)
        {
            if (t % 2 == 0)
                for (i = 0; i < Budget(2); i++)
                    if ((p = Spawn(FXA_MORE_PEBBLE0 + RndU(5), 0, mx + Rnd(Dist(40)), feet - 2)))
                    {
                        p->move = MOVE_BOUNCE;
                        p->vy = -0x120 - RndU(0xC0);
                        p->ay = 0x40;
                        p->ty = feet - 2;
                        p->life = 12;
                        p->fadeOut = 4;
                    }
            if (t == 0)
            {
                PlaySE(SE_M_EARTHQUAKE);
                if ((p = Helper(b, 0, FXD_BW_CRACK0, 0, mx, feet + 1)))
                {
                    p->nFrames = 4;
                    p->frameTime = 3;
                    p->flags = PF_ONCE | PF_UNDER;
                    p->underSprite = b->mon;
                    p->rot = 64;   // lying along the ground
                    p->stretchX = 80;
                    Size(p, 36);
                }
            }
        }
        if (t == 20 && (p = PartAt(b->helper[0])))
        {
            p->life = p->age + 6;
            p->fadeOut = 6;
        }
        k = Ramp(t, 4, 12);
        s->invisible = t < 4;
        if (t < 12)
        {
            BodyScale(b, 236, 290, TRUE);   // stretched as it falls
            s->y2 -= Dist(110) * (65536 - k * k) / 65536;   // falling, faster and faster
        }
        else
        {
            static const s16 sThud[] = {90, 54, 24, -10, -4, 0};
            r = sThud[t - 12 < 6 ? t - 12 : 5];
            BodyScale(b, 256 + r / 2, 256 - r, TRUE);
        }
        if (t == 12)
        {
            Ring(MP_BALL_BROWN, TRUE, mx, feet - 2, 12, 120, 44, 56);
            Puff(mx - Dist(14), feet - 3, -0x120, 220);
            Puff(mx + Dist(14), feet - 3, 0x120, 220);
            for (i = 0; i < Budget(6); i++)
                if ((p = Spawn(FXD_BW_CLOUD0 + i % 5, 0, mx + Rnd(Dist(10)), feet - 4)))
                {
                    p->tint = RGB(25, 20, 13);
                    p->vx = (i & 1 ? 1 : -1) * (0x140 + RndU(0xC0));
                    p->vy = -0x60 - RndU(0x60);
                    p->drag = 18;
                    p->life = 16;
                    p->fadeOut = 8;
                    p->alpha = 13;
                    Size(p, 14);
                    p->dScale = p->scale / 30;
                }
            Fling(FXD_BW_PEBBLE, 1, 0, mx, feet - 4, 8, 0x1C0, 14, 0x40, 10, 6);
            ScreenFlash(2, 6);
            PlaySE(SE_M_ROCK_THROW);
        }
        s->x2 = b->baseX2 + (t >= 12 && t < 18 ? ((t & 1) ? 2 : -2) * (18 - t) / 6 : 0);
        break;
    case MPFX_FORM_BIRDS:
    {
        // three birds - of ice, thunder and fire - swoop across the field one after another, each trailing its
        // own sparkle; where their paths cross the Moemon appears in a triple flash
        static const u8 sBirdColor[3] = {MP_BALL_CYAN, MP_BALL_YELLOW, MP_BALL_ORANGE};
        static const u16 sTrail[3] = {FXD_BW_SNOWFLAKE, FXD_BW_BOLT0, FXD_BW_FLAME0};
        static const u16 sTrailTint[3] = {RGB(24, 30, 31), RGB(31, 31, 10), 0};
        s->invisible = t < 16;
        for (i = 0; i < 3; i++)
        {
            s8 dir = (i == 1) ? -1 : 1;   // the second comes from the other side
            if (t == i * 5 && (p = Helper(b, i, FXA_MORE_BIRD, sBirdColor[i], mx - dir * Dist(140), my - Dist(30) + i * Dist(16))))
            {
                Size(p, 34);
                p->flags = PF_NOCLIP | (dir < 0 ? PF_FLIPX : 0);
            }
            if ((p = PartAt(b->helper[i])) && t >= i * 5)
            {
                s32 a = Ramp(t, i * 5, i * 5 + 16);
                p->x = (mx - dir * Dist(140) + dir * Dist(280) * a / 256) << 8;
                p->y = (my - Dist(30) + i * Dist(16) + Sin(a / 2, Dist(18))) << 8;   // a dip as it passes
                p->rot = (u8)(dir * 64 + Sin(a / 2, 10));
                if (t % 2 == 0 && (p = Spawn(sTrail[i], 0, p->x >> 8, p->y >> 8)))
                {
                    p->tint = sTrailTint[i];
                    Size(p, 8);
                    p->life = 8;
                    p->fadeOut = 6;
                    p->blend = BLEND_GLOW;
                    p->vy = 0x40;
                }
                if (a >= 256)
                    Kill(&b->helper[i]);
            }
        }
        if (t == 16)
        {
            for (i = 0; i < 3; i++)
                Ring(sBirdColor[i], FALSE, mx, my, 10 + i * 2, 60 + i * 40, 30, 256);
            Glow(MP_BALL_WHITE, mx, my, TRUE, 10, 160, 20);
            PlaySE(SE_M_SKY_UPPERCUT);
        }
        if (t == 0)
            PlaySE(SE_M_WING_ATTACK);
        r = 180 + 76 * Ease(Ramp(t, 16, 22)) / 256;
        BodyScale(b, r, r, TRUE);
        BodyTint(b, 14 - 14 * Ramp(t, 16, 26) / 256, RGB_WHITE);
        break;
    }
    case MPFX_FORM_BARRIER:
    {
        // panes of light slide in from both sides and box its spot in; the box glows as it forms inside, then the
        // panes shatter into glittering shards
        static const s8 sPane[4] = {-22, 22, -10, 10};
        for (i = 0; i < 3 && i < 4; i++)
            ;
        if (t == 0 && (p = Helper(b, 0, FXD_BW_BOX0, 0, mx, my)))
        {
            p->nFrames = 8;
            p->frameTime = 2;
            p->flags = PF_ONCE;
            p->tint = TINT_ENERGY;
            p->color = b->color;
            p->blend = BLEND_GLOW;
            Size(p, 56);
        }
        if (t == 0)
            for (i = 0; i < 4; i++)
                if ((p = SettleAt(FXA_MORE_PANE, b->color, mx + (sPane[i] < 0 ? -Dist(100) : Dist(100)), my,
                                  mx + Dist(sPane[i]), my + (i >= 2 ? Dist(4) : 0), 8 + i, 20)))
                {
                    p->blend = BLEND_GLOW;
                    p->alpha = i >= 2 ? 7 : 10;
                    p->scale = FieldScale(i >= 2 ? 300 : 280);
                    p->stretchX = i >= 2 ? 200 : 160;   // the front panes broad, the side ones seen edge-on
                    if (i >= 2)
                        p->flags = PF_FAR_DIM;
                }
        if (t == 10)
            PlaySE(SE_M_REFLECT);
        if (t >= 10 && t < 20 && t % 4 == 2 && (p = Glow(b->color, mx, my, TRUE, 6, 180, 0)))
            p->alpha = 6;
        b->alpha = t < 10 ? 0 : 3 + 13 * Ramp(t, 10, 20) / 256;
        s->invisible = t < 10;
        BodyTint(b, 14 - 14 * Ramp(t, 20, 27) / 256, light);
        if (t == 20)
        {
            Kill(&b->helper[0]);
            for (i = 0; i < 4; i++)
                Fling(i & 1 ? FXD_BW_GLASS : FXD_PL_CRYSTAL, 1, 0, mx + Dist(sPane[i]), my, 5, 0x200, 14, 0x28, 16, 10);
            Ring(b->color, TRUE, mx, my, 10, 160, 40, 256);
            ScreenFlash(2, 8);
            PlaySE(SE_ICE_BREAK);
        }
        break;
    }
    case MPFX_FORM_PINKTRAIL:
    {
        // a playful pink light zips out of the ball and loops around its spot in a figure eight, trailing
        // sparkles, then darts into the middle and bursts into it
        struct FxPart *orb = PartAt(b->helper[0]);
        if (t == 0 && (orb = Helper(b, 0, FXD_BW_REDORB, 0, b->bx, b->by)))
        {
            orb->tint = RGB(31, 20, 29);
            orb->blend = BLEND_GLOW;
            orb->alpha = 16;
            Size(orb, 12);
        }
        s->invisible = t < 22;
        if (orb && t < 22)
        {
            s32 a = 512 * Ease(Ramp(t, 0, 20)) / 256;   // two loops
            s16 x = mx + Sin(a & 0xFF, Dist(30)), y = my - Dist(4) + Sin((a * 2) & 0xFF, Dist(14));
            if (t >= 20)
                x = mx, y = my;
            orb->x = x << 8;
            orb->y = y << 8;
            Size(orb, 12 + Sin((t * 40) & 0xFF, 2));
            if ((p = Spawn(FXA_ENERGY_MOTE, MP_BALL_PINK, x, y)))
            {
                p->blend = BLEND_GLOW;
                p->life = 10;
                p->fadeOut = 10;
                p->dScale = -16;
            }
            if (t % 3 == 1 && (p = Spawn(FXD_PL_TWINKLE, 0, x + Rnd(Dist(6)), y + Rnd(Dist(6)))))
            {
                p->tint = RGB(31, 24, 30);
                p->blend = BLEND_GLOW;
                p->life = 8;
                p->fadeOut = 6;
                Size(p, 8);
                p->dRot = 8;
            }
        }
        if (t == 22)
        {
            Kill(&b->helper[0]);
            Ring(MP_BALL_PINK, TRUE, mx, my, 10, 80, 40, 256);
            Glow(MP_BALL_PINK, mx, my, TRUE, 10, 120, 30);
            for (i = 0; i < 6; i++)
                Twinkle(mx + Cos(i * 43, Dist(22)), my + Sin(i * 43, Dist(22)), i / 2);
            PlaySE(SE_M_MORNING_SUN);
        }
        if (t >= 22)
        {
            r = 120 + 136 * Ease(Ramp(t, 22, 27)) / 256 + (t == 27 ? 12 : 0);
            BodyScale(b, r, r, FALSE);
            BodyTint(b, 14 - 14 * Ramp(t, 22, 29) / 256, RGB(31, 20, 28));
        }
        if (t == 0)
            PlaySE(SE_M_TELEPORT);
        break;
    }
    case MPFX_FORM_WARPPANEL:
    {
        // a warp panel lights up on the ground at its spot; rings of light rise from it and the Moemon is drawn
        // in from its feet up behind a bright line, then the panel dims
        s16 h = BodyHeight(b), line = feet - (h + Dist(4)) * Ease(Ramp(t, 6, 20)) / 256;
        if (t == 0 && (p = Helper(b, 0, FXA_EXTRA_RING, b->color, mx, feet - 1)))
        {
            p->blend = BLEND_GLOW;
            p->stretchY = 60;
            p->scale = FieldScale(170);
            p->flags = PF_UNDER;
            p->underSprite = b->mon;
        }
        if ((p = PartAt(b->helper[0])))
        {
            p->alpha = t < 4 ? t * 4 : (t < 22 ? 12 + ((t & 2) ? 2 : 0) : 14 - 14 * Ramp(t, 22, 28) / 256);
            p->scale = FieldScale(150 + 20 * Ease(Ramp(t, 0, 4)) / 256);
        }
        if (t >= 4 && t < 22 && t % 3 == 1 && (p = Ring(b->color, FALSE, mx, feet - 2, 12, FieldScale(150), 0, 60)))
        {
            p->vy = -0x300;   // rising
            p->fadeOut = 10;
        }
        s->mpClipOn = MP_CLIP_ABOVE;
        b->clipped = TRUE;
        s->mpClipY = t < 6 ? feet + 8 : line;
        s->invisible = t < 6;
        if (t >= 6 && t < 21 && (p = Spawn(FXD_BW_PILLAR, 0, mx, line)))
        {
            // the bright line it's drawn in at
            p->tint = TINT_ENERGY;
            p->color = b->color;
            p->blend = BLEND_GLOW;
            p->rot = 64;
            p->stretchX = 40;
            Size(p, h + 8);
            p->life = 2;
        }
        BodyTint(b, 12 - 12 * Ramp(t, 16, 26) / 256, light);
        if (t == 0)
            PlaySE(SE_M_TELEPORT);
        if (t >= 21)
            s->mpClipOn = FALSE;
        break;
    }
    case MPFX_FORM_SACREDFIRE:
    {
        // a ring of flames rises round its feet, each burning in a color of the rainbow that slowly turns; it steps
        // out of them, glowing, and the flames sink away
        s32 height = t < 18 ? Ease(Ramp(t, 0, 8)) : 256 - Ease(Ramp(t, 18, 26));
        for (i = 0; i < Budget(3); i++)
        {
            u8 a = (u8)(t * 23 + i * 85);
            s16 fy = feet - 2 + Sin(a, Dist(7));
            if (height < 16 || !(p = Spawn(i == 1 ? FXD_PL_WISP : FXD_BW_WHITEFLAME, 0, mx + Cos(a, Dist(26)), fy)))
                continue;
            p->tint = MixRgb(Hue((u8)(a + t * 4)), RGB_WHITE, 3);
            p->flags |= (t & 2) ? PF_FLIPX : 0;
            p->vy = -0x90;
            p->life = 8;
            p->fadeOut = 4;
            p->alpha = 13;
            Size(p, 12 + 16 * height / 256);
            p->stretchX = 180;
            if (Sin(a, 1) < 0)
                p->flags |= PF_UNDER;   // the far side burns behind it
        }
        if (t == 2)
            PlaySE(SE_M_FLAME_WHEEL2);
        if (t == 10 && (p = Glow(MP_BALL_ORANGE, mx, feet - Dist(6), TRUE, 14, 160, 8)))
            p->stretchY = 90;
        b->alpha = 2 + 14 * Ramp(t, 6, 16) / 256;
        s->invisible = t < 6;
        BodyTint(b, 14 - 14 * Ramp(t, 12, 24) / 256, MixRgb(Hue((u8)(t * 9)), RGB(31, 20, 8), 12));
        break;
    }
    case MPFX_FORM_AEROBLAST:
    {
        // a silver whirlwind spirals down onto its spot, wind arcs circling it; it forms in the calm eye, and the
        // wind bursts apart in a scatter of silver feathers
        struct FxPart *whirl = PartAt(b->helper[0]);
        if (t == 0 && (whirl = Helper(b, 0, FXD_BW_SWIRL, 0, mx, my - Dist(60))))
        {
            whirl->tint = RGB(26, 28, 31);
            whirl->blend = BLEND_GLOW;
            whirl->alpha = 11;
            whirl->dRot = -18;
        }
        if (whirl && t < 18)
        {
            whirl->y = (my - Dist(60) * (256 - Ease(Ramp(t, 0, 10))) / 256) << 8;
            Size(whirl, 70 - 26 * Ramp(t, 0, 16) / 256);
        }
        if (t < 16 && t % 2 == 0 && (p = Spawn(FXD_BW_WIND, 0, mx, my)))
        {
            p->move = MOVE_ORBIT;
            p->tx = mx;
            p->ty = my + Dist(4);
            p->angle = (t * 37) << 8;
            p->radius = Dist(28) << 8;
            p->dAngle = 0x1400;
            p->tint = RGB(28, 30, 31);
            p->blend = BLEND_GLOW;
            p->flags = PF_FAR_DIM;
            p->life = 8;
            p->fadeOut = 4;
            Size(p, 18);
        }
        if (t == 18)
        {
            Kill(&b->helper[0]);
            Fling(FXD_BW_SILVERFEATHER, 1, 0, mx, my, 8, 0x220, 18, 0x10, 10, 12);
            Ring(MP_BALL_SILVER, TRUE, mx, my, 10, 120, 40, 256);
            PlaySE(SE_M_GUST2);
        }
        if (t == 0)
            PlaySE(SE_M_RAZOR_WIND);
        b->alpha = 3 + 13 * Ramp(t, 8, 18) / 256;
        s->invisible = t < 8;
        BodyTint(b, 12 - 12 * Ramp(t, 14, 24) / 256, RGB(26, 28, 31));
        break;
    }
    case MPFX_FORM_AURORA:
        // curtains of aurora light ripple down over its spot, green, blue and violet, swaying as they fall; it
        // shimmers in beneath them and frost glints drift down
        if (t < 18 && t % 2 == 0)
            for (i = 0; i < Budget(2); i++)
            {
                static const u16 sAurora[3] = {RGB(10, 31, 18), RGB(10, 22, 31), RGB(22, 12, 31)};
                s16 x = mx + ((t / 2 + i * 3) % 5 - 2) * Dist(11);
                if (!(p = Spawn(FXD_BW_PILLAR, 0, x, SkyY(b))))
                    break;
                p->tint = sAurora[(t / 2 + i) % 3];
                p->alpha = 7;
                p->vy = 0x300;
                p->sway = 2;
                p->angle = t * 20 + i * 70;
                p->life = 14;
                p->fadeIn = 3;
                p->fadeOut = 7;
                Size(p, 44);
                p->stretchX = 120;
            }
        if (t >= 8 && t % 3 == 0 && (p = Spawn(FXD_PL_DOTS, 0, mx + Rnd(Dist(24)), head + Rnd(Dist(10)))))
        {
            p->tint = RGB(24, 30, 31);
            p->blend = BLEND_GLOW;
            p->vy = 0x70;
            p->life = 14;
            p->fadeOut = 8;
            Size(p, 16);
        }
        if (t == 0)
            PlaySE(SE_M_MIST);
        b->alpha = 2 + 14 * Ramp(t, 6, 20) / 256;
        s->invisible = t < 6;
        BodyTint(b, 12 - 12 * Ramp(t, 14, 25) / 256, RGB(18, 30, 28));
        break;
    case MPFX_FORM_TIMETRAVEL:
    {
        // green rings of light turn round its spot like the hands of a clock, faster and smaller; it flickers in
        // from the past, then they flash outward and a leaf drifts down
        for (i = 0; i < 2; i++)
        {
            struct FxPart *ring = PartAt(b->helper[i]);
            if (t == 0 && (ring = Helper(b, i, i ? FXD_PL_WARP : FXD_BW_WARP, 0, mx, my)))
            {
                ring->tint = i ? RGB(16, 31, 14) : RGB(10, 26, 12);
                ring->blend = BLEND_GLOW;
                ring->alpha = 10;
            }
            if (ring && t < 18)
            {
                ring->rot = (u8)((i ? -1 : 1) * (s32)(t * t) * 2);
                Size(ring, (i ? 62 : 80) - (i ? 30 : 40) * Ease(Ramp(t, 0, 18)) / 256);
            }
        }
        if (t == 18)
        {
            Kill(&b->helper[0]);
            Kill(&b->helper[1]);
            Ring(MP_BALL_LIME, TRUE, mx, my, 10, 80, 46, 256);
            Glow(MP_BALL_GREEN, mx, my, TRUE, 10, 140, 20);
            for (i = 0; i < Budget(3); i++)
                if ((p = Spawn(FXD_BW_LEAF0 + (i & 1), 0, mx + Rnd(Dist(18)), head - Dist(6))))
                {
                    p->move = MOVE_FLUTTER;
                    p->vy = -0x80;
                    p->ay = 0x10;
                    p->sway = 2;
                    p->angle = i * 70;
                    p->dRot = 5;
                    p->life = 28;
                    p->fadeOut = 10;
                    Size(p, 9);
                }
            PlaySE(SE_M_MORNING_SUN);
        }
        if (t == 0)
            PlaySE(SE_M_TELEPORT);
        // flickering in from the past: there, gone, there more...
        s->invisible = t < 6 || (t < 16 && (t % 3) == 0);
        b->alpha = 4 + 12 * Ramp(t, 6, 18) / 256;
        BodyTint(b, 12 - 12 * Ramp(t, 16, 25) / 256, RGB(18, 31, 16));
        break;
    }
    case MPFX_FORM_UNOWN:
    {
        // glyphs pop up in a row and spell U-N-O-W-N above its spot, blink, then swirl down into its shape, which
        // fills in from a dark silhouette
        static const u8 sWord[5] = {'U' - 'A', 'N' - 'A', 'O' - 'A', 'W' - 'A', 'N' - 'A'};
        s16 rowY = head - Dist(16);
        for (i = 0; i < 5; i++)
        {
            if (t == i * 2 && (p = SettleAt(FXD_PL_LETTER0 + sWord[i], 0, mx + (i - 2) * Dist(14), rowY + Dist(8),
                                            mx + (i - 2) * Dist(14), rowY, 3, 12 - i * 2)))
            {
                p->tint = RGB(9, 9, 12);
                Size(p, 12);
            }
            if (t == 12 && (p = Spawn(FXD_PL_LETTER0 + sWord[i], 0, mx + (i - 2) * Dist(14), rowY)))
            {
                p->tint = RGB(9, 9, 12);
                p->move = MOVE_HOME;
                p->tx = mx + Rnd(Dist(10));
                p->ty = my + Rnd(Dist(12));
                p->dRot = (i & 1) ? 14 : -14;
                p->life = 8;
                p->fadeOut = 3;
                Size(p, 12);
                p->dScale = -p->scale / 10;
            }
        }
        if (t >= 10 && t < 12)
            for (i = 0; i < 5; i++)   // they blink (an eye opening in each)
                if ((p = Spawn(FXD_BW_TWINKLE, 0, mx + (i - 2) * Dist(14), rowY)))
                {
                    p->life = 2;
                    p->blend = BLEND_GLOW;
                    Size(p, 6);
                }
        if (t == 0)
            PlaySE(SE_M_DETECT);
        s->invisible = t < 14;
        BodyTint(b, 16 - 16 * Ramp(t, 16, 25) / 256, RGB(4, 4, 6));
        b->alpha = 6 + 10 * Ramp(t, 14, 20) / 256;
        break;
    }
    case MPFX_FORM_BELL:
    {
        // a golden bell swings above its spot; with each of its three chimes a ring of sound spreads and the Moemon
        // grows clearer
        struct FxPart *bell = PartAt(b->helper[0]);
        if (t == 0 && (bell = Helper(b, 0, FXD_PL_BELL0, 0, mx, head - Dist(22))))
        {
            bell->nFrames = 5;
            bell->frameTime = 3;
            bell->flags = PF_PINGPONG;
            bell->fadeIn = 3;
            Size(bell, 22);
        }
        if (bell && t >= 22)
            bell->alpha = 16 - 16 * Ramp(t, 22, 26) / 256;
        if (t == 4 || t == 10 || t == 16)
        {
            if ((p = Spawn(FXD_BW_RINGS, 0, mx, head - Dist(22))))
            {
                p->tint = RGB(31, 26, 10);
                p->blend = BLEND_GLOW;
                p->alpha = 10;
                p->life = 12;
                p->fadeOut = 10;
                Size(p, 14);
                p->dScale = p->scale / 4;
            }
            Ring(MP_BALL_GOLD, FALSE, mx, my, 12, 60, 30, 256);
            PlaySE(SE_M_HEAL_BELL);
        }
        b->alpha = t < 4 ? 0 : t < 10 ? 5 : t < 16 ? 10 : 16;
        s->invisible = t < 4;
        BodyTint(b, 10 - 10 * Ramp(t, 16, 24) / 256, RGB(31, 28, 16));
        break;
    }
    case MPFX_FORM_TREE:
    {
        // a little tree stands on its spot and wobbles, twigs and leaves shaking; then leaves burst everywhere
        // and it springs up out of them
        struct FxPart *tree = PartAt(b->helper[0]);
        if (t == 0)
        {
            s->invisible = TRUE;
            if ((tree = Helper(b, 0, FXD_BW_TWIGS, 0, mx, feet - Dist(20))))
                Size(tree, 40);
            if ((p = Helper(b, 1, FXD_PL_GRASS, 0, mx, feet - Dist(30))))
            {
                p->tint = RGB(14, 26, 12);
                Size(p, 34);
            }
        }
        for (i = 0; i < 2 && t < 16; i++)
            if ((p = PartAt(b->helper[i])))
            {
                p->rot = (u8)(t < 4 ? 0 : Sin((t - 4) * 40, 10 + t / 2));   // wobbling more and more
                Size(p, (i ? 34 : 40) * (t < 3 ? 100 + 52 * t : 256) / 256);
            }
        if (t == 16)
        {
            Kill(&b->helper[0]);
            Kill(&b->helper[1]);
            s->invisible = FALSE;
            for (i = 0; i < Budget(12); i++)
                if ((p = Spawn(i & 1 ? FXD_BW_LEAF0 + (i & 2 ? 1 : 0) : FXD_PL_LEAF0 + (i & 2 ? 1 : 0), 0, mx, feet - Dist(24))))
                {
                    p->move = MOVE_FLUTTER;
                    p->vx = Cos(i * 21, 0x260);
                    p->vy = Sin(i * 21, 0x180) - 0x200;
                    p->ay = 0x20;
                    p->sway = 2;
                    p->angle = i * 33;
                    p->dRot = (i & 1) ? 7 : -7;
                    p->life = 26;
                    p->fadeOut = 8;
                    Size(p, 8 + RndU(4));
                }
            PlaySE(SE_M_RAZOR_WIND2);
        }
        if (t >= 16)
        {
            static const s16 sSpring[] = {-40, -10, 30, 12, -6, 0};
            r = sSpring[t - 16 < 6 ? t - 16 : 5];
            BodyScale(b, 256 + r / 2, 256 - r, TRUE);
            s->y2 -= t < 20 ? Sin(Ramp(t, 16, 20) / 2, Dist(10)) : 0;
        }
        break;
    }
    case MPFX_FORM_DRAGON:
    {
        // a dragon of light coils up around its spot from the ball: a glowing body following a crescent head up a
        // spiral; at the top the head flashes and the Moemon is there
        s32 k = Ramp(t, 0, 16);
        s16 hx = mx + Cos((u8)(k * 3 / 2), Dist(24)), hy = feet - (feet - head + Dist(12)) * k / 256;
        if (t < 16)
        {
            if ((p = Spawn(FXD_BW_GLOW, 0, hx, hy)))
            {
                p->tint = TINT_ENERGY;
            p->color = b->color;
                p->blend = BLEND_GLOW;
                p->life = 12;
                p->fadeOut = 10;
                Size(p, 12);
                p->dScale = -p->scale / 14;
                if (Sin((u8)(k * 3 / 2 + 64), 1) < 0)
                    p->flags = PF_UNDER;   // behind it on the far side of the coil
            }
            if ((p = Spawn(FXD_BW_CLAW0 + (t / 2) % 5, 0, hx, hy)))
            {
                p->tint = MixRgb(energy, RGB_WHITE, 8);
                p->blend = BLEND_GLOW;
                p->life = 2;
                p->rot = (u8)(k * 3 / 2 + 64);
                Size(p, 16);
            }
        }
        if (t == 16)
        {
            Glow(b->color, mx, head, TRUE, 10, 200, 20);
            Ring(b->color, TRUE, mx, my, 10, 100, 40, 256);
            PlaySE(SE_M_DRAGON_RAGE);
        }
        s->invisible = t < 16;
        r = 200 + 56 * Ease(Ramp(t, 16, 22)) / 256;
        BodyScale(b, r, r, TRUE);
        BodyTint(b, 14 - 14 * Ramp(t, 16, 25) / 256, light);
        break;
    }
    case MPFX_FORM_RADIO:
        // radio waves pulse out from its spot and it comes in through the static: a fuzzy, flickering picture that
        // tunes in clearer and clearer
        if (t < 18 && t % 3 == 0)
            for (i = 0; i < 2; i++)
                if ((p = Spawn(FXD_PL_WAVES, 0, mx + (i ? Dist(20) : -Dist(20)), my)))
                {
                    p->tint = RGB(26, 30, 31);
                    p->blend = BLEND_GLOW;
                    p->alpha = 9;
                    p->rot = i ? 64 : 192;
                    p->vx = (i ? 1 : -1) * 0x180;
                    p->life = 10;
                    p->fadeOut = 6;
                    Size(p, 18);
                }
        if (t < 16 && (p = Spawn(t & 1 ? FXD_PL_NOISE : FXD_BW_NOISE, 0, mx, my)))
        {
            // the static over it
            p->tint = RGB(20, 22, 24);
            p->alpha = 5 - 5 * t / 16;
            p->life = 1;
            p->framePhase = 0;
            p->rot = t & 2 ? 128 : 0;
            Size(p, BodyHeight(b) * 3 / 4);
        }
        s->oam.mosaic = t < 16;
        sScreen.mosaicWant = t < 16 ? 7 - 7 * t / 16 : 0;
        s->invisible = t < 2 || (t < 10 && (t % 4) == 3);
        b->alpha = 6 + 10 * Ramp(t, 4, 16) / 256;
        s->x2 = b->baseX2 + (t < 12 && (t % 5) == 2 ? 2 : 0);   // the picture jumps now and then
        BodyTint(b, 10 - 10 * Ramp(t, 8, 18) / 256, RGB(20, 26, 31));
        if (t == 0)
            PlaySE(SE_M_LOCK_ON);
        break;
    case MPFX_FORM_APRICORN:
    {
        // an APRICORN in its energy's color falls onto its spot and bounces; it splits in two halves and the Moemon
        // pops out between them
        static const u16 sApri[7] = {FXA_MORE_APRIRED, FXA_MORE_APRIBLU, FXA_MORE_APRIYLW, FXA_MORE_APRIGRN,
                                     FXA_MORE_APRIPNK, FXA_MORE_APRIWHT, FXA_MORE_APRIBLK};
        u16 fruit = sApri[b->color % 7];
        if (t == 0)
        {
            s->invisible = TRUE;
            if ((p = Helper(b, 0, fruit, 0, mx, my - Dist(60))))
            {
                p->move = MOVE_BOUNCE;
                p->vy = 0x200;
                p->ay = 0x70;
                p->ty = feet - Dist(8);
                p->dRot = 6;
                Size(p, 18);
            }
        }
        if (t == 14)
        {
            Kill(&b->helper[0]);
            for (i = 0; i < 2; i++)
                if ((p = Spawn(fruit, 0, mx + (i ? 3 : -3), feet - Dist(8))))
                {
                    p->stretchX = 128;
                    p->vx = (i ? 1 : -1) * 0x150;
                    p->vy = -0x150;
                    p->ay = 0x40;
                    p->dRot = i ? 10 : -10;
                    p->flags = i ? 0 : PF_FLIPX;
                    p->life = 14;
                    p->fadeOut = 6;
                    Size(p, 18);
                    p->stretchX = 128;
                }
            Puff(mx, feet - 4, 0, 200);
            s->invisible = FALSE;
            PlaySE(SE_BALL_OPEN);
        }
        if (t >= 14)
        {
            r = t < 18 ? 80 + 220 * Ease(Ramp(t, 14, 18)) / 256 : 300 - 44 * Ease(Ramp(t, 18, 22)) / 256;
            BodyScale(b, r, r, TRUE);
            BodyTint(b, 12 - 12 * Ramp(t, 14, 22) / 256, RGB_WHITE);
        }
        break;
    }
    case MPFX_FORM_MILK:
    {
        // a milk bottle appears above its spot and tips over; the milk pours down in a white splash and it forms in
        // the white, its colors coming through
        struct FxPart *bottle = PartAt(b->helper[0]);
        if (t == 0 && (bottle = Helper(b, 0, FXD_BW_MILK0, 0, mx + Dist(10), head - Dist(16))))
        {
            bottle->fadeIn = 3;
            Size(bottle, 30);
        }
        if (bottle)
        {
            bottle->rot = (u8)(-(s32)(t < 8 ? 64 * Ease(Ramp(t, 2, 8)) / 256 : 64));   // tipping over to pour
            if (t >= 18)
                bottle->alpha = 16 - 16 * Ramp(t, 18, 22) / 256;
        }
        if (t >= 6 && t < 16)
            for (i = 0; i < Budget(2); i++)
                if ((p = Spawn(FXD_PL_DROP, 0, mx + Rnd(Dist(5)), head - Dist(10))))
                {
                    p->tint = RGB(31, 31, 29);
                    p->vy = 0x300 + RndU(0x100);
                    p->ay = 0x30;
                    p->life = (feet - head) * 256 / 0x400;
                    Size(p, 5);
                }
        if (t == 12)
            for (i = 0; i < Budget(6); i++)
                if ((p = Spawn(FXD_BW_CLOUD0 + i % 5, 0, mx + Rnd(Dist(12)), feet - Dist(4))))
                {
                    p->tint = RGB(31, 31, 30);
                    p->vx = (i & 1 ? 1 : -1) * (0xC0 + RndU(0x80));
                    p->vy = -0xA0 - RndU(0x80);
                    p->drag = 14;
                    p->life = 14;
                    p->fadeOut = 8;
                    Size(p, 14);
                }
        if (t == 6)
            PlaySE(SE_M_MILK_DRINK);
        s->invisible = t < 10;
        b->alpha = 4 + 12 * Ramp(t, 10, 18) / 256;
        BodyTint(b, 16 - 16 * Ramp(t, 14, 24) / 256, RGB(31, 31, 29));
        break;
    }
    case MPFX_FORM_FIREFLIES:
        // a swarm of glowing bugs drifts in from all around and settles over its dark shape, blinking; as they
        // gather it lights up, and they flit away
        if (t < 16)
            for (i = 0; i < Budget(2); i++)
            {
                u8 a = RndU(256);
                if (!(p = Spawn(FXD_BW_SOFTBALL, 0, mx + Cos(a, Dist(70)), my + Sin(a, Dist(50)))))
                    break;
                p->tint = RGB(22, 31, 8);
                p->blend = BLEND_GLOW;
                p->move = MOVE_HOME;
                p->tx = mx + Rnd(Dist(18));
                p->ty = my + Rnd(Dist(22));
                p->life = 10 + RndU(6);
                p->fadeIn = 3;
                p->fadeOut = 3;
                p->sway = 1;
                Size(p, 4);
            }
        if (t >= 8 && t < 22 && (p = Spawn(FXD_BW_SOFTBALL, 0, mx + Rnd(Dist(18)), my + Rnd(Dist(22)))))
        {
            // blinking on it
            p->tint = RGB(26, 31, 10);
            p->blend = BLEND_GLOW;
            p->life = 3;
            Size(p, 6);
        }
        if (t == 22)
            for (i = 0; i < Budget(8); i++)
                if ((p = Spawn(FXD_BW_SOFTBALL, 0, mx + Rnd(Dist(16)), my + Rnd(Dist(20)))))
                {
                    p->tint = RGB(22, 31, 8);
                    p->blend = BLEND_GLOW;
                    p->vx = Rnd(0x180);
                    p->vy = -0xC0 - RndU(0x80);
                    p->sway = 1;
                    p->life = 16;
                    p->fadeOut = 10;
                    Size(p, 4);
                }
        BodyTint(b, 16 - 16 * Ramp(t, 10, 24) / 256, t < 18 ? RGB(2, 4, 3) : RGB(22, 31, 10));
        if (t == 0)
            PlaySE(SE_M_SWEET_SCENT);
        break;
    case MPFX_FORM_DISTORT:
    {
        // the ground at its spot twists open into a dark, turning hole; shadowy tendrils reach out of it and the
        // Moemon rises from the hole, warped, settling into its shape as the hole closes
        struct FxPart *hole = PartAt(b->helper[0]);
        if (t == 0 && (hole = Helper(b, 0, FXD_BW_DARKDOTS, 0, mx, feet)))
        {
            hole->tint = RGB(14, 6, 20);
            hole->stretchY = 70;
            hole->flags = PF_UNDER;
            hole->underSprite = b->mon;
            hole->dRot = 6;
        }
        if (hole)
        {
            Size(hole, 8 + 48 * (t < 18 ? Ease(Ramp(t, 0, 6)) : 256 - Ease(Ramp(t, 18, 24))) / 256);
            hole->stretchY = 70;
        }
        if (t >= 2 && t < 16 && t % 3 == 2 && (p = Spawn(FXD_BW_STATICBALL, 0, mx + Rnd(Dist(20)), feet - 4)))
        {
            p->tint = RGB(10, 4, 16);
            p->vy = -0x140;
            p->life = 10;
            p->fadeOut = 6;
            p->alpha = 11;
            Size(p, 10);
            p->dScale = -p->scale / 12;
        }
        if (t == 2)
            for (i = 0; i < 2; i++)
                if ((p = Spawn(FXA_MORE_TENDRIL, 0, mx + (i ? Dist(14) : -Dist(14)), feet - Dist(16))))
                {
                    p->tint = RGB(12, 6, 18);
                    p->flags = i ? PF_FLIPX : 0;
                    p->life = 16;
                    p->fadeIn = 4;
                    p->fadeOut = 6;
                    Size(p, 34);
                }
        s->mpClipOn = TRUE;
        b->clipped = TRUE;
        s->mpClipY = feet + 1;
        s->y2 = b->baseY2 + BodyHeight(b) * (256 - Ease(Ramp(t, 4, 18))) / 256;
        r = t < 22 ? 30 * (256 - Ramp(t, 4, 22)) / 256 : 0;
        BodyScale(b, 256 + Sin((t * 30) & 0xFF, r), 256 - Sin((t * 30) & 0xFF, r / 2), FALSE);
        s->y2 += BodyHeight(b) * (256 - Ease(Ramp(t, 4, 18))) / 256;
        BodyTint(b, 14 - 14 * Ramp(t, 14, 25) / 256, RGB(10, 4, 16));
        if (t == 0)
            PlaySE(SE_M_HAZE);
        break;
    }
    case MPFX_FORM_SPACERIFT:
    {
        // a pink slash cuts the air at its spot and splits open into a shining tear; it steps out through the tear,
        // which seals itself behind it in a glitter
        struct FxPart *tear = PartAt(b->helper[0]);
        if (t == 0 && (tear = Helper(b, 0, FXD_BW_PILLAR, 0, mx, my)))
        {
            tear->tint = RGB(31, 16, 26);
            tear->blend = BLEND_GLOW;
        }
        if (tear)
        {
            Size(tear, BodyHeight(b) + 6);
            tear->stretchY = 256 * Ease(Ramp(t, 0, 4)) / 256 + 8;
            tear->stretchX = t < 6 ? 40 : t < 18 ? 40 + 160 * Ease(Ramp(t, 6, 10)) / 256 : 200 - 190 * Ramp(t, 18, 23) / 256;
            tear->alpha = 14;
        }
        if (t >= 6 && t < 18 && t % 2 == 0 && (p = Spawn(FXD_PL_SPARKLE, 0, mx + Rnd(Dist(10)), my + Rnd(Dist(24)))))
        {
            p->tint = RGB(31, 24, 30);
            p->blend = BLEND_GLOW;
            p->vx = Rnd(0x100);
            p->life = 8;
            p->fadeOut = 6;
            Size(p, 10);
        }
        if (t == 23)
        {
            Kill(&b->helper[0]);
            Ring(MP_BALL_PINK, TRUE, mx, my, 8, 60, 30, 256);
        }
        if (t == 0)
            PlaySE(SE_M_RAZOR_WIND);
        if (t == 8)
            PlaySE(SE_M_TELEPORT);
        s->invisible = t < 8;
        b->alpha = 4 + 12 * Ramp(t, 8, 16) / 256;
        r = 140 + 116 * Ease(Ramp(t, 8, 16)) / 256;
        BodyScale(b, r, r, TRUE);
        BodyTint(b, 12 - 12 * Ramp(t, 14, 24) / 256, RGB(31, 20, 28));
        break;
    }
    case MPFX_FORM_LAKE:
    {
        // a ripple spreads on the ground; three lights - yellow, pink and blue - rise out of it, circling each
        // other up to its middle, and merge into it in a flash
        static const u16 sLight[3] = {RGB(31, 30, 10), RGB(31, 18, 26), RGB(12, 22, 31)};
        if (t == 0 || t == 4)
            if ((p = Spawn(FXD_BW_OVALS, 0, mx, feet - 1)))
            {
                p->tint = RGB(16, 26, 31);
                p->alpha = 10;
                p->stretchY = 90;
                p->flags = PF_UNDER;
                p->life = 14;
                p->fadeOut = 10;
                Size(p, 16);
                p->dScale = p->scale / 7;
            }
        for (i = 0; i < 3 && t < 18; i++)
        {
            struct FxPart *orb = PartAt(b->helper[i]);
            s32 k = Ramp(t, 2, 16);
            u8 a = (u8)(i * 85 + t * 18);
            if (t == 2 && (orb = Helper(b, i, FXD_BW_ORB, 0, mx, feet)))
            {
                orb->tint = sLight[i];
                orb->blend = BLEND_GLOW;
                orb->trailEvery = 1;
                orb->trailArt = FXA_ENERGY_MOTE;
                orb->color = i == 0 ? MP_BALL_YELLOW : i == 1 ? MP_BALL_PINK : MP_BALL_BLUE;
            }
            if (orb)
            {
                orb->x = (mx + Cos(a, Dist(20) * (256 - k) / 256)) << 8;
                orb->y = (feet - (feet - my) * Ease(k) / 256 + Sin(a, Dist(6) * (256 - k) / 256)) << 8;
                Size(orb, 10);
            }
        }
        if (t == 18)
        {
            for (i = 0; i < 3; i++)
            {
                Kill(&b->helper[i]);
                Ring(i == 0 ? MP_BALL_YELLOW : i == 1 ? MP_BALL_PINK : MP_BALL_BLUE, FALSE, mx, my, 10, 40 + i * 30, 30, 256);
            }
            Glow(MP_BALL_WHITE, mx, my, TRUE, 10, 150, 20);
            PlaySE(SE_M_MORNING_SUN);
        }
        s->invisible = t < 16;
        r = 160 + 96 * Ease(Ramp(t, 16, 22)) / 256;
        BodyScale(b, r, r, FALSE);
        BodyTint(b, 14 - 14 * Ramp(t, 17, 25) / 256, RGB(28, 30, 31));
        if (t == 0)
            PlaySE(SE_M_WATERFALL);
        break;
    }
    case MPFX_FORM_MOON:
    {
        // a crescent moon fades in over its spot and pours down a pale beam of moonlight; it forms in the light,
        // sparkles drifting, and the moon fades away
        struct FxPart *moon = PartAt(b->helper[0]);
        s16 my0 = SkyY(b);
        if (t == 0 && (moon = Helper(b, 0, FXD_BW_CRESCENT, 0, mx + Dist(4), my0)))
        {
            moon->tint = RGB(31, 30, 20);
            moon->fadeIn = 6;
            Size(moon, 30);
        }
        if (moon && t >= 20)
            moon->alpha = 16 - 16 * Ramp(t, 20, 26) / 256;
        if (t >= 4 && t < 22 && (p = Spawn(FXD_BW_RAY, 0, mx, (my0 + feet) / 2)))
        {
            p->tint = RGB(26, 26, 31);
            p->blend = BLEND_GLOW;
            p->alpha = t < 8 ? (t - 4) * 2 : t < 18 ? 8 : 8 - (t - 18) * 2;
            p->life = 1;
            Size(p, feet - my0);
            p->stretchX = 170;
        }
        if (t >= 6 && t % 3 == 0 && (p = Spawn(FXD_PL_TWINKLE, 0, mx + Rnd(Dist(18)), my0 + RndU(feet - my0))))
        {
            p->tint = RGB(28, 28, 31);
            p->blend = BLEND_GLOW;
            p->vy = 0x60;
            p->life = 10;
            p->fadeOut = 6;
            Size(p, 8);
        }
        if (t == 4)
            PlaySE(SE_M_MOONLIGHT);
        s->invisible = t < 8;
        b->alpha = 2 + 14 * Ramp(t, 8, 20) / 256;
        BodyTint(b, 12 - 12 * Ramp(t, 16, 26) / 256, RGB(26, 26, 31));
        break;
    }
    case MPFX_FORM_NIGHTMARE:
        // black mist swirls in over its spot; a grinning face looms in the mist, red eyes glinting, then the mist
        // is blown away and it stands there
        if (t < 14)
            for (i = 0; i < Budget(2); i++)
                if ((p = Spawn(FXD_BW_SMOKE0 + (t + i) % 4, 0, mx, my)))
                {
                    p->move = MOVE_SPIRAL;
                    p->tx = mx;
                    p->ty = my;
                    p->angle = ((t * 47 + i * 128) & 0xFF) << 8;
                    p->radius = Dist(40) << 8;
                    p->dRadius = -(Dist(26) << 8) / 12;
                    p->dAngle = 0x900;
                    p->tint = RGB(5, 3, 8);
                    p->alpha = 13;
                    p->life = 18 - t / 2;
                    p->fadeIn = 3;
                    p->fadeOut = 6;
                    Size(p, 22);
                }
        if (t == 6 && (p = Helper(b, 0, FXD_BW_GHOSTFACE0, 0, mx, my - Dist(6))))
        {
            p->nFrames = 3;
            p->frameTime = 3;
            p->flags = PF_ONCE;
            p->fadeIn = 4;
            Size(p, 34);
        }
        if (t == 16)
        {
            Kill(&b->helper[0]);
            for (i = 0; i < Budget(8); i++)
                if ((p = Spawn(FXD_BW_SMOKE0 + i % 4, 0, mx, my)))
                {
                    p->tint = RGB(6, 4, 10);
                    p->vx = Cos(i * 32, 0x280);
                    p->vy = Sin(i * 32, 0x200);
                    p->drag = 10;
                    p->alpha = 11;
                    p->life = 12;
                    p->fadeOut = 10;
                    Size(p, 20);
                }
            PlaySE(SE_M_FAINT_ATTACK);
        }
        if (t == 0)
            PlaySE(SE_M_CONFUSE_RAY);
        s->invisible = t < 14;
        BodyTint(b, 14 - 14 * Ramp(t, 16, 25) / 256, RGB(6, 3, 10));
        break;
    case MPFX_FORM_SEEDFLARE:
        // seeds pop out of the ground round its spot and sprout, growing into a ring of flowers; it springs up from
        // among them and petals burst into the air
        for (i = 0; i < Budget(6); i++)
            if (t == i && (p = Spawn(FXD_BW_SPROUT0, 0, mx + Cos(i * 43, Dist(28)), feet - 4 + Sin(i * 43, Dist(5)))))
            {
                p->nFrames = 6;
                p->frameTime = 2;
                p->flags = PF_ONCE | (Sin(i * 43, 1) < 0 ? PF_UNDER : 0);
                p->tint = RGB(16, 28, 12);
                p->life = 14;
                Size(p, 14);
                p->ty = 0;
            }
        for (i = 0; i < Budget(6); i++)
            if (t == 12 + i / 2 && (p = Spawn(FXD_BW_FLOWER, 0, mx + Cos(i * 43, Dist(28)), feet - 6 + Sin(i * 43, Dist(5)))))
            {
                p->tint = (i & 1) ? RGB(31, 26, 30) : RGB(31, 31, 31);
                p->flags = Sin(i * 43, 1) < 0 ? PF_UNDER : 0;
                p->life = 14;
                p->fadeOut = 6;
                Size(p, 4);
                p->dScale = p->scale;   // opening
            }
        if ((t >= 14 && t < 16))
            for (i = 0; i < Budget(6); i++)
                if ((p = Spawn(FXD_PL_PETAL0 + RndU(8), 0, mx + Rnd(Dist(26)), feet - 6)))
                {
                    p->move = MOVE_FLUTTER;
                    p->vy = -0x260 - RndU(0x100);
                    p->vx = Rnd(0x100);
                    p->ay = 0x20;
                    p->sway = 2;
                    p->dRot = 6;
                    p->life = 24;
                    p->fadeOut = 8;
                    Size(p, 8);
                }
        if (t == 14)
            PlaySE(SE_M_SWEET_SCENT);
        s->invisible = t < 14;
        if (t >= 14)
        {
            static const s16 sSpring[] = {-60, -20, 26, 10, -4, 0};
            r = sSpring[t - 14 < 6 ? t - 14 : 5];
            BodyScale(b, 256 + r / 2, 256 - r, TRUE);
            s->y2 -= t < 18 ? Sin(Ramp(t, 14, 18) / 2, Dist(12)) : 0;
            BodyTint(b, 10 - 10 * Ramp(t, 14, 22) / 256, RGB(26, 31, 22));
        }
        break;
    case MPFX_FORM_JUDGMENT:
        // shafts of golden light rain down over the field, closing in on its spot; the last strikes it full on, a
        // starburst flashes and it shines in the light
        if (t < 14 && t % 2 == 0)
            for (i = 0; i < Budget(2); i++)
            {
                s16 x = mx + Rnd(Dist(60) * (14 - t) / 14);
                if ((p = Spawn(i ? FXD_PL_BEAM : FXD_BW_RAY, 0, x, SkyY(b))))
                {
                    p->tint = RGB(31, 28, 14);
                    p->blend = BLEND_GLOW;
                    p->alpha = 12;
                    p->vy = 0x500;
                    p->life = 8;
                    p->fadeOut = 4;
                    Size(p, 30);
                    p->stretchX = 100;
                }
            }
        if (t >= 14 && t < 20 && (p = Spawn(FXD_BW_PILLAR, 0, mx, (feet + SkyY(b)) / 2)))
        {
            p->tint = RGB(31, 30, 18);
            p->blend = BLEND_GLOW;
            p->alpha = 14;
            p->life = 1;
            Size(p, feet - SkyY(b));
            p->stretchX = 160 - 20 * (t - 14);
        }
        if (t == 16 && (p = Spawn(FXD_BW_STARBURST, 0, mx, my)))
        {
            p->tint = RGB(31, 30, 20);
            p->blend = BLEND_GLOW;
            p->life = 10;
            p->fadeOut = 8;
            p->dRot = 4;
            Size(p, 40);
            p->dScale = p->scale / 10;
            PlaySE(SE_M_SWIFT);
        }
        s->invisible = t < 15;
        BodyTint(b, 16 - 16 * Ramp(t, 16, 26) / 256, RGB(31, 30, 20));
        break;
    case MPFX_FORM_VINES:
    {
        // vines and roots climb up from the ground around its spot, twisting together over its hidden shape;
        // then they slide back down into the ground and leaves flutter off it
        s32 up = t < 16 ? Ease(Ramp(t, 0, 10)) : 256 - Ease(Ramp(t, 16, 23));
        for (i = 0; i < 3; i++)
        {
            struct FxPart *v = PartAt(b->helper[i]);
            if (t == 0 && (v = Helper(b, i, i == 1 ? FXD_BW_ROOTS0 : FXA_MORE_VINE4, 0, mx, feet)))
            {
                v->flags = i == 2 ? PF_FLIPX : 0;
                v->tint = i == 1 ? RGB(18, 24, 12) : 0;
            }
            if (v)
            {
                s16 h = BodyHeight(b) * up / 256;
                Size(v, 56);
                v->stretchY = 40 + 216 * up / 256;
                v->x = (mx + (i - 1) * Dist(10)) << 8;
                v->y = (feet - h / 2) << 8;
                v->rot = (u8)(i == 1 ? 0 : (i ? 1 : -1) * 6);
            }
        }
        if (t == 16)
            for (i = 0; i < Budget(5); i++)
                if ((p = Spawn(FXD_PL_LEAF0 + (i & 1), 0, mx + Rnd(Dist(14)), my + Rnd(Dist(16)))))
                {
                    p->move = MOVE_FLUTTER;
                    p->vx = Rnd(0x180);
                    p->vy = -0x100;
                    p->ay = 0x18;
                    p->sway = 2;
                    p->dRot = 6;
                    p->life = 22;
                    p->fadeOut = 8;
                    Size(p, 8);
                }
        if (t == 0)
            PlaySE(SE_M_STRING_SHOT);
        s->invisible = t < 8;
        BodyTint(b, 14 - 14 * Ramp(t, 16, 24) / 256, RGB(10, 20, 8));
        break;
    }
    case MPFX_FORM_SPIRITFLAME:
        // ghostly blue flames circle its spot, flickering; it fades in pale and see-through among them, then the
        // flames are drawn into it and go out
        if (t < 18 && t % 2 == 0)
            for (i = 0; i < Budget(5); i++)
            {
                u8 a = (u8)(i * 51 + t * 10);
                if ((p = Spawn(i & 1 ? FXD_BW_BLUEFIRE0 : FXD_BW_WISPFIRE0, 0, mx + Cos(a, Dist(26)), my + Dist(6) + Sin(a, Dist(8)))))
                {
                    p->nFrames = i & 1 ? 3 : 4;
                    p->frameTime = 2;
                    p->alpha = 13;
                    p->life = 2;
                    p->flags = Sin(a, 1) < 0 ? PF_UNDER : 0;
                    Size(p, 14);
                }
            }
        if (t == 18)
            for (i = 0; i < Budget(5); i++)
                if ((p = Spawn(FXD_BW_BLUEFIRE0, 0, mx + Cos(i * 51, Dist(26)), my + Dist(6))))
                {
                    p->move = MOVE_HOME;
                    p->tx = mx;
                    p->ty = my;
                    p->life = 6;
                    p->fadeOut = 4;
                    p->blend = BLEND_GLOW;
                    Size(p, 12);
                    p->dScale = -p->scale / 8;
                }
        if (t == 0)
            PlaySE(SE_M_CONFUSE_RAY);
        s->invisible = t < 4;
        b->alpha = t < 18 ? 3 + 6 * Ramp(t, 4, 16) / 256 : 9 + 7 * Ramp(t, 18, 24) / 256;
        BodyTint(b, 12 - 12 * Ramp(t, 18, 26) / 256, RGB(16, 22, 31));
        break;
    case MPFX_FORM_ICESCULPT:
        // shards of ice gather onto its spot and freeze into its shape, a glittering ice sculpture; the ice melts,
        // dripping away, and its colors come through
        if (t < 10)
            for (i = 0; i < Budget(2); i++)
            {
                u8 a = RndU(256);
                if ((p = SettleAt(t & 1 ? FXD_BW_ICE0 + RndU(3) : FXD_PL_ICE0 + RndU(3), 0, mx + Cos(a, Dist(60)), my + Sin(a, Dist(46)),
                                  mx + Rnd(Dist(14)), my + Rnd(Dist(18)), 5, 16 - t)))
                {
                    p->tint = RGB(24, 29, 31);
                    p->dRot = 9;
                    p->alpha = 13;
                    Size(p, 9);
                }
            }
        if (t >= 10 && t < 16 && t % 2 == 0 && (p = Spawn(FXD_BW_SPARKLE, 0, mx + Rnd(Dist(14)), my + Rnd(Dist(18)))))
        {
            p->blend = BLEND_GLOW;
            p->life = 4;
            Size(p, 10);
        }
        if (t >= 16 && t < 24)
            for (i = 0; i < Budget(1); i++)
                if ((p = Spawn(FXD_PL_DROP, 0, mx + Rnd(Dist(16)), my + Rnd(Dist(16)))))
                {
                    p->tint = RGB(22, 28, 31);
                    p->vy = 0x80;
                    p->ay = 0x30;
                    p->life = 10;
                    p->fadeOut = 4;
                    Size(p, 4);
                }
        if (t == 2)
            PlaySE(SE_M_ICY_WIND);
        if (t == 16)
            PlaySE(SE_M_BUBBLE3);
        s->invisible = t < 8;
        BodyTint(b, t < 16 ? 15 : 15 - 15 * Ramp(t, 16, 25) / 256, RGB(24, 29, 31));
        break;
    case MPFX_FORM_RIBBON:
    {
        // three ribbons of color twirl round its spot, wrapping it like a gift; with a flourish they untie and fly
        // off, sparkles left behind, and it strikes a pose
        static const u16 sRibbon[3] = {RGB(31, 10, 14), RGB(10, 18, 31), RGB(31, 26, 6)};
        if (t < 16 && t % 2 == 0)
            for (i = 0; i < 3; i++)
            {
                u8 a = (u8)(t * 18 + i * 85);
                if ((p = Spawn(FXD_BW_HALFARC, 0, mx + Cos(a, Dist(24)), my + Dist(4) - (i - 1) * Dist(12) + Sin(a, Dist(6)))))
                {
                    p->tint = sRibbon[i];
                    p->rot = a + 64;
                    p->life = 3;
                    p->flags = Sin(a, 1) < 0 ? PF_UNDER : 0;
                    Size(p, 18);
                }
            }
        if (t == 16)
        {
            for (i = 0; i < 6; i++)
                if ((p = Spawn(FXD_BW_HALFARC, 0, mx, my)))
                {
                    p->tint = sRibbon[i % 3];
                    p->vx = Cos(i * 43, 0x300);
                    p->vy = Sin(i * 43, 0x200);
                    p->dRot = 16;
                    p->life = 12;
                    p->fadeOut = 6;
                    Size(p, 18);
                }
            for (i = 0; i < 5; i++)
                Twinkle(mx + Cos(i * 51, Dist(22)), my + Sin(i * 51, Dist(22)), i / 2);
            PlaySE(SE_M_SWAGGER);
        }
        s->invisible = t < 6;
        b->alpha = 4 + 12 * Ramp(t, 6, 16) / 256;
        if (t >= 16)
        {
            static const s16 sPose[] = {30, 10, -12, -4, 0};
            r = sPose[t - 16 < 5 ? t - 16 : 4];
            BodyScale(b, 256 - r / 2, 256 + r, TRUE);
        }
        BodyTint(b, 10 - 10 * Ramp(t, 14, 22) / 256, RGB_WHITE);
        break;
    }
    }
}

// ---------------------------------------------------------------------------
// Returns

static void MoreRecallFrame(struct FxBody *b)
{
    struct Sprite *s = &gSprites[b->mon];
    u8 t = b->t;
    u16 energy = sEnergyRgb[b->color];
    u16 light = MixRgb(energy, RGB_WHITE, 6);
    s16 mx = MonX(b), my = MonY(b), feet = Feet(b), head = Head(b);
    struct FxPart *p;
    s32 r, k;
    u8 i;

    (void)light;
    switch (b->id)
    {
    case MPFX_RECALL_ROCKTOMB:
        // rocks drop from above and pile up over it; the pile shudders and sinks into the ground with it
        for (i = 0; i < 6; i++)
            if (t == i * 2 && (p = Spawn(FXD_PL_ROCK0 + i % 5, 0, mx + Rnd(Dist(16)), SkyY(b))))
            {
                p->move = MOVE_SETTLE;
                p->tx = mx + ((i % 3) - 1) * Dist(12);
                p->ty = feet - Dist(8) - (i / 3) * Dist(14);
                p->dRadius = 6;
                p->life = 26 - i * 2;
                p->rot = RndU(256);
                Size(p, 20);
            }
        if (t >= 12 && t < 26 && t % 3 == 0)
            Puff(mx + Rnd(Dist(16)), feet - 2, Rnd(0x80), 140);
        if (t == 6)
            PlaySE(SE_M_ROCK_THROW);
        s->invisible = t >= 12;
        s->x2 = b->baseX2 + (t >= 8 && t < 12 ? ((t & 1) ? 1 : -1) : 0);
        BodyTint(b, 10 * Ramp(t, 4, 12) / 256, RGB(16, 12, 8));
        break;
    case MPFX_RECALL_SURF:
        // a wave rolls in from the right, crests over it and carries it off to the left in the spray
        if (t < 24)
            for (i = 0; i < Budget(2); i++)
            {
                s16 wx = mx + Dist(80) - Dist(170) * Ramp(t, 0, 24) / 256 + Rnd(Dist(8));
                if ((p = Spawn(i ? FXD_BW_WATERFALL : FXD_PL_DROP, 0, wx, feet - Dist(10) - RndU(Dist(18)))))
                {
                    p->vx = -0x200;
                    p->vy = -0x40;
                    p->life = 6;
                    p->fadeOut = 4;
                    p->alpha = 13;
                    p->rot = 64;   // a wave lying along the ground
                    Size(p, i ? 30 : 6);
                }
            }
        k = Ramp(t, 10, 24);
        s->x2 = b->baseX2 - Dist(110) * Ease(k) / 256;
        s->y2 = b->baseY2 - Sin(k / 2, Dist(10));
        BodyTurn(b, 256, 256, (u8)(-(s32)(24 * k / 256)));
        s->invisible = t >= 24;
        if (t == 0)
            PlaySE(SE_M_SURF);
        break;
    case MPFX_RECALL_VOLT:
        // it crackles, shrinks into a ball of lightning and zips back to the ball in a jagged line
        k = Ramp(t, 6, 22);
        if (t < 8 && (p = Spawn(FXD_BW_BOLT0 + RndU(4), 0, mx + Rnd(Dist(14)), my + Rnd(Dist(18)))))
        {
            p->tint = RGB(31, 31, 12);
            p->blend = BLEND_GLOW;
            p->life = 2;
            Size(p, 14);
        }
        if (t == 6 && (p = Helper(b, 0, FXD_BW_ORB, 0, mx, my)))
        {
            p->tint = RGB(31, 31, 10);
            p->blend = BLEND_GLOW;
            p->trailEvery = 1;
            p->trailArt = FXA_ENERGY_MOTE;
            p->color = MP_BALL_YELLOW;
            Size(p, 14);
        }
        if ((p = PartAt(b->helper[0])))
        {
            p->x = (mx + (b->bx - mx) * k / 256) << 8;
            p->y = (my + (b->by - my) * k / 256 + ((t & 2) ? Dist(5) : -Dist(5))) << 8;   // zigzagging
        }
        if (t == 22)
        {
            Kill(&b->helper[0]);
            Glow(MP_BALL_YELLOW, b->bx, b->by, FALSE, 8, 160, 20);
        }
        r = 256 - 230 * Ramp(t, 2, 7) / 256;
        BodyScale(b, r, r, FALSE);
        BodyTint(b, 16, RGB(31, 31, 12));
        s->invisible = t >= 7;
        if (t == 0)
            PlaySE(SE_M_THUNDERBOLT2);
        break;
    case MPFX_RECALL_SLEEP:
        // sleep powder sprinkles down on it; it nods off, Zs rising, and fades into a drift of spores
        if (t < 12 && (p = Spawn(FXD_PL_DOTS, 0, mx + Rnd(Dist(18)), head - Dist(8))))
        {
            p->tint = RGB(18, 31, 16);
            p->vy = 0xA0;
            p->life = 10;
            p->fadeOut = 6;
            Size(p, 14);
        }
        if (t == 8 || t == 14)
            if ((p = Spawn(t == 8 ? FXD_BW_Z : FXD_PL_Z, 0, mx + Dist(10), head)))
            {
                p->vy = -0xC0;
                p->vx = 0x40;
                p->sway = 1;
                p->life = 14;
                p->fadeOut = 8;
                Size(p, 10);
            }
        s->y2 = b->baseY2 + (t >= 6 ? Sin(((t - 6) * 20) & 0xFF, 2) + 2 : 0);   // nodding
        BodyTurn(b, 256, 256, (u8)(t >= 6 ? Sin(((t - 6) * 20) & 0xFF, 5) : 0));
        b->alpha = 16 - 15 * Ramp(t, 14, 26) / 256;
        BodyTint(b, 6 * Ramp(t, 10, 24) / 256, RGB(20, 31, 18));
        if (t >= 16 && t % 2 == 0 && (p = Spawn(FXD_PL_DOTS, 0, mx + Rnd(Dist(16)), my + Rnd(Dist(16)))))
        {
            p->tint = RGB(22, 31, 18);
            p->vx = -0xA0;
            p->vy = -0x30;
            p->life = 12;
            p->fadeOut = 8;
            Size(p, 12);
        }
        s->invisible = t >= 26;
        if (t == 2)
            PlaySE(SE_M_POISON_POWDER);
        break;
    case MPFX_RECALL_DECOY:
    {
        // a puff of smoke: it's gone and a doll stands in its place, which flops over and vanishes in a puff
        struct FxPart *doll = PartAt(b->helper[0]);
        if (t == 2)
        {
            PartSmoke(mx, my, 8, 20, 12);
            s->invisible = TRUE;
            if ((doll = Helper(b, 0, (b->flags & FLAG_FRONT) ? FXD_BW_DOLL : FXA_MORE_DOLLBACK, 0, mx, my + Dist(4))))
                Size(doll, (b->flags & FLAG_FRONT) ? 60 : 104);
            PlaySE(SE_M_HAZE);
        }
        if (doll && t >= 12)
        {
            doll->rot = (u8)(-(s32)(64 * Ease(Ramp(t, 12, 18)) / 256));   // flops over to the left
            doll->y = (my + Dist(4) + Dist(14) * Ramp(t, 12, 18) / 256) << 8;
        }
        if (t == 22)
        {
            Kill(&b->helper[0]);
            PartSmoke(mx - Dist(10), feet - Dist(6), 5, 16, 8);
        }
        s->invisible = t >= 2;
        break;
    }
    case MPFX_RECALL_KINESIS:
    {
        // a spoon appears over it and bends; it floats up, glowing pink, and is drawn back to the ball
        if (t == 0 && (p = Helper(b, 0, FXD_BW_SPOON0, 0, mx + Dist(16), head - Dist(10))))
        {
            p->nFrames = 4;
            p->frameTime = 3;
            p->flags = PF_ONCE;
            p->fadeIn = 3;
            Size(p, 18);
        }
        if (t == 14)
            Kill(&b->helper[0]);
        k = Ramp(t, 14, 26);
        s->y2 = b->baseY2 - Dist(12) * Ease(Ramp(t, 4, 14)) / 256 + (b->by - my) * k / 256;
        s->x2 = b->baseX2 + (b->bx - mx) * Ease(k) / 256;
        r = 256 - 200 * k / 256;
        BodyScale(b, r, r, FALSE);
        s->y2 += -Dist(12) * Ease(Ramp(t, 4, 14)) / 256 + (b->by - my) * k / 256;
        BodyTint(b, 12 * Ramp(t, 4, 14) / 256, RGB(28, 14, 30));
        if (t >= 6 && t % 3 == 0)
            Ring(MP_BALL_PINK, FALSE, BodyX(b), BodyY(b), 8, 60, 20, 256);
        s->invisible = t >= 26;
        if (t == 4)
            PlaySE(SE_M_PSYBEAM2);
        break;
    }
    case MPFX_RECALL_EMBERS:
        // it glows red-hot from the feet up and comes apart into embers that drift back to the ball
        BodyTint(b, 16 * Ramp(t, 0, 10) / 256, RGB(31, 14, 4));
        if (t >= 8 && t < 22)
            for (i = 0; i < Budget(3); i++)
                if ((p = Spawn(FXD_BW_FLAME0 + RndU(3), 0, mx + Rnd(Dist(16)), my + Rnd(Dist(20)))))
                {
                    p->move = MOVE_HOME;
                    p->tx = b->bx + Rnd(4);
                    p->ty = b->by + Rnd(4);
                    p->life = 12 + RndU(6);
                    p->fadeOut = 6;
                    p->sway = 1;
                    Size(p, 6 + RndU(4));
                }
        b->alpha = 16 - 15 * Ramp(t, 10, 22) / 256;
        s->invisible = t >= 22;
        if (t == 2)
            PlaySE(SE_M_EMBER);
        break;
    case MPFX_RECALL_DIG:
        // it spins, dirt flying, and digs straight down into the ground
        k = Ramp(t, 6, 20);
        s->mpClipOn = TRUE;
        b->clipped = TRUE;
        s->mpClipY = feet + 1;
        BodyTurn(b, 256, 256, (u8)(t < 6 ? 0 : (t * 32) & 0xFF) * 0);
        s->x2 = b->baseX2 + (t >= 4 && t < 20 ? ((t & 1) ? 2 : -2) : 0);
        s->y2 = b->baseY2 + BodyHeight(b) * Ease(k) / 256;
        if (t >= 4 && t < 22 && t % 2 == 0)
        {
            for (i = 0; i < Budget(2); i++)
                if ((p = Spawn(FXD_BW_PEBBLE, 0, mx + Rnd(Dist(10)), feet - 2)))
                {
                    p->vx = (i ? 1 : -1) * (0x100 + RndU(0x100));
                    p->vy = -0x200 - RndU(0x100);
                    p->ay = 0x50;
                    p->life = 12;
                    p->fadeOut = 4;
                    p->dRot = 12;
                    Size(p, 5);
                }
            Puff(mx + Rnd(Dist(12)), feet - 3, Rnd(0x100), 130);
        }
        s->invisible = t >= 21;
        if (t == 4)
            PlaySE(SE_M_DIG);
        break;
    case MPFX_RECALL_FLY:
        // two great wings spread from its back; it lifts off and flies away to the upper left, a feather drifting
        k = Ramp(t, 8, 24);
        for (i = 0; i < 2; i++)
        {
            struct FxPart *w = PartAt(b->helper[i]);
            if (t == 2 && (w = Helper(b, i, FXD_PL_WING0 + i, 0, mx, my)))
            {
                w->flags = i ? 0 : PF_FLIPX;
                w->fadeIn = 3;
            }
            if (w)
            {
                s16 flap = Sin((t * 40) & 0xFF, 40);
                w->x = (BodyX(b) + (i ? Dist(16) : -Dist(16))) << 8;
                w->y = (BodyY(b) - Dist(4)) << 8;
                w->stretchY = 200 + flap;
                Size(w, 26 * (256 - k / 2) / 256);
                w->stretchY = 200 + flap;
            }
        }
        s->x2 = b->baseX2 - Dist(120) * k * k / 65536;
        s->y2 = b->baseY2 - Dist(100) * k * k / 65536 - (t < 8 ? Sin(Ramp(t, 2, 8) / 2, Dist(4)) : 0);
        r = 256 - 150 * k / 256;
        BodyScale(b, r, r, FALSE);
        s->x2 = b->baseX2 - Dist(120) * k * k / 65536;
        s->y2 = b->baseY2 - Dist(100) * k * k / 65536;
        if (t == 12 && (p = Spawn(FXD_BW_FEATHER, 0, mx, my)))
        {
            p->move = MOVE_FLUTTER;
            p->vy = 0x40;
            p->sway = 2;
            p->life = 28;
            p->fadeOut = 10;
            Size(p, 10);
        }
        s->invisible = t >= 24;
        if (t >= 24)
        {
            Kill(&b->helper[0]);
            Kill(&b->helper[1]);
        }
        if (t == 2)
            PlaySE(SE_M_WING_ATTACK);
        break;
    case MPFX_RECALL_MIRROR:
        // it turns to glass, a crack runs across it and it shatters; the shards spin away to the ball
        BodyTint(b, 14 * Ramp(t, 0, 8) / 256, RGB(26, 30, 31));
        b->alpha = t < 12 ? 16 - 6 * Ramp(t, 2, 10) / 256 : 10;
        if (t == 8 && (p = Spawn(FXD_PL_GLASS, 0, mx, my)))
        {
            p->life = 6;
            p->blend = BLEND_GLOW;
            Size(p, 26);
        }
        if (t == 12)
        {
            for (i = 0; i < Budget(10); i++)
                if ((p = Spawn(i & 1 ? FXD_BW_GLASS : FXD_PL_CRYSTAL, 0, mx + Rnd(Dist(14)), my + Rnd(Dist(18)))))
                {
                    p->move = MOVE_HOME;
                    p->tx = b->bx;
                    p->ty = b->by;
                    p->life = 10 + i;
                    p->dRot = (i & 1) ? 12 : -12;
                    p->fadeOut = 4;
                    p->tint = RGB(26, 30, 31);
                    Size(p, 8);
                }
            PlaySE(SE_ICE_BREAK);
        }
        s->invisible = t >= 12;
        break;
    case MPFX_RECALL_BLOB:
    {
        // it goes soft and pink, melts into a round blob and bounces back to the ball in three hops
        s32 hop = 0;
        k = Ramp(t, 10, 26);
        BodyTint(b, 16 * Ramp(t, 0, 8) / 256, RGB(31, 20, 28));
        if (t < 10)
        {
            r = 50 * Ramp(t, 2, 10) / 256;
            BodyScale(b, 256 + r, 256 - r * 2, TRUE);
        }
        else
        {
            hop = Sin(((k * 3) / 2) & 0xFF, Dist(14));
            if (hop < 0)
                hop = -hop;
            r = 256 - 160 * k / 256;
            BodyScale(b, r * 300 / 256, r * 150 / 256, TRUE);
            s->x2 = b->baseX2 + (b->bx - mx) * k / 256;
            s->y2 += (b->by - feet) * k / 256 - hop;
        }
        s->invisible = t >= 26;
        if (t == 2)
            PlaySE(SE_M_ACID_ARMOR);
        break;
    }
    case MPFX_RECALL_SHEET:
    {
        // a white sheet drops over it like a ghost costume; it flutters, then falls flat - empty
        struct FxPart *sheet = PartAt(b->helper[0]);
        if (t == 0 && (sheet = Helper(b, 0, FXA_MORE_SHEET, 0, mx, head - Dist(30))))
            Size(sheet, BodyHeight(b) * 3 / 4);
        if (sheet)
        {
            s32 drop = Ease(Ramp(t, 0, 6));
            sheet->y = (head - Dist(30) + (my - head + Dist(30)) * drop / 256) << 8;
            sheet->rot = (u8)(t >= 6 && t < 16 ? Sin(((t - 6) * 40) & 0xFF, 6) : 0);
            if (t >= 16)
            {
                sheet->stretchY = 256 - 220 * Ease(Ramp(t, 16, 20)) / 256;   // falls flat
                sheet->y = (feet - Dist(4) - Dist(14) * (256 - Ramp(t, 16, 20)) / 256) << 8;
            }
            if (t >= 22)
                sheet->alpha = 16 - 16 * Ramp(t, 22, 27) / 256;
        }
        s->invisible = t >= 6;
        if (t == 4)
            PlaySE(SE_M_CONFUSE_RAY);
        break;
    }
    case MPFX_RECALL_SILVERWIND:
        // a silver gust blows in, lifting it; it spins up and away to the left among silver scales
        k = Ramp(t, 4, 24);
        if (t < 22 && t % 2 == 0 && (p = Spawn(FXD_BW_WIND, 0, mx + Dist(30), my + Rnd(Dist(16)))))
        {
            p->tint = RGB(26, 28, 31);
            p->blend = BLEND_GLOW;
            p->vx = -0x400;
            p->life = 10;
            p->fadeOut = 4;
            p->flags = PF_FLIPX;
            Size(p, 20);
        }
        if (t >= 4 && t % 3 == 0 && (p = Spawn(FXD_BW_SILVERFEATHER, 0, BodyX(b), BodyY(b))))
        {
            p->vx = -0x200;
            p->vy = -0x80;
            p->dRot = 10;
            p->life = 14;
            p->fadeOut = 6;
            Size(p, 8);
        }
        r = 256 - 180 * k / 256;
        BodyTurn(b, r, r, (u8)(-(s32)(512 * k / 256)));
        s->x2 = b->baseX2 - Dist(110) * Ease(k) / 256;
        s->y2 = b->baseY2 - Dist(40) * Ease(k) / 256;
        s->invisible = t >= 24;
        if (t == 0)
            PlaySE(SE_M_GUST);
        break;
    case MPFX_RECALL_RAINBOWRISE:
        // a rainbow-tinted feather circles it; it rises straight up on a column of rainbow light, shrinking
        k = Ramp(t, 8, 26);
        if (t < 10 && (p = Spawn(FXD_PL_FEATHER, 0, mx + Cos((u8)(t * 26), Dist(22)), my + Sin((u8)(t * 26), Dist(8)))))
        {
            p->tint = MixRgb(Hue((u8)(t * 25)), RGB_WHITE, 6);
            p->life = 4;
            p->fadeOut = 3;
            Size(p, 12);
        }
        if (t >= 6 && (p = Spawn(FXD_BW_PILLAR, 0, mx, my - Dist(30))))
        {
            p->tint = Hue((u8)(t * 14));
            p->blend = BLEND_GLOW;
            p->alpha = 8;
            p->life = 2;
            Size(p, 80);
            p->stretchX = 90;
        }
        r = 256 - 170 * k / 256;
        BodyScale(b, r, r, FALSE);
        s->y2 = b->baseY2 - Dist(110) * Ease(k) / 256;
        BodyTint(b, 12 * Ramp(t, 6, 14) / 256, MixRgb(Hue((u8)(t * 14)), RGB_WHITE, 8));
        s->invisible = t >= 26;
        if (t == 6)
            PlaySE(SE_M_MORNING_SUN);
        break;
    case MPFX_RECALL_EXTREMESPEED:
        // it crouches, then blurs: afterimages trail behind as it streaks off to the left
        if (t < 6)
        {
            r = 40 * Ramp(t, 0, 5) / 256;
            BodyScale(b, 256 + r, 256 - r, TRUE);
        }
        else
        {
            k = Ramp(t, 6, 13);
            BodyScale(b, 256 + 160 * k / 256, 256 - 40 * k / 256, TRUE);
            s->x2 = b->baseX2 - Dist(160) * k * k / 65536;
            for (i = 1; i <= 2; i++)
                if ((p = Spawn(FXD_BW_SPEED, 0, BodyX(b) + i * Dist(16), BodyY(b) + Rnd(Dist(14)))))
                {
                    p->tint = light;
                    p->blend = BLEND_GLOW;
                    p->life = 4;
                    p->fadeOut = 4;
                    Size(p, 30);
                    p->stretchY = 40;
                }
            BodyTint(b, 8, light);
        }
        if (t == 6)
        {
            Puff(mx + Dist(10), feet - 3, 0x200, 180);
            PlaySE(SE_M_DOUBLE_TEAM);
        }
        s->invisible = t >= 13;
        break;
    case MPFX_RECALL_TIMEWARP:
    {
        // green rings spin up round it, faster and faster; it stretches thin, flickering, and is pulled into the
        // past through their middle
        struct FxPart *ring = PartAt(b->helper[0]);
        if (t == 0 && (ring = Helper(b, 0, FXD_BW_WARP, 0, mx, my)))
        {
            ring->tint = RGB(12, 28, 12);
            ring->blend = BLEND_GLOW;
            ring->alpha = 10;
        }
        if (ring)
        {
            ring->rot = (u8)((s32)t * t * 3);
            Size(ring, 40 + 30 * Ramp(t, 0, 14) / 256 - 60 * Ramp(t, 18, 26) / 256);
        }
        k = Ramp(t, 12, 24);
        BodyScale(b, 256 - 230 * k / 256, 256 + 120 * k / 256, FALSE);
        s->invisible = t >= 24 || (t >= 8 && t % 3 == 0);
        BodyTint(b, 12 * Ramp(t, 4, 14) / 256, RGB(18, 31, 16));
        if (t == 24)
            Ring(MP_BALL_LIME, TRUE, mx, my, 8, 120, -10, 256);
        if (t == 0)
            PlaySE(SE_M_TELEPORT);
        break;
    }
    case MPFX_RECALL_GLYPHS:
        // it breaks into glyphs - letters scattering off it, turning and fading as they float away
        b->alpha = 16 - 15 * Ramp(t, 4, 16) / 256;
        BodyTint(b, 12 * Ramp(t, 0, 8) / 256, RGB(4, 4, 6));
        if (t >= 4 && t < 18 && t % 2 == 0)
            for (i = 0; i < Budget(2); i++)
                if ((p = Spawn(FXD_PL_LETTER0 + RndU(26), 0, mx + Rnd(Dist(16)), my + Rnd(Dist(20)))))
                {
                    p->tint = RGB(9, 9, 12);
                    p->vx = -0x60 - RndU(0x80);
                    p->vy = -0x80 - RndU(0x60);
                    p->dRot = (i & 1) ? 6 : -6;
                    p->life = 16;
                    p->fadeOut = 10;
                    Size(p, 10);
                }
        s->invisible = t >= 18;
        if (t == 2)
            PlaySE(SE_M_DETECT);
        break;
    case MPFX_RECALL_BELLTOLL:
        // a bell tolls three times above it; with each toll it fades a little more, until it's gone
        if (t == 0 && (p = Helper(b, 0, FXD_BW_BELL, 0, mx, head - Dist(18))))
        {
            p->fadeIn = 3;
            Size(p, 18);
        }
        if ((p = PartAt(b->helper[0])))
        {
            p->rot = (u8)Sin((t * 32) & 0xFF, 14);
            if (t >= 22)
                p->alpha = 16 - 16 * Ramp(t, 22, 26) / 256;
        }
        if (t == 4 || t == 10 || t == 16)
        {
            if ((p = Spawn(FXD_PL_RINGS, 0, mx, head - Dist(18))))
            {
                p->tint = RGB(31, 26, 10);
                p->blend = BLEND_GLOW;
                p->alpha = 9;
                p->life = 12;
                p->fadeOut = 10;
                Size(p, 12);
                p->dScale = p->scale / 4;
            }
            PlaySE(SE_M_HEAL_BELL);
        }
        b->alpha = t < 4 ? 16 : t < 10 ? 11 : t < 16 ? 6 : 6 - 6 * Ramp(t, 16, 20) / 256;
        s->invisible = t >= 20;
        break;
    case MPFX_RECALL_EGG:
    {
        // an eggshell closes round it, top and bottom; the EGG wobbles, then rolls along the ground to the ball
        struct FxPart *egg = PartAt(b->helper[0]);
        if (t < 8)
            for (i = 0; i < 2; i++)
                if ((p = Spawn(FXA_MORE_SHELL, 0, mx, my + (i ? 1 : -1) * Dist(24) * (256 - Ease(Ramp(t, 0, 7))) / 256)))
                {
                    p->life = 1;
                    p->flags = i ? 0 : 0;
                    p->rot = i ? 128 : 0;
                    Size(p, 34);
                }
        if (t == 8)
        {
            s->invisible = TRUE;
            if ((egg = Helper(b, 0, FXD_PL_EGG, 0, mx, feet - Dist(16))))
                Size(egg, 32);
            PlaySE(SE_BALL_BOUNCE_1);
        }
        if (egg && t >= 8)
        {
            k = Ramp(t, 14, 26);
            egg->rot = (u8)(t < 14 ? Sin(((t - 8) * 48) & 0xFF, 14) : -(s32)(384 * k / 256));
            egg->x = (mx + (b->bx - mx) * Ease(k) / 256) << 8;
            egg->y = (feet - Dist(16) + (b->by - feet) * k / 256) << 8;
            Size(egg, 32 - 22 * k / 256);
        }
        if (t == 26)
            Kill(&b->helper[0]);
        break;
    }
    case MPFX_RECALL_DRAGONCOIL:
        // it turns to light and coils into a dragon - a glowing body behind a crescent head - that spirals up and
        // away off the top of the screen
        k = Ramp(t, 6, 26);
        BodyTint(b, 16 * Ramp(t, 0, 6) / 256, light);
        r = 256 - 230 * Ramp(t, 4, 8) / 256;
        BodyScale(b, r, r, FALSE);
        s->invisible = t >= 8;
        if (t >= 6)
        {
            s16 hx = mx + Cos((u8)(k * 2), Dist(20)), hy = my - Dist(120) * k / 256;
            if ((p = Spawn(FXD_BW_GLOW, 0, hx, hy)))
            {
                p->tint = TINT_ENERGY;
            p->color = b->color;
                p->blend = BLEND_GLOW;
                p->life = 10;
                p->fadeOut = 8;
                Size(p, 12);
                p->dScale = -p->scale / 12;
            }
            if ((p = Spawn(FXD_BW_CLAW0 + (t / 2) % 5, 0, hx, hy)))
            {
                p->tint = light;
                p->blend = BLEND_GLOW;
                p->life = 2;
                p->rot = (u8)(k * 2 + 64);
                Size(p, 16);
            }
        }
        if (t == 4)
            PlaySE(SE_M_DRAGON_RAGE);
        break;
    case MPFX_RECALL_STATIC:
        // the picture breaks up: static rolls over it, it jumps and fuzzes, and the signal fades out
        if (t < 20 && (p = Spawn(t & 1 ? FXD_PL_NOISE : FXD_BW_NOISE, 0, mx, my)))
        {
            p->tint = RGB(22, 24, 26);
            p->alpha = 1 + 4 * Ramp(t, 0, 16) / 256;
            p->life = 1;
            Size(p, BodyHeight(b) * 3 / 4);
        }
        s->oam.mosaic = TRUE;
        sScreen.mosaicWant = 8 * Ramp(t, 4, 20) / 256;
        s->x2 = b->baseX2 + ((t % 4) == 1 ? 3 : (t % 5) == 3 ? -2 : 0);
        s->invisible = t >= 20 || (t >= 10 && (t % 3) == 0);
        b->alpha = 16 - 12 * Ramp(t, 8, 20) / 256;
        if (t >= 20)
        {
            s->oam.mosaic = FALSE;
            sScreen.mosaicWant = 0;
        }
        if (t == 0)
            PlaySE(SE_PC_OFF);
        break;
    case MPFX_RECALL_KARPHOP:
    {
        // it flops on the spot twice, splashing, then leaps away to the left in a big arc into the ball
        s16 feetNow = feet;
        if (t < 12)
        {
            s32 a = Ramp(t % 6, 0, 5);
            BodyTurn(b, 256, 256, (u8)((t / 6) & 1 ? 20 : -20));
            s->y2 = b->baseY2 - Dist(10) * (4 * a * (256 - a) / 256) / 256;
            if (t % 6 == 0 && (p = Spawn(FXA_WATER_SPLASH, 0, mx, feetNow - 6)))
            {
                p->life = 6;
                p->fadeOut = 4;
                Size(p, 22);
                p->stretchY = 120;
            }
        }
        else
        {
            k = Ramp(t, 12, 26);
            r = 256 - 180 * k / 256;
            BodyTurn(b, r, r, (u8)(-(s32)(256 * k / 256)));
            s->x2 = b->baseX2 + (b->bx - mx) * k / 256;
            s->y2 = b->baseY2 + (b->by - my) * k / 256 - Dist(40) * (4 * k * (256 - k) / 256) / 256;
        }
        if (t == 0 || t == 6)
            PlaySE(SE_M_DIVE);
        s->invisible = t >= 26;
        break;
    }
    case MPFX_RECALL_BUGNET:
    {
        // a bug net swoops down from the upper right over it; it's caught, and the net is whisked away to the
        // left
        struct FxPart *net = PartAt(b->helper[0]);
        if (t == 0 && (net = Helper(b, 0, FXA_MORE_NET, 0, mx + Dist(60), my - Dist(60))))
            Size(net, 44);
        if (net)
        {
            if (t < 8)
            {
                k = Ease(Ramp(t, 0, 7));
                net->x = (mx + Dist(60) - Dist(56) * k / 256) << 8;
                net->y = (my - Dist(60) + Dist(54) * k / 256) << 8;
                net->rot = (u8)(-(s32)(40 * k / 256));
            }
            else if (t >= 12)
            {
                k = Ease(Ramp(t, 12, 24));
                net->x = (mx + Dist(4) - Dist(130) * k / 256) << 8;
                net->y = (my - Dist(6) - Dist(20) * k / 256) << 8;
            }
        }
        s->invisible = t >= 9;
        if (t == 8)
        {
            Puff(mx, my, 0, 160);
            PlaySE(SE_M_VITAL_THROW);
        }
        if (t == 26)
            Kill(&b->helper[0]);
        break;
    }
    case MPFX_RECALL_BLIZZARD:
        // a blizzard sweeps in from the right; it whitens, frosting over, and is gone with the snow to the left
        if (t < 24)
            for (i = 0; i < Budget(3); i++)
                if ((p = Spawn(i == 2 ? FXD_BW_SNOWFLAKE : FXA_SNOW_FLAKE0 + (i & 1), 0, mx + Dist(70) + Rnd(Dist(20)), my + Rnd(Dist(34)))))
                {
                    p->vx = -0x500 - RndU(0x100);
                    p->vy = 0x60;
                    p->dRot = 10;
                    p->life = 14;
                    p->fadeOut = 4;
                    Size(p, i == 2 ? 8 : 6);
                }
        BodyTint(b, 16 * Ramp(t, 2, 14) / 256, RGB(28, 30, 31));
        k = Ramp(t, 12, 24);
        s->x2 = b->baseX2 - Dist(100) * Ease(k) / 256;
        b->alpha = 16 - 14 * k / 256;
        s->invisible = t >= 24;
        if (t == 0)
            PlaySE(SE_M_BLIZZARD);
        break;
    case MPFX_RECALL_SHADOWFORCE:
        // a dark rift opens in the ground under it; it sinks into it, and the rift closes behind it
        if (t == 0 && (p = Helper(b, 0, FXD_BW_DARKDOTS, 0, mx, feet)))
        {
            p->tint = RGB(10, 4, 14);
            p->flags = PF_UNDER;
            p->underSprite = b->mon;
            p->dRot = -6;
        }
        if ((p = PartAt(b->helper[0])))
        {
            Size(p, 10 + 44 * (t < 18 ? Ease(Ramp(t, 0, 6)) : 256 - Ease(Ramp(t, 18, 26))) / 256);
            p->stretchY = 70;
        }
        s->mpClipOn = TRUE;
        b->clipped = TRUE;
        s->mpClipY = feet + 1;
        s->y2 = b->baseY2 + BodyHeight(b) * Ease(Ramp(t, 4, 18)) / 256;
        BodyTint(b, 14 * Ramp(t, 0, 10) / 256, RGB(8, 3, 12));
        if (t >= 4 && t < 18 && t % 3 == 0 && (p = Spawn(FXD_BW_STATICBALL, 0, mx + Rnd(Dist(16)), feet - 2)))
        {
            p->tint = RGB(10, 4, 16);
            p->vy = -0x100;
            p->life = 10;
            p->fadeOut = 6;
            Size(p, 8);
        }
        s->invisible = t >= 18;
        if (t == 2)
            PlaySE(SE_M_FAINT_ATTACK);
        break;
    case MPFX_RECALL_SPACIALREND:
        // a pink slash cuts the air beside it, opening a tear; space pulls it in sideways and the tear seals
        if (t >= 2 && t < 22 && (p = Spawn(FXD_BW_PILLAR, 0, mx - Dist(20), my)))
        {
            p->tint = RGB(31, 14, 26);
            p->blend = BLEND_GLOW;
            p->alpha = 14;
            p->life = 1;
            Size(p, BodyHeight(b));
            p->stretchX = t < 6 ? 30 : t < 18 ? 30 + 100 * Ease(Ramp(t, 6, 9)) / 256 : 130 - 120 * Ramp(t, 18, 22) / 256;
            p->stretchY = 256 * Ease(Ramp(t, 2, 5)) / 256 + 6;
        }
        k = Ramp(t, 8, 18);
        BodyScale(b, 256 - 240 * k / 256, 256 - 60 * k / 256, FALSE);
        s->x2 = b->baseX2 - Dist(20) * Ease(k) / 256;
        BodyTint(b, 12 * k / 256, RGB(31, 20, 28));
        s->invisible = t >= 18;
        if (t == 22)
            Ring(MP_BALL_PINK, TRUE, mx - Dist(20), my, 8, 60, 30, 256);
        if (t == 2)
            PlaySE(SE_M_RAZOR_WIND);
        break;
    case MPFX_RECALL_LAKEMIST:
    {
        // mist rolls in around it; three lights circle it and it fades into the mist with them, the lights
        // rising away
        static const u16 sLight[3] = {RGB(31, 30, 10), RGB(31, 18, 26), RGB(12, 22, 31)};
        if (t < 14 && t % 2 == 0 && (p = Spawn(FXD_PL_MIST, 0, mx + Rnd(Dist(24)), my + Dist(8) + Rnd(Dist(10)))))
        {
            p->tint = RGB(28, 30, 31);
            p->alpha = 7;
            p->vx = Rnd(0x60);
            p->life = 18;
            p->fadeIn = 4;
            p->fadeOut = 8;
            Size(p, 30);
        }
        for (i = 0; i < 3; i++)
            if (t >= 4 && (p = Spawn(FXD_BW_ORB, 0, mx + Cos((u8)(i * 85 + t * 16), Dist(22)),
                                     my - Dist(30) * Ramp(t, 16, 26) / 256 + Sin((u8)(i * 85 + t * 16), Dist(7)))))
            {
                p->tint = sLight[i];
                p->blend = BLEND_GLOW;
                p->life = 1;
                Size(p, 8);
            }
        b->alpha = 16 - 15 * Ramp(t, 6, 18) / 256;
        BodyTint(b, 10 * Ramp(t, 4, 14) / 256, RGB(26, 28, 31));
        s->invisible = t >= 18;
        if (t == 2)
            PlaySE(SE_M_MIST);
        break;
    }
    case MPFX_RECALL_LUNARDANCE:
        // it twirls in moonlight, glowing pale, and shrinks into a crescent that sinks below the ground line
        k = Ramp(t, 4, 18);
        BodyTurn(b, 256 - 230 * k / 256, 256 - 200 * k / 256, (u8)(512 * k / 256));
        BodyTint(b, 16 * Ramp(t, 0, 8) / 256, RGB(26, 26, 31));
        s->invisible = t >= 18;
        if (t >= 6 && t % 3 == 0 && (p = Spawn(FXD_PL_TWINKLE, 0, mx + Rnd(Dist(20)), my + Rnd(Dist(20)))))
        {
            p->tint = RGB(26, 26, 31);
            p->blend = BLEND_GLOW;
            p->life = 8;
            p->fadeOut = 6;
            Size(p, 8);
        }
        if (t == 16 && (p = Helper(b, 0, FXD_PL_CRESCENT, 0, mx, my)))
        {
            p->tint = RGB(28, 28, 31);
            p->blend = BLEND_GLOW;
            Size(p, 18);
        }
        if ((p = PartAt(b->helper[0])))
        {
            p->y = (my + (feet - my + Dist(10)) * Ramp(t, 18, 27) / 256) << 8;
            p->alpha = 16 - 16 * Ramp(t, 22, 27) / 256;
        }
        if (t == 2)
            PlaySE(SE_M_MOONLIGHT);
        break;
    case MPFX_RECALL_DARKVOID:
        // a black void spreads under it; it nods off - Zs rising - and sinks into the darkness
        if (t == 0 && (p = Helper(b, 0, FXD_BW_STATICBALL, 0, mx, feet)))
        {
            p->tint = RGB(4, 2, 8);
            p->flags = PF_UNDER;
            p->underSprite = b->mon;
        }
        if ((p = PartAt(b->helper[0])))
        {
            Size(p, 10 + 46 * (t < 20 ? Ease(Ramp(t, 0, 8)) : 256 - Ease(Ramp(t, 20, 27))) / 256);
            p->stretchY = 60;
        }
        if (t == 8 && (p = Spawn(FXD_BW_Z, 0, mx + Dist(10), head)))
        {
            p->vy = -0xA0;
            p->sway = 1;
            p->life = 16;
            p->fadeOut = 8;
            Size(p, 10);
        }
        s->mpClipOn = TRUE;
        b->clipped = TRUE;
        s->mpClipY = feet + 1;
        s->y2 = b->baseY2 + BodyHeight(b) * Ease(Ramp(t, 10, 20)) / 256;
        BodyTint(b, 12 * Ramp(t, 2, 12) / 256, RGB(6, 3, 10));
        s->invisible = t >= 20;
        if (t == 2)
            PlaySE(SE_M_HAZE);
        break;
    case MPFX_RECALL_SKYFORM:
        // flower petals swirl up around it and lift it into the sky, petals trailing after it
        k = Ramp(t, 8, 26);
        if (t < 24)
            for (i = 0; i < Budget(2); i++)
                if ((p = Spawn(FXD_BW_PETAL0 + (i & 1), 0, BodyX(b) + Cos((u8)(t * 30 + i * 128), Dist(20)), BodyY(b) + Dist(10))))
                {
                    p->tint = (t + i) & 1 ? RGB(31, 24, 28) : 0;
                    p->vy = -0x180;
                    p->dRot = 9;
                    p->life = 10;
                    p->fadeOut = 6;
                    Size(p, 9);
                }
        r = 256 - 160 * k / 256;
        BodyScale(b, r, r, FALSE);
        s->y2 = b->baseY2 - Dist(120) * Ease(k) / 256;
        s->x2 = b->baseX2 + Sin((t * 20) & 0xFF, 3);
        BodyTint(b, 8 * Ramp(t, 4, 12) / 256, RGB(31, 26, 28));
        s->invisible = t >= 26;
        if (t == 2)
            PlaySE(SE_M_PETAL_DANCE);
        break;
    case MPFX_RECALL_ASCEND:
        // golden rings descend around it and lift it straight up into a shaft of light
        if (t < 20 && t % 3 == 0 && (p = Spawn(FXD_PL_RING, 0, mx, head - Dist(10))))
        {
            p->tint = RGB(31, 28, 12);
            p->blend = BLEND_GLOW;
            p->stretchY = 70;
            p->vy = 0x180;
            p->life = 12;
            p->fadeOut = 6;
            Size(p, 34);
        }
        if (t >= 10 && (p = Spawn(FXD_BW_PILLAR, 0, mx, my - Dist(40))))
        {
            p->tint = RGB(31, 30, 20);
            p->blend = BLEND_GLOW;
            p->alpha = 9;
            p->life = 1;
            Size(p, 100);
            p->stretchX = 130;
        }
        k = Ramp(t, 10, 26);
        s->y2 = b->baseY2 - Dist(120) * k * k / 65536;
        BodyTint(b, 14 * Ramp(t, 6, 16) / 256, RGB(31, 30, 20));
        s->invisible = t >= 26;
        if (t == 8)
            PlaySE(SE_M_SWIFT);
        break;
    case MPFX_RECALL_VINEPULL:
    {
        // vines lash out from the left, coil round it and pull it back to the ball
        struct FxPart *vine = PartAt(b->helper[0]);
        if (t == 0 && (vine = Helper(b, 0, FXA_MORE_VINE4, 0, b->bx, my)))
            vine->rot = 64;
        k = Ramp(t, 12, 26);
        if (vine)
        {
            s16 x0 = b->bx, x1 = BodyX(b);
            vine->x = ((x0 + x1) / 2) << 8;
            vine->y = BodyY(b) << 8;
            Size(vine, 30);
            vine->stretchY = (x1 - x0 > 4 ? (x1 - x0) * 256 / 30 : 30) * Ease(Ramp(t, 0, 6)) / 256 + 8;
            vine->rot = 64;
        }
        if (t >= 6 && t < 12 && (p = Spawn(FXD_BW_ROOT0 + (t & 1), 0, mx, my + Rnd(Dist(10)))))
        {
            p->life = 2;
            Size(p, 24);
        }
        s->x2 = b->baseX2 + (b->bx - mx) * Ease(k) / 256;
        r = 256 - 190 * k / 256;
        BodyScale(b, r, r, FALSE);
        s->x2 = b->baseX2 + (b->bx - mx) * Ease(k) / 256;
        BodyTint(b, 8 * Ramp(t, 6, 12) / 256, RGB(10, 20, 8));
        s->invisible = t >= 26;
        if (t == 26)
            Kill(&b->helper[0]);
        if (t == 2)
            PlaySE(SE_M_STRING_SHOT);
        break;
    }
    case MPFX_RECALL_WISP:
        // blue flames circle it; it fades into a single wisp of blue fire that drifts away to the left
        if (t < 14 && t % 2 == 0)
            for (i = 0; i < Budget(4); i++)
            {
                u8 a = (u8)(i * 64 + t * 14);
                if ((p = Spawn(FXD_BW_WISPFIRE0, 0, mx + Cos(a, Dist(22)), my + Sin(a, Dist(8)))))
                {
                    p->nFrames = 4;
                    p->frameTime = 2;
                    p->alpha = 13;
                    p->life = 2;
                    p->flags = Sin(a, 1) < 0 ? PF_UNDER : 0;
                    Size(p, 12);
                }
            }
        if (t == 14 && (p = Spawn(FXD_BW_BLUEFIRE0, 0, mx, my)))
        {
            p->nFrames = 3;
            p->frameTime = 2;
            p->move = MOVE_HOME;
            p->tx = b->bx;
            p->ty = b->by - Dist(10);
            p->sway = 2;
            p->life = 14;
            p->fadeOut = 6;
            Size(p, 16);
        }
        b->alpha = 16 - 15 * Ramp(t, 6, 14) / 256;
        BodyTint(b, 12 * Ramp(t, 2, 12) / 256, RGB(14, 20, 31));
        s->invisible = t >= 14;
        if (t == 2)
            PlaySE(SE_M_CONFUSE_RAY);
        break;
    case MPFX_RECALL_SNOWGLOBE:
    {
        // a snow globe closes over it, snow swirling inside; the globe shrinks and floats back to the ball
        struct FxPart *globe = PartAt(b->helper[0]);
        if (t == 0 && (globe = Helper(b, 0, FXA_MORE_GLOBE, 0, mx, my + Dist(4))))
            globe->fadeIn = 4;
        k = Ramp(t, 14, 26);
        if (globe)
        {
            Size(globe, (BodyHeight(b) + 6) * (256 - 200 * k / 256) / 256);
            globe->x = (mx + (b->bx - mx) * Ease(k) / 256) << 8;
            globe->y = (my + Dist(4) + (b->by - my) * k / 256) << 8;
        }
        if (t < 14 && (p = Spawn(FXD_BW_SNOWFLAKE, 0, mx + Rnd(Dist(16)), my - Dist(10))))
        {
            p->vy = 0xA0;
            p->dRot = 8;
            p->life = 8;
            p->fadeOut = 3;
            Size(p, 5);
        }
        r = 256 - 200 * k / 256;
        BodyScale(b, r, r, FALSE);
        s->x2 = b->baseX2 + (b->bx - mx) * Ease(k) / 256;
        s->y2 = b->baseY2 + (b->by - my) * k / 256;
        BodyTint(b, 6, RGB(26, 30, 31));
        s->invisible = t >= 26;
        if (t == 26)
            Kill(&b->helper[0]);
        if (t == 2)
            PlaySE(SE_M_ICY_WIND);
        break;
    }
    case MPFX_RECALL_BLACKOUT:
        // the light flickers - dark, light, dark - and when it comes back for good, it's gone
        b->alpha = 16;
        BodyTint(b, (t % 6) < 3 && t < 18 ? 16 : 0, RGB(1, 1, 2));
        if (t < 15 && (t % 6) == 3 && (p = Spawn(FXD_BW_SPARKLE, 0, mx + Rnd(Dist(10)), head)))
        {
            // a last flicker of light as it comes back on
            p->tint = RGB(31, 30, 22);
            p->blend = BLEND_GLOW;
            p->life = 3;
            Size(p, 12);
        }
        s->invisible = t >= 15;
        if (t == 0 || t == 6 || t == 12)
            PlaySE(SE_PC_OFF);
        break;
    }
}

// ---------------------------------------------------------------------------
// Particle sets

static void MoreParticles(u8 part, u8 color, s16 mx, s16 my, s16 feet, s16 head, struct FxBody *b)
{
    struct FxPart *p;
    u8 i;
    (void)b;
    switch (part)
    {
    case MPFX_PART_BALLS:
        // little balls of every kind pop out and bounce round its feet (Black's own POKé BALLS)
        for (i = 0; i < Budget(10); i++)
            if ((p = Spawn(FXD_BW_BALL0 + (i * 7) % 20, 0, mx, my)))
            {
                p->move = MOVE_BOUNCE;
                p->vx = Cos(i * 25 + 4, 0x200);
                p->vy = -0x300 - RndU(0x100);
                p->ay = 0x50;
                p->ty = feet - 4 + Rnd(Dist(4));
                p->dRot = (i & 1) ? 10 : -10;
                p->delay = i;
                p->life = 30;
                p->fadeOut = 8;
                Size(p, 9);
            }
        break;
    case MPFX_PART_MASTER:
        // a MASTER BALL's opening: the ball opens over it in a burst of purple and pink sparkles
        if ((p = Spawn(FXD_BW_MASTER0, 0, mx, head - Dist(14))))
        {
            p->nFrames = 7;
            p->frameTime = 2;
            p->flags = PF_ONCE;
            p->life = 18;
            p->fadeOut = 6;
            Size(p, 18);
        }
        for (i = 0; i < Budget(14); i++)
            if ((p = Spawn(i & 1 ? FXD_PL_SPARKLE : FXD_BW_TWINKLE, 0, mx, head - Dist(14))))
            {
                p->tint = (i % 3) ? RGB(26, 12, 31) : RGB(31, 16, 26);
                p->blend = BLEND_GLOW;
                p->vx = Cos(i * 18, 0x260);
                p->vy = Sin(i * 18, 0x260);
                p->drag = 14;
                p->delay = 8;
                p->life = 18;
                p->fadeOut = 10;
                p->dRot = 6;
                Size(p, 10);
            }
        break;
    case MPFX_PART_RIPPLES:
        // rings of water ripple out from its feet, one after another, and a few drops leap up
        for (i = 0; i < 4; i++)
            if ((p = Spawn(FXD_BW_OVALS, 0, mx, feet - 1)))
            {
                p->tint = RGB(14, 24, 31);
                p->alpha = 11;
                p->stretchY = 80;
                p->flags = PF_UNDER;
                p->delay = i * 5;
                p->life = 16;
                p->fadeOut = 12;
                Size(p, 14);
                p->dScale = p->scale / 6;
            }
        for (i = 0; i < Budget(6); i++)
            if ((p = Spawn(FXD_PL_DROP, 0, mx + Rnd(Dist(20)), feet - 2)))
            {
                p->vy = -0x200 - RndU(0x100);
                p->ay = 0x50;
                p->delay = RndU(12);
                p->life = 12;
                p->fadeOut = 4;
                Size(p, 5);
            }
        break;
    case MPFX_PART_BERRIES:
        // BERRIES tumble out and bounce about its feet
        for (i = 0; i < Budget(8); i++)
            if ((p = Spawn(FXA_MORE_BERRYORAN + i % 5, 0, mx, my)))
            {
                p->move = MOVE_BOUNCE;
                p->vx = Cos(i * 32 + 10, 0x1C0);
                p->vy = -0x2C0 - RndU(0x100);
                p->ay = 0x50;
                p->ty = feet - 5;
                p->dRot = (i & 1) ? 8 : -8;
                p->delay = i;
                p->life = 30;
                p->fadeOut = 8;
                Size(p, 10);
            }
        break;
    case MPFX_PART_STREAMERS:
        // paper streamers shoot out in every color and curl down, as when a ship sets sail
        for (i = 0; i < Budget(12); i++)
            if ((p = Spawn(FXA_MORE_STREAMER0 + i % 6, 0, mx, my - Dist(6))))
            {
                p->move = MOVE_FLUTTER;
                p->vx = Cos(i * 21 + 140, 0x300);
                p->vy = -0x300 - RndU(0x100);
                p->ay = 0x30;
                p->sway = 2;
                p->angle = i * 40;
                p->dRot = (i & 1) ? 10 : -10;
                p->life = 32;
                p->fadeOut = 10;
                p->flags = PF_NOCLIP;
                Size(p, 14);
            }
        break;
    case MPFX_PART_SHOCKORBS:
        // crackling electric orbs circle it, zapping now and then
        for (i = 0; i < Budget(4); i++)
            if ((p = Spawn(FXD_BW_ORB, 0, mx, my)))
            {
                p->tint = RGB(31, 31, 10);
                p->blend = BLEND_GLOW;
                p->move = MOVE_ORBIT;
                p->tx = mx;
                p->ty = my + Dist(4);
                p->angle = (i * 64) << 8;
                p->radius = Dist(26) << 8;
                p->dAngle = 0x900;
                p->flags = PF_FAR_DIM;
                p->life = 30;
                p->fadeIn = 4;
                p->fadeOut = 8;
                Size(p, 8);
            }
        for (i = 0; i < Budget(6); i++)
            if ((p = Spawn(FXD_BW_BOLT0 + i % 4, 0, mx + Rnd(Dist(24)), my + Rnd(Dist(16)))))
            {
                p->tint = RGB(31, 31, 16);
                p->blend = BLEND_GLOW;
                p->delay = 4 + i * 4;
                p->life = 3;
                Size(p, 14);
            }
        break;
    case MPFX_PART_APRICORNS:
        // APRICORNS of every color tumble out around it
        for (i = 0; i < Budget(7); i++)
            if ((p = Spawn(FXA_MORE_APRIRED + i, 0, mx, my - Dist(8))))
            {
                p->move = MOVE_BOUNCE;
                p->vx = Cos(i * 36 + 20, 0x200);
                p->vy = -0x300 - RndU(0x80);
                p->ay = 0x50;
                p->ty = feet - 5;
                p->dRot = (i & 1) ? 7 : -7;
                p->delay = i * 2;
                p->life = 30;
                p->fadeOut = 8;
                Size(p, 11);
            }
        break;
    case MPFX_PART_HEAL:
        // green crosses and soft sparkles rise round it, the way a POKéMON CENTER's heal looks
        for (i = 0; i < Budget(10); i++)
            if ((p = Spawn(i & 1 ? FXA_MORE_CROSS : FXD_BW_TWINKLE, 0, mx + Rnd(Dist(24)), feet - RndU(Dist(10)))))
            {
                if (!(i & 1))
                {
                    p->tint = RGB(20, 31, 20);
                    p->blend = BLEND_GLOW;
                }
                p->move = MOVE_RISE;
                p->vy = -0xC0;
                p->sway = 1;
                p->angle = i * 30;
                p->delay = i * 2;
                p->life = 22;
                p->fadeIn = 3;
                p->fadeOut = 8;
                Size(p, i & 1 ? 7 : 9);
            }
        break;
    case MPFX_PART_SOUND:
        // rings of sound pulse out from it, notes riding the first ones
        for (i = 0; i < 4; i++)
            if ((p = Spawn(FXD_PL_RINGS, 0, mx, my)))
            {
                p->tint = MixRgb(sEnergyRgb[color], RGB_WHITE, 8);
                p->blend = BLEND_GLOW;
                p->alpha = 10;
                p->delay = i * 4;
                p->life = 14;
                p->fadeOut = 10;
                Size(p, 16);
                p->dScale = p->scale / 5;
            }
        for (i = 0; i < Budget(4); i++)
            if ((p = Spawn(FXD_BW_NOTE0 + i % 3, 0, mx, my)))
            {
                p->vx = Cos(i * 64 + 32, 0x180);
                p->vy = Sin(i * 64 + 32, 0x180);
                p->delay = 2;
                p->life = 16;
                p->fadeOut = 8;
                p->sway = 1;
                Size(p, 10);
            }
        break;
    case MPFX_PART_SCALES:
        // glinting scales swirl up around it like a dragon's
        for (i = 0; i < Budget(12); i++)
            if ((p = Spawn(i & 1 ? FXD_BW_CRYSTAL : FXA_MORE_SCALE, color, mx, feet - Dist(4))))
            {
                p->tint = i & 1 ? TINT_ENERGY : 0;
                p->move = MOVE_SPIRAL;
                p->tx = mx;
                p->ty = head;
                p->angle = (i * 21) << 8;
                p->radius = Dist(26) << 8;
                p->dRadius = -(Dist(18) << 8) / 24;
                p->dAngle = 0x900;
                p->delay = i;
                p->life = 24;
                p->fadeOut = 8;
                p->dRot = 9;
                Size(p, 8);
            }
        break;
    case MPFX_PART_POLLEN:
        // glowing pollen drifts up round it in slow spirals
        for (i = 0; i < Budget(16); i++)
            if ((p = Spawn(FXD_BW_SOFTBALL, 0, mx + Rnd(Dist(26)), feet - RndU(Dist(14)))))
            {
                p->tint = RGB(31, 28, 10);
                p->blend = BLEND_GLOW;
                p->move = MOVE_RISE;
                p->vy = -0x60 - RndU(0x40);
                p->sway = 2;
                p->angle = i * 23;
                p->delay = RndU(14);
                p->life = 30;
                p->fadeIn = 6;
                p->fadeOut = 12;
                Size(p, 3 + (i & 1));
            }
        break;
    case MPFX_PART_CALL:
        // signal arcs pulse out on both sides of it, like a call coming in
        for (i = 0; i < 6; i++)
            if ((p = Spawn(FXA_MORE_SIGNAL, color, mx + ((i & 1) ? Dist(16) : -Dist(16)), head)))
            {
                p->flags = (i & 1) ? 0 : PF_FLIPX;
                p->delay = (i / 2) * 6;
                p->life = 8;
                p->fadeIn = 2;
                p->fadeOut = 4;
                Size(p, 12);
                p->dScale = p->scale / 10;
            }
        break;
    case MPFX_PART_DIAMOND:
        // tiny glints of ice sparkle in the cold air all round it
        for (i = 0; i < Budget(16); i++)
            if ((p = Spawn(i & 1 ? FXD_BW_SPARKLE : FXD_PL_TWINKLE, 0, mx + Rnd(Dist(36)), my + Rnd(Dist(30)))))
            {
                p->tint = RGB(24, 30, 31);
                p->blend = BLEND_GLOW;
                p->vy = 0x30;
                p->delay = RndU(20);
                p->life = 10;
                p->fadeIn = 3;
                p->fadeOut = 5;
                p->dRot = 6;
                Size(p, 5 + (i % 3) * 2);
            }
        break;
    case MPFX_PART_SPHERES:
        // glittering spheres pop up out of the ground round its feet, each in its own color
        for (i = 0; i < Budget(8); i++)
            if ((p = Spawn(FXA_MORE_SPHERER + i % 5, 0, mx + Rnd(Dist(26)), feet - 2)))
            {
                p->move = MOVE_BOUNCE;
                p->vy = -0x260 - RndU(0x100);
                p->vx = Rnd(0x80);
                p->ay = 0x50;
                p->ty = feet - 4;
                p->delay = i * 2;
                p->life = 26;
                p->fadeOut = 8;
                p->trailEvery = 3;
                p->trailArt = FXA_ENERGY_MOTE;
                Size(p, 7);
            }
        break;
    case MPFX_PART_PLATES:
        // sixteen colored plates spin in a ring round it, then fly off
        for (i = 0; i < Budget(16); i++)
            if ((p = Spawn(FXA_MORE_PLATE0 + i, color, mx, my)))
            {
                p->move = MOVE_ORBIT;
                p->tx = mx;
                p->ty = my;
                p->angle = (i * 16) << 8;
                p->radius = Dist(30) << 8;
                p->dRadius = 0x40;
                p->dAngle = 0x700;
                p->flags = PF_FAR_DIM;
                p->dRot = 8;
                p->life = 26;
                p->fadeIn = 4;
                p->fadeOut = 8;
                Size(p, 7);
            }
        break;
    case MPFX_PART_CAMERA:
        // camera flashes pop from all around, as at JUBILIFE TV's studio
        for (i = 0; i < Budget(8); i++)
            if ((p = Spawn(FXD_BW_STARBURST, 0, mx + Rnd(Dist(50)), my + Rnd(Dist(34)))))
            {
                p->tint = RGB(31, 31, 28);
                p->blend = BLEND_GLOW;
                p->delay = i * 3;
                p->life = 5;
                p->fadeOut = 4;
                Size(p, 16);
                if (i == 0)
                    PlaySE(SE_M_DETECT);
            }
        break;
    case MPFX_PART_DANDELION:
        // fluffy seeds float off on the breeze, drifting to the left
        for (i = 0; i < Budget(12); i++)
            if ((p = Spawn(FXA_MORE_SEEDFLUFF, 0, mx + Rnd(Dist(20)), my + Rnd(Dist(16)))))
            {
                p->move = MOVE_RISE;
                p->vy = -0x50;
                p->vx = -0x60;
                p->sway = 2;
                p->angle = i * 23;
                p->delay = RndU(10);
                p->life = 34;
                p->fadeOut = 12;
                Size(p, 7);
            }
        break;
    case MPFX_PART_LANTERNS:
        // paper lanterns float up round it, glowing warm
        for (i = 0; i < Budget(6); i++)
            if ((p = Spawn(FXA_MORE_LANTERN, 0, mx + Rnd(Dist(30)), feet - Dist(6))))
            {
                p->move = MOVE_RISE;
                p->vy = -0xA0 - RndU(0x40);
                p->sway = 1;
                p->angle = i * 40;
                p->delay = i * 3;
                p->life = 34;
                p->fadeIn = 4;
                p->fadeOut = 10;
                Size(p, 12);
                if ((p = Glow(MP_BALL_ORANGE, p->x >> 8, p->y >> 8, FALSE, 30, 80, 0)))
                {
                    p->move = MOVE_RISE;
                    p->vy = -0xA0;
                    p->delay = i * 3;
                    p->alpha = 6;
                }
            }
        break;
    case MPFX_PART_SPARKS:
        // hit sparks flare all round it, as in a clash
        for (i = 0; i < Budget(6); i++)
            if ((p = Spawn(i & 1 ? FXA_MORE_HIT0 : FXD_BW_STARBURST, 0, mx + Rnd(Dist(26)), my + Rnd(Dist(20)))))
            {
                if (i & 1)
                {
                    p->nFrames = 5;
                    p->frameTime = 2;
                    p->flags = PF_ONCE;
                }
                else
                {
                    p->tint = RGB(31, 26, 12);
                    p->blend = BLEND_GLOW;
                }
                p->delay = i * 3;
                p->life = 10;
                p->fadeOut = 4;
                Size(p, 18);
            }
        break;
    case MPFX_PART_SEALS:
        // contest seals pop out all round it: hearts, stars, notes, bolts and smoke
        {
            static const u16 sSeal[6] = {FXD_PL_SEALHEART, FXD_PL_SEALPINK, FXD_PL_SEALSTAR, FXD_PL_SEALNOTE,
                                         FXD_PL_SEALNOTE2, FXD_PL_ZIGZAG};
            for (i = 0; i < Budget(12); i++)
                if ((p = Spawn(sSeal[i % 6], 0, mx, my)))
                {
                    p->vx = Cos(i * 21, 0x200);
                    p->vy = Sin(i * 21, 0x200);
                    p->drag = 18;
                    p->delay = i / 2;
                    p->life = 22;
                    p->fadeOut = 8;
                    p->fadeIn = 3;
                    Size(p, 12);
                }
        }
        break;
    }
}
