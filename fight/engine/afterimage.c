/*
 * Afterimages (original module 0x401000-0x40168F, the first one of the
 * executable): a ring buffer of the frames an animation showed, with their
 * draw parameters, PalFX and palette remapping, drawn behind the animation
 * with the colour effects of the AfterImage controller growing with the age
 * of the frame.
 *
 * AfterImage.next is the next frame written, oldest the oldest one kept
 * (both indices into the ring of settings.length frames), ticks counts the
 * ticks since the afterimage started and running says it is running.
 */
#include "mugen.h"

/* A recorded frame (0x114 bytes; AfterImage.frames holds AFTERIMAGE_FRAMES of them). */
typedef struct AfterImageFrame {
    int sprite;             /* 0x000 slot of the sprite in the animation's sprite set, -1 = none */
    int layer;              /* 0x004 sort key of the animation (AnimState.layer) */
    DrawParams dp;          /* 0x008 draw parameters, position and scale in game space */
    PalFx palfx;            /* 0x068 rebuilt by AfterImage_Draw */
    PalRemap remap;         /* 0x090 remapping of the owner when recorded */
} AfterImageFrame;
MUGEN_ASSERT_OFFSET(AfterImageFrame, dp, 0x008);
MUGEN_ASSERT_OFFSET(AfterImageFrame, palfx, 0x068);
MUGEN_ASSERT_OFFSET(AfterImageFrame, remap, 0x090);
MUGEN_ASSERT_SIZE(AfterImageFrame, 0x114);

#define AFTERIMAGE_FRAMES 60

/*
 * 0x00401000: clears ai and allocates its 60 frames (DrawParams_Init and
 * PalFx_Init(NULL) inlined for each). Returns 0 when out of memory.
 */
int AfterImage_Init(AfterImage *ai)
{
    AfterImageFrame *frames;
    int i;

    memset(ai, 0, sizeof(*ai));
    frames = calloc(1, AFTERIMAGE_FRAMES * sizeof(AfterImageFrame));
    if (frames == NULL) {
        g_outOfMemory = 1;
    }
    ai->frames = frames;
    if (frames == NULL) {
        return 0;
    }
    ai->max_length = AFTERIMAGE_FRAMES;
    for (i = 0; i < AFTERIMAGE_FRAMES; i++) {
        AfterImageFrame *f = &frames[i];
        PalFx_Init(&f->palfx, NULL);
        memset(&f->remap, 0, sizeof(f->remap));
        DrawParams_Init(&f->dp);
    }
    return 1;
}

/*
 * 0x004010E0: records the current frame of the followed animation every
 * timegap ticks (drawn with dp, the owner's draw parameters, and remap),
 * converted to game space. Stops the afterimage (returns 0) once its time
 * and the frames it shows are over; returns 1 while it runs. active counts
 * the tick.
 */
int AfterImage_Update(const DrawParams *dp, AfterImage *ai, const PalRemap *remap, int active)
{
    AfterImageSettings *s = &ai->settings;

    if (!ai->running || ai->anim == NULL) {
        return 0;
    }
    if (s->time != -1 && ai->ticks + 1 >= s->timegap * s->length + s->time) {
        ai->running = 0;
        return 0;
    }
    if (ai->ticks % s->timegap == 0) {
        AfterImageFrame *f = (AfterImageFrame *)ai->frames + ai->next;
        DrawParams tmp;
        Coord c1, c2;
        const Coord *pc;

        f->sprite = ai->anim->sprite;
        tmp = ai->anim->dp;
        f->dp = *DrawParams_Combine(dp, DrawParams_ScalePos(dp, &tmp));
        pc = Coord_MakeUnit(&c1, (double)f->dp.x, (double)f->dp.y);
        pc = Coord_ScreenToGame(&c2, pc->x, pc->y, pc->z, pc->unit, pc->type, pc->pad_24);
        f->dp.x = (float)pc->x;
        f->dp.y = (float)pc->y;
        pc = Coord_Make(&c1, (double)f->dp.scale_x, (double)f->dp.scale_y);
        pc = Coord_Unzoom(&c2, pc->x, pc->y, pc->z, pc->unit, pc->type, pc->pad_24);
        f->dp.scale_x = (float)pc->x;
        f->dp.scale_y = (float)pc->y;
        f->layer = ai->anim->layer;
        if (remap != NULL) {
            f->remap = *remap;
        }
        if (ai->ticks + 1 >= s->time && s->time != -1) {
            f->sprite = -1;
        }
        ai->next++;
        if (ai->next >= s->length) {
            ai->next = 0;
        }
        if (ai->next == ai->oldest) {
            ai->oldest++;
            if (ai->oldest >= s->length) {
                ai->oldest = 0;
            }
        }
    }
    if (active) {
        ai->ticks++;
    }
    return 1;
}

/* A new PalFX serial for fx, as the original bumps it after every change. */
static void AfterImage_Touch(PalFx *fx)
{
    fx->serial = g_palFxSerial++;
}

/*
 * 0x004012C0: queues the recorded frames, newest first (every framegap-th
 * one), on the animation's sprite queue with sort keys just below
 * min(frame layer, layer). Each frame gets the PalFX of the controller:
 * palcolor, palinvertall, palbright and palcontrast, palpostbright (divided
 * by palcontrast), then paladd times its age and palmul to the power of its
 * age on the colour factors (the newest frame drawn gets neither).
 */
