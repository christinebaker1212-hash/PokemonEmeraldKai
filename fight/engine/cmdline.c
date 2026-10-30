/*
 * Command line parsing (original module 0x40B070-0x40B9C9).
 */
#include "mugen.h"

#define CMDLINE_DEFAULT_ROUNDS   1000000
#define CMDLINE_DEFAULT_SPEED    999
#define CMDLINE_DEFAULT_AI_LEVEL 4

static const char cmdline_usage[] =
    "Options (case sensitive):\n"
    " -h -?               Help\n"
    " -log <logfile>      Records match data to <logfile>\n"
    " -r <sysfile>        Loads motif <sysfile>. eg. -r motifdir or -r motifdir/system.def\n"
    "\n"
    "Quick VS Options:\n"
    " -p<n> <playername>  Loads player n, eg. -p3 kfm\n"
    " -p<n>.ai 1          Enables AI for player ?, eg. -p1.ai 1\n"
    " -p<n>.color <col>   Set player n's color to <col>\n"
    " -p<n>.life <life>   Sets player n's life to <life>\n"
    " -p<n>.power <power> Sets player n's power to <power>\n"
    " -rounds <num>       Plays for <num> rounds, and then quits\n"
    " -s <stagename>      Loads stage <stagename>\n"
    "\n"
    "Debug Options:\n"
    " -nojoy              Disables joysticks\n"
    " -nomusic            Disables music\n"
    " -nosound            Disables all sound effects and music\n"
    "\n";

/* 0x0040B960: shows the usage in the error dialog and quits. */
void CmdLine_Help(void)
{
    char text[0x4000];

    sprintf(text, "M.U.G.E.N ver %i.%i%i (%i.%02i.%02i)\n(c)%i Elecbyte\n\n",
            MUGEN_VERSION_MAJOR, MUGEN_VERSION_MINOR, MUGEN_VERSION_PATCH,
            MUGEN_BUILD_YEAR, MUGEN_BUILD_MONTH, MUGEN_BUILD_DAY, MUGEN_BUILD_YEAR);
    strcat(text, cmdline_usage);
    Error_ShowDialog(text, "M.U.G.E.N Command line options");
    exit(0);
}

static void CmdLine_Copy(char *dst, const char *src)
{
    dst[0] = '\0';
    strncat(dst, src, 0x3F);
}

/*
 * 0x0040B070. Unknown options show the usage and quit. Quirks kept from the
 * original: "-log" only checks that an argument follows and does not consume
 * it (it then counts as a player name), "-rubric" is an alias of "-r", and
 * unknown "-g..." options as well as -m/-q/-t are silently ignored.
 */
