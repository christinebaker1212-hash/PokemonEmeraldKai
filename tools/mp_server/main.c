// Moemon Emerald Kai Co-op: standalone relay server.
// Run this on any PC (or a cheap VPS) that all players can reach, then set
// "server = <this machine's address>" in everyone's moemon_coop.ini.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifdef _WIN32
#include <windows.h>
static void SleepMs(int ms) { Sleep(ms); }
#else
#include <unistd.h>
static void SleepMs(int ms) { usleep(ms * 1000); }
#endif
#include "../../src/mp/mp_net.h"

static volatile int sRunning = 1;
static void OnSignal(int sig) { (void)sig; sRunning = 0; }

int main(int argc, char **argv)
{
    int port = MP_DEFAULT_PORT;
    int i, lastCount = -1;

    for (i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc)
            port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printf("usage: %s [--port N]   (default port %d)\n", argv[0], MP_DEFAULT_PORT);
            return 0;
        }
    }
    signal(SIGINT, OnSignal);
    signal(SIGTERM, OnSignal);
    printf("Moemon Emerald Kai Co-op server (protocol %d)\n", MP_PROTOCOL_VERSION);
    if (MPServer_Start(port) != 0)
    {
        printf("Could not start the server: %s\n", MPNet_LastError());
        return 1;
    }
    printf("Players connect with  server = <this computer's IP>  and  port = %d\n", port);
    printf("Internet play: forward TCP port %d on your router to this computer.\n", port);
    printf("Press Ctrl+C to stop.\n");
    while (sRunning)
    {
        MPServer_Poll();
        if (MPServer_PlayerCount() != lastCount)
        {
            lastCount = MPServer_PlayerCount();
            printf("Players online: %d/%d\n", lastCount, MP_MAX_PLAYERS);
            fflush(stdout);
        }
        SleepMs(2);
    }
    MPServer_Stop();
    printf("Server stopped.\n");
    return 0;
}
