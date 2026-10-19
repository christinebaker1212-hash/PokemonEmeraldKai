// Moemon Emerald Kai Co-op: online lobby server.
//
// Hosts create a named room (with an optional password), friends pick it from
// the list, and all game traffic is relayed here.
// Run it on a PC at home: it opens its port on the router automatically
// (UPnP / NAT-PMP), prints an invite code for players, and closes the port
// again when it stops. Players can also put its address in lobby.txt.
// See tools/mp_server/DEPLOY.md.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "../../src/mp/mp_net.h"
#ifdef _WIN32
#include <windows.h>
#endif

static volatile int sRunning = 1;
static void OnSignal(int sig) { (void)sig; sRunning = 0; }
#ifdef _WIN32
// Closing the console window: stop cleanly so the router port gets closed
static BOOL WINAPI OnConsole(DWORD type)
{
    (void)type;
    sRunning = 0;
    Sleep(4000);
    return TRUE;
}
#endif

int main(int argc, char **argv)
{
    int port = MP_DEFAULT_PORT;
    int lan = 0, router = 1, shownCode = 0;
    int i, lastPlayers = -1, lastRooms = -1;

    for (i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc)
            port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--lan") == 0)
            lan = 1;
        else if (strcmp(argv[i], "--no-router") == 0)
            router = 0;
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printf("usage: %s [--port N] [--lan] [--no-router]\n"
                   "  --port N      TCP port to listen on (default %d)\n"
                   "  --lan         also answer local network searches (UDP %d)\n"
                   "  --no-router   don't open the port on the router (cloud servers, manual forwarding)\n",
                   argv[0], MP_DEFAULT_PORT, MP_DISCOVERY_PORT);
            return 0;
        }
    }
    signal(SIGINT, OnSignal);
    signal(SIGTERM, OnSignal);
#ifdef _WIN32
    SetConsoleCtrlHandler(OnConsole, TRUE);
#endif
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
    if (router)
    {
        printf("Opening the port on your router...\n");
        MPPortMap_Open(port);
    }
    while (sRunning)
    {
        MPServer_Wait(50);
        MPServer_Poll();
        if (router && !shownCode && MPPortMap_State() == MP_PM_OPEN)
        {
            printf("Router port opened. Invite code for players: %s  (address %s:%d)\n",
                   MPPortMap_InviteCode(), MPPortMap_PublicAddress(), MPPortMap_ExternalPort());
            shownCode = 1;
        }
        else if (router && !shownCode && MPPortMap_State() == MP_PM_FAILED)
        {
            static const char *const why[] = {
                "the router didn't answer (turn on UPnP in its settings)",
                "the router refused",
                "your internet provider shares one address between homes (CGNAT)",
            };
            printf("Couldn't open the port automatically: %s.\n", why[MPPortMap_Error() % 3]);
            shownCode = 1;
        }
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
