#ifndef MUGEN_FTOL_H
#define MUGEN_FTOL_H

/*
 * The original's two float to int conversions.
 *
 * Casts to int go through __ftol2_sse (0x4DC850): with SSE2 (any machine
 * the game runs on today) that is cvttsd2si, which truncates and gives
 * 0x80000000 for NaN and values out of the int range. A plain cast in this
 * build's CRT makes the same choice; the range check keeps the result
 * independent of the compiler.
 */
static __inline int Ftol(double x)
{
    if (!(x > -2147483649.0 && x < 2147483648.0)) {
        return (int)0x80000000u;
    }
    return (int)x;
}

/*
 * The conversion the original inlines in a few places (software scaling,
 * parallax and the sound resampler): a truncating 64-bit fistp of which only
 * the low 32 bits are kept.
 */
static __inline int Trunc64(double x)
{
    return (int)(__int64)x;
}

#endif /* MUGEN_FTOL_H */
