/* Reads every simfile under a StepMania Songs folder with the game's reader (src/rhythm/rh_chart.c) and checks it:
 * each chart loads, its notes are in order, the time of each beat goes up, and the counts match the header's.
 * usage: tools/rhythm/test_charts.sh [SONGS_DIR] */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "rhythm/chart.h"

static int sSongs, sCharts, sBad;

static void Check(const char *path, const char *group)
{
    struct RhSong song;
    int d;
    if (!RhSong_Load(&song, path, group))
    {
        printf("NOT A SONG: %s\n", path);
        sBad++;
        return;
    }
    sSongs++;
    for (d = 0; d < RH_DIFFS; d++)
    {
        struct RhChart chart;
        int i;
        if (!song.charts[d].present)
            continue;
        if (!RhChart_Load(&song, d, &chart))
        {
            printf("CHART %d didn't load: %s\n", d, path);
            sBad++;
            continue;
        }
        sCharts++;
        for (i = 1; i < chart.count; i++)
            if (chart.notes[i].time < chart.notes[i - 1].time - 1e-9)
            {
                printf("OUT OF ORDER %s chart %d at %d\n", path, d, i);
                sBad++;
                break;
            }
        if (chart.rows != song.charts[d].steps)
        {
            printf("ROWS %d != header %d: %s chart %d\n", chart.rows, song.charts[d].steps, path, d);
            sBad++;
        }
        if (getenv("VERBOSE"))
            printf("  %-40.40s %d meter %2d rows %4d holds %3d last %.2fs bpm %.0f-%.0f\n", song.title, d,
                   song.charts[d].meter, chart.rows, chart.holds, chart.lastTime, song.displayBpmLo, song.displayBpmHi);
        RhChart_Free(&chart);
    }
    {
        double b, last = -1e9;
        for (b = -4; b < song.lastBeat + 8; b += 0.25)
        {
            double t = RhSong_TimeAt(&song, b), back = RhSong_BeatAt(&song, t);
            if (t < last - 1e-9)
            {
                printf("TIME GOES BACK %s at beat %.2f\n", path, b);
                sBad++;
                break;
            }
            if (back > b + 1e-6 || back < b - 1e-6)
            {
                /* (a beat in a stop comes back as the stop's beat) */
                int i, inStop = 0;
                for (i = 0; i < song.stopCount; i++)
                    if (b > song.stops[i].beat - 1e-9 && RhSong_TimeAt(&song, b) <= song.stops[i].time + song.stops[i].seconds + 1e-9 && b - song.stops[i].beat < 1e-6)
                        inStop = 1;
                if (!inStop && (back > b + 1e-4 || back < b - 1e-4))
                {
                    printf("BEAT %.4f -> %.4f s -> beat %.4f: %s\n", b, t, back, path);
                    sBad++;
                    break;
                }
            }
            last = t;
        }
    }
    RhSong_Free(&song);
}

int main(int argc, char **argv)
{
    const char *root = argc > 1 ? argv[1] : "stepmania/Songs";
    DIR *groups = opendir(root);
    struct dirent *g;
    if (!groups)
    {
        printf("no %s\n", root);
        return 2;
    }
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
            char sp[1300];
            DIR *files;
            struct dirent *f;
            char found[1600] = "";
            if (s->d_name[0] == '.')
                continue;
            snprintf(sp, sizeof(sp), "%s/%s", gp, s->d_name);
            if (!(files = opendir(sp)))
                continue;
            while ((f = readdir(files)) != NULL)
            {
                size_t n = strlen(f->d_name);
                if ((n > 4 && !strcasecmp(f->d_name + n - 4, ".ssc")) || (n > 3 && !strcasecmp(f->d_name + n - 3, ".sm") && !found[0]))
                    snprintf(found, sizeof(found), "%s/%s", sp, f->d_name);
            }
            closedir(files);
            if (found[0])
                Check(found, g->d_name);
        }
        closedir(songs);
    }
    closedir(groups);
    printf("%d songs, %d charts, %d problems\n", sSongs, sCharts, sBad);
    return sBad != 0;
}