void AfterImage_Draw(AfterImage *ai, int layer)
{
    AfterImageSettings *s = &ai->settings;
    int wrapped = 1;
    int drawn = 0;
    int key_offset;
    int idx;
    float mul_r = 256.0f, mul_g = 256.0f, mul_b = 256.0f;

    if (!ai->running) {
        return;
    }
    idx = ai->next;
    if (idx == ai->oldest) {
        return;
    }
    if (idx < ai->oldest) {
        wrapped = 0;
    }
    key_offset = -1;
    if (idx < 0) {
        idx += s->length;
        wrapped = 1;
    }
    for (;;) {
        AfterImageFrame *f;
        SpriteSet *set;
        const KeyedArray *sprites;
        PalFx *fx;
        SpriteQueueItem item;
        Coord c1;
        const Coord *pc;
        int sprite, key;

        idx -= s->framegap;
        if (idx < 0 && !wrapped) {
            idx += s->length;
            wrapped = 1;
        }
        if (idx < ai->oldest && wrapped) {
            break;
        }
        f = (AfterImageFrame *)ai->frames + idx;
        sprite = f->sprite;
        if (sprite == -1) {
            continue;
        }
        set = ai->anim->sprites;
        sprites = set->sprites;
        /* the address of the sprite slot, as the original tests it (never NULL in practice) */
        if ((uintptr_t)sprites->data + (uintptr_t)(sprites->elem_size * sprite) == 0) {
            continue;
        }
        fx = &f->palfx;
        fx->add[0] = 0;
        fx->add[1] = 0;
        fx->add[2] = 0;
        fx->unused_1c = 0;
        fx->invertall = 0;
        fx->mul[2] = 0x100;
        fx->mul[1] = 0x100;
        fx->mul[0] = 0x100;
        fx->color = 0x100;
        AfterImage_Touch(fx);
        if (s->palcolor != 0x100) {
            AfterImage_Touch(fx);
            fx->color = (int)((unsigned int)s->palcolor << 8) >> 8;
        }
        if (s->palinvertall) {
            AfterImage_Touch(fx);
            fx->invertall = fx->invertall == 0;
        }
        if (s->palbright[0] != 0 || s->palbright[1] != 0 || s->palbright[2] != 0) {
            fx->add[0] += s->palbright[0];
            fx->add[1] += s->palbright[1];
            fx->add[2] += s->palbright[2];
            AfterImage_Touch(fx);
        }
        if (s->palcontrast[0] != 0x100 || s->palcontrast[1] != 0x100 || s->palcontrast[2] != 0x100) {
            AfterImage_Touch(fx);
            fx->mul[0] = (int)((unsigned int)s->palcontrast[0] << 8) >> 8;
            fx->mul[1] = (int)((unsigned int)s->palcontrast[1] << 8) >> 8;
            fx->mul[2] = (int)((unsigned int)s->palcontrast[2] << 8) >> 8;
        }
        if (s->palcontrast[0] != 0 && s->palpostbright[0] / s->palcontrast[0] != 0) {
            fx->add[0] += s->palpostbright[0] / s->palcontrast[0];
            AfterImage_Touch(fx);
        }
        if (s->palcontrast[1] != 0 && s->palpostbright[1] / s->palcontrast[1] != 0) {
            fx->add[1] += s->palpostbright[1] / s->palcontrast[1];
            AfterImage_Touch(fx);
        }
        if (s->palcontrast[2] != 0 && s->palpostbright[2] / s->palcontrast[2] != 0) {
            fx->add[2] += s->palpostbright[2] / s->palcontrast[2];
            AfterImage_Touch(fx);
        }
        if (drawn == 0) {
            mul_r = 256.0f;
            mul_g = 256.0f;
            mul_b = 256.0f;
        } else {
            int add_r = s->paladd[0] * drawn;
            int add_g = s->paladd[1] * drawn;
            int add_b = s->paladd[2] * drawn;
            mul_r = s->palmul[0] * mul_r;
            mul_g = s->palmul[1] * mul_g;
            mul_b = s->palmul[2] * mul_b;
            if (add_r != 0 || add_g != 0 || add_b != 0) {
                fx->add[0] += add_r;
                fx->add[1] += add_g;
                fx->add[2] += add_b;
                AfterImage_Touch(fx);
            }
            {
                int b = Ftol(mul_b);
                int g = Ftol(mul_g);
                int r = Ftol(mul_r);
                PalFx_ScaleMul(fx, r, g, b);
            }
        }
        drawn++;
        memset(&item, 0, sizeof(item));
        key = f->layer;
        if (key > layer) {
            key = layer;
        }
        item.layer = key + key_offset;
        item.sprites = set;
        item.sprite = sprite;
        item.palfx = fx;
        item.remap = &f->remap;
        item.dp = f->dp;
        /* pad_24 is stack garbage in the original; only x and y of the result are used */
        c1.x = (double)f->dp.x;
        c1.y = (double)f->dp.y;
        c1.z = 0.0;
        c1.unit = 1.0;
        c1.type = 2;
        c1.pad_24 = 0;
        {
            Coord c2;
            pc = Coord_GameToScreen(&c2, c1.x, c1.y, c1.z, c1.unit, c1.type, c1.pad_24);
            item.dp.x = (float)pc->x;
            item.dp.y = (float)pc->y;
        }
        item.dp.scale_x = f->dp.scale_x * g_game->camera_zoom;
        item.dp.scale_y = f->dp.scale_y * g_game->camera_zoom;
        DrawParams_SetBlend(s->trans, &item.dp, s->alpha[0], s->alpha[1]);
        SpriteQueue_Add(&item, ai->anim->queue);
        key_offset--;
    }
}
