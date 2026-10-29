#ifndef GUARD_MP_VERSION_H
#define GUARD_MP_VERSION_H

// Moemon Co-op: this build's version. tools/release/package.sh puts it in moemon_release.txt and update.txt,
// and the GitHub release that ships it is tagged v<this>. Bump it for every release (the updater only
// installs newer releases within a channel, or switches channels explicitly).
#define MOEMON_VERSION "1.44.1"

// Main and dev compile the same sources. CI supplies its increasing run number
// as the dev revision, independently of the live game's version.
#ifndef MOEMON_DEV_BUILD
#define MOEMON_DEV_BUILD 0
#endif
#ifndef MOEMON_DEV_REVISION
#define MOEMON_DEV_REVISION 0
#endif
#define MP_VERSION_STRING_(x) #x
#define MP_VERSION_STRING(x) MP_VERSION_STRING_(x)
#if MOEMON_DEV_BUILD
#define MOEMON_BUILD_VERSION MOEMON_VERSION "." MP_VERSION_STRING(MOEMON_DEV_REVISION)
#define MOEMON_BUILD_CHANNEL "dev"
#define MOEMON_BUILD_TITLE "Moemon Emerald Kai Co-op " MOEMON_BUILD_VERSION " DEV - SAVING DISABLED"
#else
#define MOEMON_BUILD_VERSION MOEMON_VERSION
#define MOEMON_BUILD_CHANNEL "main"
#define MOEMON_BUILD_TITLE "Moemon Emerald Kai Co-op " MOEMON_VERSION
#endif

// Where the updater looks for releases (src/platform/updater.c)
#define MOEMON_UPDATE_REPO "christinebaker1212-hash/PokemonEmeraldKai"

#endif // GUARD_MP_VERSION_H
