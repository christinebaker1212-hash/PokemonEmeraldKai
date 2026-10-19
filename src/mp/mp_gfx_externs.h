// Moemon Co-op: overworld graphics infos used by the avatar and pack code
#ifndef GUARD_MP_GFX_EXTERNS_H
#define GUARD_MP_GFX_EXTERNS_H
#define X(name) extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_##name;
X(BrendanNormal) X(BrendanMachBike) X(BrendanAcroBike) X(BrendanSurfing) X(BrendanUnderwater)
X(BrendanFieldMove) X(BrendanFishing) X(BrendanWatering) X(BrendanDecorating)
X(MayNormal) X(MayMachBike) X(MayAcroBike) X(MaySurfing) X(MayUnderwater)
X(MayFieldMove) X(MayFishing) X(MayWatering) X(MayDecorating)
X(RubySapphireBrendan) X(RubySapphireMay)
X(Boy1) X(LittleBoy) X(CyclingTriathleteM)
#undef X
#endif
