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
                if (!(p = Spawn(FXD_BW_WATERFALL, 0, mx + Rnd(Dist(t < 14 ? 6 : 3)), sFieldMode ? head - 40 : -8)))
                    break;
                p->stretchX = t < 14 ? 256 : 128;   // the stream thins away
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
                    p->blend = BLEND_ADD;
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
                    p->blend = BLEND_ADD;
                    Size(p, 14);
                }
            }
            if (t % 3 == 0 && (p = Spawn(FXD_PL_BOLT0 + RndU(4), 0, mx + Rnd(Dist(10)), my + Rnd(Dist(14)))))
            {
                p->tint = RGB(31, 30, 18);
                p->life = 3;
                p->blend = BLEND_ADD;
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
            Size(doll, 52 * pop / 256);
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
                p->blend = BLEND_ADD;
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
            Fling(FXD_BW_DIRTSPRAY, 1, 0, mx, feet - 4, 3, 0x200, 12, 0x30, 6, 18);
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
                    p->blend = BLEND_ADD;
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
            p->blend = BLEND_ADD;
            Size(p, 56);
        }
        if (t == 0)
            for (i = 0; i < 4; i++)
                if ((p = SettleAt(FXA_MORE_PANE, b->color, mx + (sPane[i] < 0 ? -Dist(100) : Dist(100)), my,
                                  mx + Dist(sPane[i]), my + (i >= 2 ? Dist(4) : 0), 8 + i, 20)))
                {
                    p->blend = BLEND_ADD;
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
            orb->blend = BLEND_ADD;
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
                p->blend = BLEND_ADD;
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
            p->blend = BLEND_ADD;
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
            p->blend = BLEND_ADD;
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
    }
}

// ---------------------------------------------------------------------------
// Returns

static void MoreRecallFrame(struct FxBody *b)
{
    (void)b;
}

// ---------------------------------------------------------------------------
// Particle sets

static void MoreParticles(u8 part, u8 color, s16 mx, s16 my, s16 feet, s16 head, struct FxBody *b)
{
    (void)part; (void)color; (void)mx; (void)my; (void)feet; (void)head; (void)b;
}
