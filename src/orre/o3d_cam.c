// Moemon Kai: Colosseum's camera shots in ORRE's engine (docs/ORRE_HANDOFF.md "Colosseum's cameras"). A shot (.cam,
// in orre/orre_data.pak as the disc's bytes) is an HSD scene whose camera set is a perspective CObjDesc and its
// CameraAnim (the Colosseum decomp's include/hsd/hsd_cobj.h, hsd_wobj.h, hsd_fobj.h): the eye and the point it looks
// at, keyframed (tracks 5 6 7: x y z), the roll (track 9) and the field of view (track 10). While one plays, ORRE's view
// (o3d.c SetupView) looks through it instead of following the player: Colosseum's scenes (cameraPlayAnime) and its
// opening. tools/orre/hsd_cam.py reads the same bytes for looking at a shot.
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "o3d.h"
#include "orre_data.h"

void MPNet_Log(const char *fmt, ...);

enum { TR_EYE_X, TR_EYE_Y, TR_EYE_Z, TR_TGT_X, TR_TGT_Y, TR_TGT_Z, TR_ROLL, TR_FOV, TR_COUNT };
enum { OP_CON = 1, OP_LIN, OP_SPL0, OP_SPL, OP_SLP, OP_KEY };

struct Key { float t, v, in, out; u8 op; };
struct Track { struct Key *k; u32 n; };

static struct
{
    bool8 on, loop, keep;   // keep: the shot goes on into the next room (the opening's rooms change under it)
    u8 *d;
    u32 size;
    float frame, end;
    float base[TR_COUNT];
    struct Track tr[TR_COUNT];
} sCam;

static u32 U32(u32 o) { const u8 *p = sCam.d + 0x20 + o; return o + 4 <= sCam.size - 0x20 ? (u32)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3] : 0; }
static float F32(u32 o) { u32 v = U32(o); float f; memcpy(&f, &v, 4); return f; }
static u8 U8(u32 o) { return o < sCam.size - 0x20 ? sCam.d[0x20 + o] : 0; }

// A keyframe value (Colosseum's are little-endian, unlike the rest of the file): FMT >> 5 its type, & 0x1F its fraction
static float Value(u32 *p, u8 fmt)
{
    const u8 *b = sCam.d + 0x20 + *p;
    float sc = (float)(1 << (fmt & 0x1F));
    if (*p + 4 > sCam.size - 0x20)
        return 0.0f;
    switch (fmt >> 5)
    {
    case 0: { float f; u32 v = b[0] | b[1] << 8 | b[2] << 16 | (u32)b[3] << 24; memcpy(&f, &v, 4); *p += 4; return f; }
    case 1: *p += 2; return (s16)(b[0] | b[1] << 8) / sc;
    case 2: *p += 2; return (u16)(b[0] | b[1] << 8) / sc;
    case 3: *p += 1; return (s8)b[0] / sc;
    default: *p += 1; return b[0] / sc;
    }
}

// An FObjDesc's keys (hsd_fobj.c; tools/orre/hsd_anim.py decode_fobj)
static void Decode(u32 fo, struct Track *out)
{
    u32 length = U32(fo + 4), p = U32(fo + 0x10), end = p + length, cap = 16;
    float t = F32(fo + 8);
    u8 fv = U8(fo + 0xD), fs = U8(fo + 0xE);
    out->k = malloc(sizeof(struct Key) * cap);
    out->n = 0;
    while (p < end && p < sCam.size - 0x20)
    {
        u8 b = U8(p++), op = b & 0xF;
        u32 n = (b >> 4) & 7, sh = 3, i;
        while (b & 0x80)
        {
            b = U8(p++);
            n += (u32)(b & 0x7F) << sh, sh += 7;
        }
        for (i = 0; i <= n; i++)
        {
            float v = 0, tan = 0;
            u32 wait = 0, s2 = 0;
            if (op == OP_CON || op == OP_LIN || op == OP_SPL0 || op == OP_SPL || op == OP_KEY)
                v = Value(&p, fv);
            if (op == OP_SPL || op == OP_SLP)
                tan = Value(&p, fs);
            if (op == OP_CON || op == OP_LIN || op == OP_SPL0 || op == OP_SPL)
                do
                {
                    b = U8(p++);
                    wait |= (u32)(b & 0x7F) << s2, s2 += 7;
                } while (b & 0x80);
            if (op == OP_SLP)
            {
                if (out->n)
                    out->k[out->n - 1].out = tan;   // (the last key's outgoing slope)
                continue;
            }
            if (out->n == cap)
                out->k = realloc(out->k, sizeof(struct Key) * (cap *= 2));
            out->k[out->n].t = t, out->k[out->n].v = v, out->k[out->n].op = op;
            out->k[out->n].in = out->k[out->n].out = tan;
            out->n++;
            t += wait;
        }
    }
}

