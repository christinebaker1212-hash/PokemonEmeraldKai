// Moemon Emerald Kai Co-op: online lobby server.
//
// Run this once on an always-on computer that everyone can reach (for example
// a free cloud VM, see tools/mp_server/DEPLOY.md). Players never need to know
// its address: it is built into the game with  make ... LOBBY=<address>.
// Hosts create a named room (with an optional password), friends pick it from
// the list, and all game traffic is relayed here, so nobody has to open ports.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "../../src/mp/mp_net.h"

static volatile int sRunning = 1;
static void OnSignal(int sig) { (void)sig; sRunning = 0; }

int main(int argc, char **argv)
{
    int port = MP_DEFAULT_PORT;
    int lan = 0;
    int i, lastPlayers = -1, lastRooms = -1;

    for (i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc)
            port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--lan") == 0)
            lan = 1;
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printf("usage: %s [--port N] [--lan]\n"
                   "  --port N  TCP port to listen on (default %d)\n"
                   "  --lan     also answer local network searches (UDP %d)\n",
                   argv[0], MP_DEFAULT_PORT, MP_DISCOVERY_PORT);
            return 0;
        }
    }
    signal(SIGINT, OnSignal);
    signal(SIGTERM, OnSignal);
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif
    setvbuf(stdout, NULL, _IOLBF, 0);
    printf("Moemon Emerald Kai Co-op lobby server (protocol %d)\n", MP_PROTOCOL_VERSION);
    if (MPServer_Start(port, lan) != 0)
    {
        printf("Could not start: port %d is in use or not allowed.\n", port);
        return 1;
    }
    printf("Listening on TCP port %d. Press Ctrl+C to stop.\n", port);
    while (sRunning)
    {
        MPServer_Wait(50);
        MPServer_Poll();
        if (MPServer_PlayerCount() != lastPlayers || MPServer_RoomCount() != lastRooms)
        {
            lastPlayers = MPServer_PlayerCount();
            lastRooms = MPServer_RoomCount();
            printf("Rooms: %d  Players: %d\n", lastRooms, lastPlayers);
        }
    }
    MPServer_Stop();
    printf("Server stopped.\n");
    return 0;
}