void CmdLine_Parse(int argc, char **argv)
{
    CmdLineArgs *args = g_cmdline;
    int positional = 0;
    int i, p;

    Log_Printf("Command line: %s", argv[0]);
    memset(args, 0, sizeof(CmdLineArgs));
    args->rounds = CMDLINE_DEFAULT_ROUNDS;
    args->speed = CMDLINE_DEFAULT_SPEED;
    for (p = 0; p < 4; p++) {
        args->ai[p] = CMDLINE_UNSET;
        args->color[p] = CMDLINE_UNSET;
        args->life[p] = CMDLINE_UNSET;
        args->power[p] = CMDLINE_UNSET;
    }

    for (i = 1; i < argc; i++) {
        const char *arg = argv[i];
        const char *opt = arg + 1;
        int has_value = i + 1 < argc;

        Log_Printf(" %s", arg);
        if (arg[0] != '-') {
            if (positional == 0) {
                CmdLine_Copy(args->player[0], arg);
            } else if (positional == 1) {
                CmdLine_Copy(args->player[1], arg);
            }
            positional++;
            continue;
        }
        switch (arg[1]) {
        case '?':
        case 'h':
            CmdLine_Help();
        default:
            printf("Error: Invalid option %s\n\n", argv[i]);
            CmdLine_Help();
        case 'a':
            if (strcmp(opt, "ailevel") != 0 || !has_value) {
                CmdLine_Help();
            }
            args->ai_level = atoi(argv[++i]);
            if (args->ai_level < 1 || args->ai_level > 9) {
                args->ai_level = CMDLINE_DEFAULT_AI_LEVEL;
            }
            break;
        case 'g':
            if (strcmp(opt, "gamedir") == 0) {
                if (!has_value) {
                    CmdLine_Help();
                }
                args->game_dir[0] = '\0';
                strncat(args->game_dir, argv[++i], 0x1FF);
            }
            break;
        case 'l':
            if (strcmp(opt, "log") != 0 || !has_value) {
                CmdLine_Help();
            }
            break;
        case 'm':
        case 'q':
        case 't':
            break;
        case 'n':
            if (strcmp(opt, "nomusic") == 0) {
                args->nomusic = 1;
            } else if (strcmp(opt, "nosound") == 0) {
                args->nosound = 1;
            } else if (strcmp(opt, "nojoy") == 0) {
                args->nojoy = 1;
            } else {
                CmdLine_Help();
            }
            break;
        case 'p': {
            int n;
            if (arg[2] < '1' || arg[2] > '4' || !has_value) {
                CmdLine_Help();
            }
            n = arg[2] - '1';
            if (arg[3] == '.') {
                const char *field = arg + 4;
                if (strcmp(field, "ai") == 0) {
                    args->ai[n] = atoi(argv[++i]);
                } else if (strcmp(field, "color") == 0) {
                    args->color[n] = atoi(argv[++i]) - 1;
                } else if (strcmp(field, "life") == 0) {
                    args->life[n] = atoi(argv[++i]);
                } else if (strcmp(field, "power") == 0) {
                    args->power[n] = atoi(argv[++i]);
                } else {
                    CmdLine_Help();
                }
                break;
            }
            if (arg[3] != '\0') {
                CmdLine_Help();
            }
            CmdLine_Copy(args->player[n], argv[++i]);
            break;
        }
        case 'r':
            if (arg[2] != '\0' && strcmp(opt, "rubric") != 0) {
                if (strcmp(opt, "rounds") != 0 || !has_value) {
                    CmdLine_Help();
                }
                args->rounds = atoi(argv[++i]);
                if (args->rounds < 2) {
                    args->rounds = 1;
                }
                break;
            }
            if (!has_value) {
                CmdLine_Help();
            }
            CmdLine_Copy(args->motif, argv[++i]);
            if (strlen(args->motif) > 3 && _stricmp(args->motif + strlen(args->motif) - 4, ".def") == 0) {
                break;
            }
            Str_Append(args->motif, "/system.def", sizeof(args->motif));
            break;
        case 's':
            if (arg[2] == '\0' || strcmp(opt, "stage") == 0) {
                if (!has_value) {
                    CmdLine_Help();
                }
                CmdLine_Copy(args->stage, argv[++i]);
            } else if (strcmp(opt, "speedtest") == 0) {
                args->speedtest = 1;
            } else if (strcmp(opt, "storyboard") == 0) {
                if (!has_value) {
                    CmdLine_Help();
                }
                CmdLine_Copy(args->storyboard, argv[++i]);
            } else if (strcmp(opt, "stresstest") == 0) {
                if (!has_value) {
                    CmdLine_Help();
                }
                args->stresstest = atoi(argv[++i]);
            } else if (strcmp(opt, "speed") == 0) {
                if (!has_value) {
                    CmdLine_Help();
                }
                args->speed = atoi(argv[++i]);
                if (args->speed < -9 || args->speed > 9) {
                    args->speed = CMDLINE_DEFAULT_SPEED;
                }
            } else if (strcmp(opt, "sessionseed") == 0) {
                if (!has_value) {
                    CmdLine_Help();
                }
                args->session_seed = atoi(argv[++i]);
            } else {
                CmdLine_Help();
            }
            break;
        }
    }
    Log_Printf("\n");
}