static bool8 Sample(const struct Track *tr, float f, float *out)
{
    u32 i;
    if (tr->n == 0)
        return FALSE;
    if (f <= tr->k[0].t || tr->n == 1)
        return *out = tr->k[0].v, TRUE;
    for (i = 0; i + 1 < tr->n; i++)
    {
        const struct Key *a = &tr->k[i], *b = &tr->k[i + 1];
        if (f >= a->t && f < b->t)
        {
            float d = b->t - a->t, u = (f - a->t) / d, m0 = a->out, m1 = b->in;
            if (a->op == OP_CON || d <= 0)
                return *out = a->v, TRUE;
            if (a->op == OP_LIN)
                return *out = a->v + (b->v - a->v) * u, TRUE;
            if (a->op == OP_SPL0)
                m0 = m1 = 0;
            *out = (2 * u * u * u - 3 * u * u + 1) * a->v + (u * u * u - 2 * u * u + u) * d * m0
                 + (-2 * u * u * u + 3 * u * u) * b->v + (u * u * u - u * u) * d * m1;
            return TRUE;
        }
    }
    return *out = tr->k[tr->n - 1].v, TRUE;
}

// An AObjDesc's tracks: TYPE n (Colosseum's track number) goes to slot MAP[n] (-1 none)
static void Tracks(u32 aobj, const s8 *map, u32 mapLen)
{
    u32 fo;
    if (!aobj)
        return;
    if (F32(aobj + 4) > sCam.end)
        sCam.end = F32(aobj + 4);
    for (fo = U32(aobj + 8); fo; fo = U32(fo))
    {
        u8 type = U8(fo + 0xC);
        if (type < mapLen && map[type] >= 0 && sCam.tr[(u8)map[type]].n == 0)
            Decode(fo, &sCam.tr[(u8)map[type]]);
    }
}

static void StopShot(void)
{
    u32 i;
    for (i = 0; i < TR_COUNT; i++)
        free(sCam.tr[i].k);
    free(sCam.d);
    memset(&sCam, 0, sizeof(sCam));
}

// ---- the scene's own camera (GSscene_SetMode(3): Colosseum's camera.c, mode 3): an eye and a rotation the scripts
// set (std040_cameraSetPosition, cameraSetRotation) or move to in a straight line over seconds (cameraMovePositionXYZ,
// cameraMoveRotationXYZ: _cameraPadMoveUpdate's lerps), the rotation as HSD's camera takes it: radians about X (up
// and down), Y (round) and Z (the roll), applied Z, X then Y to a camera looking down -Z. cameraReturn moves it back
// to the field's camera; mode 0 ends it at once
static struct
{
    bool8 on, back;          // back: going back to the field's camera (cameraReturn), then off
    float eye[3], rot[3], fov, fov0, fov1;
    float e0[3], e1[3], et, ed;
    float r0[3], r1[3], rt, rd;
} sFree;

void O3D_CamStop(void)
{
    StopShot();
    memset(&sFree, 0, sizeof(sFree));
}

void O3D_CamStopShot(void)
{
    StopShot();
}

static void FreeBegin(void)
{
    if (sFree.on)
        return;
    memset(&sFree, 0, sizeof(sFree));
    O3D_FieldView(sFree.eye, sFree.rot);
    sFree.fov = 30.0f;   // (Colosseum's camera's own, lbl_8047D720)
    sFree.on = TRUE;
}

void O3D_FreeMode(bool8 on)
{
    if (on)
        FreeBegin();
    else
        memset(&sFree, 0, sizeof(sFree));
}

// WHICH: 0 the eye, 1 the rotation; SECONDS 0: at once
void O3D_FreeSet(u8 which, const float v[3], float seconds)
{
    FreeBegin();
    sFree.back = FALSE;
    if (seconds <= 0.0f)
    {
        memcpy(which ? sFree.rot : sFree.eye, v, sizeof(float) * 3);
        if (which)
            sFree.rd = 0;
        else
            sFree.ed = 0;
        return;
    }
    if (which)
        memcpy(sFree.r0, sFree.rot, sizeof(sFree.r0)), memcpy(sFree.r1, v, sizeof(sFree.r1)), sFree.rt = 0, sFree.rd = seconds;
    else
        memcpy(sFree.e0, sFree.eye, sizeof(sFree.e0)), memcpy(sFree.e1, v, sizeof(sFree.e1)), sFree.et = 0, sFree.ed = seconds;
}

