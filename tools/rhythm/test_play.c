/* The judging rules (src/rhythm/rh_play.c) on every chart under a Songs folder:
 * - stepping on every arrow exactly: MARVELOUS full combo, AAA, feet x 10,000,000 points;
 * - every step 50 ms late: GREAT (a step that lands nearer the next arrow in its column is GOOD or worse, as on
 *   the arcade: only where arrows in a column are under 100 ms apart);
 * - no steps: the life bar empties and the song fails.
 * usage: test_play SONGS_DIR */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "rhythm/play.h"

static int sCharts, sBad;

static void Run(const struct RhSong *song, int d, double late, int steps, struct RhPlay *p)
{
    struct RhChart chart;
    double t, end = 0;
    int i, next = 0;
    RhChart_Load(song, d, &chart);
    RhPlay_Begin(p, song, &chart, song->charts[d].meter, NULL, 1);
    RhChart_Free(&chart);
    /* press each arrow at its time + late, hold freezes to their end, at 1 ms steps */
    for (i = 0; i < p->count; i++)
        if (p->notes[i].endTime > end)
            end = p->notes[i].endTime;
    for (t = -1; t < end + 1 && !p->failed && !p->finished; t += 0.001)
    {
        while (steps && next < p->count && p->notes[next].time + late <= t)
        {
            if (p->notes[next].type != RH_NOTE_MINE)
                RhPlay_Press(p, p->notes[next].col, p->notes[next].time + late);
            next++;
        }
        for (i = 0; i < RH_COLS; i++)
        {
            int k, keep = 0;
            for (k = p->first; k < p->count && p->notes[k].time <= t; k++)
                if (p->notes[k].col == i && p->notes[k].type == RH_NOTE_HOLD && p->notes[k].endTime + late > t)
                    keep = 1;
            if (!keep && p->held[i])
                RhPlay_Release(p, i, t);
        }
        RhPlay_Update(p, t, 0.001);
    }
}

/* arrows in one column under 100 ms apart */
static int Tight(const struct RhPlay *p)
{
    int i, k;
    for (i = 0; i < p->count; i++)
        for (k = i + 1; k < p->count && p->notes[k].time - p->notes[i].time < 0.1; k++)
            if (p->notes[k].col == p->notes[i].col)
                return 1;
    return 0;
}

static void Check(const char *path, const char *group)
{
    struct RhSong song;
    int d;
    if (!RhSong_Load(&song, path, group))
        return;
    for (d = 0; d < RH_DIFFS; d++)
    {
        struct RhPlay p;
        int meter = song.charts[d].meter < 1 ? 1 : song.charts[d].meter > 10 ? 10 : song.charts[d].meter;
        if (!song.charts[d].present)
            continue;
        sCharts++;
        Run(&song, d, 0, 1, &p);
        if (p.failed || RhPlay_FullCombo(&p) != 3 || RhPlay_Grade(&p) != RH_GRADE_AAAA || p.score != meter * 10000000
            || p.ok != p.holdGroups || p.maxCombo == 0)
        {
            printf("PERFECT RUN %s [%d]: score %d (want %d) grade %d fc %d ok %d/%d failed %d combo %d\n", song.title, d,
                   p.score, meter * 10000000, RhPlay_Grade(&p), RhPlay_FullCombo(&p), p.ok, p.holdGroups, p.failed,
                   p.maxCombo);
            sBad++;
        }
        RhPlay_End(&p);
        Run(&song, d, 0.050, 1, &p);
        if (p.failed || (p.rowCount[RH_J_GREAT] != p.rowsTotal && !Tight(&p)))
        {
            printf("LATE RUN %s [%d]: greats %d/%d fc %d failed %d\n", song.title, d, p.rowCount[RH_J_GREAT], p.rowsTotal,
                   RhPlay_FullCombo(&p), p.failed);
            sBad++;
        }
        RhPlay_End(&p);
        Run(&song, d, 0, 0, &p);
        if (!p.failed)
        {
            printf("NO STEPS %s [%d] didn't fail (life %.2f)\n", song.title, d, p.life);
            sBad++;
        }
        RhPlay_End(&p);
    }
    RhSong_Free(&song);
}

int main(int argc, char **argv)
{
    const char *root = argc > 1 ? argv[1] : "stepmania/Songs";
    DIR *groups = opendir(root);
    struct dirent *g;
    if (!groups)
        return 2;
    while ((g = readdir(groups)) != NULL)
    {
        char gp[1024];
        DIR *songs;
        struct dirent *s;
        if (g->d_name[0] == '.')
            continue;
        snprintf(gp, sizeof(gp), "%s/%s", root, g->d_name);
        if (!(songs = opendir(gp)))
            continue;
        while ((s = readdir(songs)) != NULL)
        {
            char sp[1300], found[1600] = "";
            DIR *files;
            struct dirent *f;
            if (s->d_name[0] == '.')
                continue;
            snprintf(sp, sizeof(sp), "%s/%s", gp, s->d_name);
            if (!(files = opendir(sp)))
                continue;
            while ((f = readdir(files)) != NULL)
            {
                size_t n = strlen(f->d_name);
                if (n > 3 && !strcasecmp(f->d_name + n - 3, ".sm"))
                    snprintf(found, sizeof(found), "%s/%s", sp, f->d_name);
            }
            closedir(files);
            if (found[0])
                Check(found, g->d_name);
        }
        closedir(songs);
    }
    closedir(groups);
    printf("%d charts, %d problems\n", sCharts, sBad);
    return sBad != 0;
}
