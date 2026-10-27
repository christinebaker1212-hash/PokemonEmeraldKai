/*
 * Common header of the engine sources. Include it first.
 */
#ifndef MUGEN_H
#define MUGEN_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <SDL.h>
#include <unzip.h>
#include <png.h>

#include "crt.h"
#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC 1
#endif
#include <pthread.h>    /* Moemon Fight: the mutex types in GameVars and the BGM globals */
#include "ftol.h"

/* Version reported by the original build. */
#define MUGEN_VERSION_MAJOR 1
#define MUGEN_VERSION_MINOR 1
#define MUGEN_VERSION_PATCH 0
#define MUGEN_VERSION_TAG   "Beta 1 P1"
#define MUGEN_BUILD_YEAR    2013
#define MUGEN_BUILD_MONTH   8
#define MUGEN_BUILD_DAY     11

/* Checks that a decompiled struct matches the layout of the original.
 * Moemon Fight: only on 32-bit x86, where pointers are 4 bytes as in the original. */
#if defined(_M_IX86) || defined(__i386__)
#define MUGEN_ASSERT_SIZE(type, size) \
    _Static_assert(sizeof(type) == (size), "sizeof(" #type ") != " #size)
#define MUGEN_ASSERT_OFFSET(type, field, offset) \
    _Static_assert(offsetof(type, field) == (offset), "offsetof(" #type ", " #field ") != " #offset)
#else
#define MUGEN_ASSERT_SIZE(type, size) _Static_assert(1, "")
#define MUGEN_ASSERT_OFFSET(type, field, offset) _Static_assert(1, "")
#endif

#include "util.h"
#include "stream.h"
#include "ini.h"
#include "palette.h"
#include "keyedarray.h"
#include "hashtable.h"
#include "luautil.h"
#include "ai.h"
#include "sprite.h"
#include "sff.h"
#include "textring.h"
#include "log.h"
#include "gfx.h"
#include "glrender.h"
#include "video.h"
#include "events.h"
#include "sound.h"
#include "bgm.h"
#include "keybind.h"
#include "font.h"
#include "syselem.h"
#include "fight.h"
#include "clsn.h"
#include "anim.h"
#include "bg.h"
#include "stage.h"
#include "expr.h"
#include "trigger.h"
#include "select.h"
#include "title.h"
#include "motif.h"
#include "storyboard.h"
#include "game.h"
#include "console.h"
#include "char.h"
#include "player.h"
#include "cnsparse.h"
#include "statedef.h"
#include "playerstate.h"
#include "sctrl.h"
#include "result.h"
#include "pngio.h"
#include "spritecodec.h"

#include "symbols.h"

#include "mf_port.h"     /* Moemon Fight: files, time and exit go through the game */

#endif /* MUGEN_H */