void O3D_FreeFov(float fov)
{
    if (fov > 1.0f && fov < 170.0f)
        sFree.fov = fov;
}

// cameraReturn: back to the field's camera over SECONDS
void O3D_FreeReturn(float seconds)
{
    float eye[3], rot[3];
    if (!sFree.on)
        return;
    if (seconds <= 0.0f)
    {
        memset(&sFree, 0, sizeof(sFree));
        return;
    }
    O3D_FieldView(eye, rot);
    // (the yaw the short way round)
    while (rot[1] - sFree.rot[1] > 3.14159265f)
        rot[1] -= 2 * 3.14159265f;
    while (rot[1] - sFree.rot[1] < -3.14159265f)
        rot[1] += 2 * 3.14159265f;
    O3D_FreeSet(0, eye, seconds);
    O3D_FreeSet(1, rot, seconds);
    sFree.fov0 = sFree.fov, sFree.fov1 = O3D_FieldFov();
    sFree.back = TRUE;
}

// cameraMoveEndCheck: a move still going
bool8 O3D_FreeMoving(void)
{
    return sFree.on && (sFree.ed > 0 || sFree.rd > 0);
}

static void FreeTick(void)
{
    u32 i;
    if (!sFree.on)
        return;
    if (sFree.ed > 0)
    {
        sFree.et += 1.0f / 60.0f;
        for (i = 0; i < 3; i++)
            sFree.eye[i] = sFree.et >= sFree.ed ? sFree.e1[i] : sFree.e0[i] + (sFree.e1[i] - sFree.e0[i]) * sFree.et / sFree.ed;
        if (sFree.back)   // (the field's narrower view, in step)
            sFree.fov = sFree.et >= sFree.ed ? sFree.fov1 : sFree.fov0 + (sFree.fov1 - sFree.fov0) * sFree.et / sFree.ed;
        if (sFree.et >= sFree.ed)
            sFree.ed = 0;
    }
    if (sFree.rd > 0)
    {
        sFree.rt += 1.0f / 60.0f;
        for (i = 0; i < 3; i++)
            sFree.rot[i] = sFree.rt >= sFree.rd ? sFree.r1[i] : sFree.r0[i] + (sFree.r1[i] - sFree.r0[i]) * sFree.rt / sFree.rd;
        if (sFree.rt >= sFree.rd)
            sFree.rd = 0;
    }
    if (sFree.back && sFree.ed == 0 && sFree.rd == 0)
        memset(&sFree, 0, sizeof(sFree));
}

// Plays the shot NAME of the data pack ("S1_out/cam_s1_out_shot_01.cam") from FRAME; FALSE: no such shot
bool8 O3D_CamPlay(const char *name, float frame, bool8 loop)
{
    static const s8 sEye[] = {-1, -1, -1, -1, -1, TR_EYE_X, TR_EYE_Y, TR_EYE_Z};
    static const s8 sTgt[] = {-1, -1, -1, -1, -1, TR_TGT_X, TR_TGT_Y, TR_TGT_Z};
    static const s8 sCObj[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, TR_ROLL, TR_FOV};
    u32 scene = 0, cset, cobj, anims, ca, eye, tgt, i, nroots, off;
    StopShot();
    if ((sCam.d = OrreData_Read(name, &sCam.size)) == NULL || sCam.size < 0x20)
        return MPNet_Log("o3d cam: no %s", name), StopShot(), FALSE;
    // the root "scene_data" (an HSD archive: data, relocations, roots: offset, name)
    {
        const u8 *h = sCam.d;
        u32 ds = (u32)h[4] << 24 | h[5] << 16 | h[6] << 8 | h[7], rc = (u32)h[8] << 24 | h[9] << 16 | h[10] << 8 | h[11];
        nroots = (u32)h[12] << 24 | h[13] << 16 | h[14] << 8 | h[15];
        off = 0x20 + ds + rc * 4;
        for (i = 0; i < nroots && off + 8 * i + 8 <= sCam.size; i++)
        {
            const u8 *r = sCam.d + off + 8 * i;
            u32 o = (u32)r[0] << 24 | r[1] << 16 | r[2] << 8 | r[3];
            u32 so = (u32)r[4] << 24 | r[5] << 16 | r[6] << 8 | r[7];
            u32 sp = off + 8 * (nroots + ((u32)h[16] << 24 | h[17] << 16 | h[18] << 8 | h[19])) + so;
            if (sp + 10 < sCam.size && memcmp(sCam.d + sp, "scene_data", 10) == 0)
                scene = o;
        }
    }
    cset = scene ? U32(scene + 4) : 0;
    if (!cset || !(cobj = U32(cset)))
        return MPNet_Log("o3d cam: %s has no camera", name), StopShot(), FALSE;
    // (CObjDesc: eye + 0x18, target + 0x1C (WObjDesc: x y z at + 4), roll + 0x20, fov + 0x30)
    eye = U32(cobj + 0x18), tgt = U32(cobj + 0x1C);
    for (i = 0; i < 3; i++)
    {
        sCam.base[TR_EYE_X + i] = eye ? F32(eye + 4 + 4 * i) : 0;
        sCam.base[TR_TGT_X + i] = tgt ? F32(tgt + 4 + 4 * i) : 0;
    }
    sCam.base[TR_ROLL] = F32(cobj + 0x20);
    sCam.base[TR_FOV] = F32(cobj + 0x30);
    if (!(sCam.base[TR_FOV] > 1.0f && sCam.base[TR_FOV] < 170.0f))
        sCam.base[TR_FOV] = 55.0f;
    // (CameraAnim: its own AObjDesc, then the eye's and the target's WObjAnim: an AObjDesc first)
    anims = U32(cset + 4);
    if (anims && (ca = U32(anims)) != 0)
    {
        Tracks(U32(ca), sCObj, sizeof(sCObj));
        if (U32(ca + 4))
            Tracks(U32(U32(ca + 4)), sEye, sizeof(sEye));
        if (U32(ca + 8))
            Tracks(U32(U32(ca + 8)), sTgt, sizeof(sTgt));
    }
    sCam.on = TRUE, sCam.loop = loop, sCam.frame = frame;
    MPNet_Log("o3d cam: %s, %.0f frames", name, sCam.end);
    return TRUE;
}

