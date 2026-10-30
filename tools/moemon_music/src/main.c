// Moemon Music Studio: plays MIDI files with Moemon Emerald Kai's own instruments and sound
// engine, and exports them as songs for the game.
#include <string.h>
#include <SDL.h>

int cli_main(int argc, char **argv);
int ui_main(int argc, char **argv);

int main(int argc, char **argv)
{
    if (argc >= 2 && !strncmp(argv[1], "--", 2))
        return cli_main(argc, argv);
    return ui_main(argc, argv);
}
