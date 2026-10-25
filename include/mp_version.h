#ifndef GUARD_MP_VERSION_H
#define GUARD_MP_VERSION_H

// Moemon Co-op: this build's version. tools/release/package.sh puts it in moemon_release.txt and update.txt,
// and the GitHub release that ships it is tagged v<this>. Bump it for every release (the updater only
// installs a release whose version is higher than the running one).
#define MOEMON_VERSION "1.18.4"

// Where the updater looks for releases (src/platform/updater.c)
#define MOEMON_UPDATE_REPO "christinebaker1212-hash/PokemonEmeraldKai"

#endif // GUARD_MP_VERSION_H