// By the resource id a script names (cameraPlayAnime: index/ids.txt of the data pack)
bool8 O3D_CamPlayId(u32 id, float frame, bool8 loop)
{
    char name[128];
    if (!OrreData_ResourceName(id, name, sizeof(name)))
        return MPNet_Log("o3d cam: no shot %08X", id), FALSE;
    return O3D_CamPlay(name, frame, loop);
}

bool8 O3D_CamKeep(void)
{
    return sCam.keep;
}

void O3D_CamSetKeep(bool8 keep)
{
    sCam.keep = keep;
}

bool8 O3D_CamActive(void)
{
    return sCam.on || sFree.on;
}

// The shot has played to its end (cameraWaitSyncAnime)
bool8 O3D_CamDone(void)
{
    return !sCam.on || (!sCam.loop && sCam.frame >= sCam.end);
}

// Every frame (o3d.c): the shot moves on, Colosseum's 60 frames a second as Kai's
void O3D_CamTick(void)
{
    FreeTick();
    if (!sCam.on)
        return;
    sCam.frame += 1.0f;
    if (sCam.loop && sCam.end > 0 && sCam.frame > sCam.end)
        sCam.frame -= sCam.end;
    if (sCam.frame > sCam.end)
        sCam.frame = sCam.end;
}

// This frame's camera, in Colosseum's units: the eye, the point it looks at, the roll (degrees? radians: as the
// track has it) and the vertical field of view in degrees; FALSE: none playing
bool8 O3D_CamGet(float eye[3], float tgt[3], float *roll, float *fov)
{
    float v[TR_COUNT];
    u32 i;
    if (!sCam.on && sFree.on)
    {
        // (the scene's camera: looking down -Z turned by Z, X, then Y)
        float cx = cosf(sFree.rot[0]), sx = sinf(sFree.rot[0]), cy = cosf(sFree.rot[1]), sy = sinf(sFree.rot[1]);
        for (i = 0; i < 3; i++)
            eye[i] = sFree.eye[i];
        tgt[0] = eye[0] - cx * sy * 10.0f;
        tgt[1] = eye[1] + sx * 10.0f;
        tgt[2] = eye[2] - cx * cy * 10.0f;
        *roll = sFree.rot[2];
        *fov = sFree.fov;
        return TRUE;
    }
    if (!sCam.on)
        return FALSE;
    for (i = 0; i < TR_COUNT; i++)
        if (!Sample(&sCam.tr[i], sCam.frame, &v[i]))
            v[i] = sCam.base[i];
    for (i = 0; i < 3; i++)
        eye[i] = v[TR_EYE_X + i], tgt[i] = v[TR_TGT_X + i];
    *roll = v[TR_ROLL];
    *fov = v[TR_FOV] > 1.0f && v[TR_FOV] < 170.0f ? v[TR_FOV] : 55.0f;
    return TRUE;
}
