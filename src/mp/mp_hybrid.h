// Moemon Co-op: action sprites for looks that have none of their own (see mp_hybrid.c)
#ifndef GUARD_MP_HYBRID_H
#define GUARD_MP_HYBRID_H

#define MP_HY_MAX_FRAMES 27

struct MpHybridLook
{
    const u8 *frame[3];   // the look's standing frames facing S, N and W (4bpp tiles)
    u8 w, h;              // their size in pixels
    u8 neck[3];           // first body row of each (the head is above it)
    const u16 *pal;       // the look's palette: the result uses it as it is
    u8 skin[4];           // palette indices of the skin shades, lightest first
    u8 nSkin;
    bool8 kaiArms;        // Kai's 16-wide sprites hold their arms beside the head: not part of it
};

// Brendan's (female: May's) frames for an avatar state (mp_avatar.c ST_*) with the look's head and
// the look's clothes. bodyArt: the state's first frame (frames follow each other), bodyPal: its
// palette as drawn. Writes the state's frames (frameBytes each) to out and returns how many, 0 when
// the state has none.
u8 MP_Hybrid_Compose(const struct MpHybridLook *look, bool8 female, u8 state, const u8 *bodyArt,
                     const u16 *bodyPal, u8 *out, u8 maxFrames, u32 *frameBytes);
// Which way frame n of a state faces (0 S, 1 N, 2 W, 3 E), 0xFF when there is no such frame
u8 MP_Hybrid_FrameDir(bool8 female, u8 state, u8 frame);
u8 MP_Hybrid_FrameCount(bool8 female, u8 state);

#endif // GUARD_MP_HYBRID_H
