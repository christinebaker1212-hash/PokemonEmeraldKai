#!/bin/bash
# The release's object cache for the game's C files (Makefile's $(C_BUILDDIR)/%.o rule when OBJCACHE is set): their
# compile is a pipe (cpp | preproc | cc1 | as) that ccache can't see, so this keeps each object by a hash of all that
# decides it:
#   - the preprocessed source as cc1 gets it (every header, and the INCBIN data preproc puts in it),
#   - the contents of each file it .incbin's (the assembler reads those itself),
#   - the cc1 and as command lines (compiler, flags), and OBJCACHE_SALT (the compilers' own bytes, from
#     tools/release/ccache_setup.sh),
# and on a match copies the object it made before; otherwise it compiles exactly as the rule does and keeps the result.
# usage: CPP ... | PREPROC ... | tools/release/objcache.sh OUT.o "CC1 CFLAGS" "AS ASFLAGS"
set -eo pipefail
OUT=$1 CC1=$2 AS=$3
DIR=${OBJCACHE:?}
mkdir -p "$DIR"
SRC=$(mktemp)
trap 'rm -f "$SRC" "$SRC.o"' EXIT
cat > "$SRC"
KEY=$( {
    printf '%s\n%s\n%s\n' "$CC1" "$AS" "${OBJCACHE_SALT:-}"
    cat "$SRC"
    # (the files the assembler reads itself: .incbin "path" in the source's asm strings)
    { grep -o '\.incbin \\"[^\\"]*\\"' "$SRC" || true; } | sed 's/^\.incbin \\"//; s/\\"$//' | sort -u | while read -r f; do
        printf '%s\n' "$f"
        if [ -f "$f" ]; then sha256sum < "$f"; else echo missing; fi
    done
} | sha256sum | cut -c1-40)
HIT="$DIR/${KEY:0:2}/$KEY.o"
if [ -f "$HIT" ]; then
    cp "$HIT" "$OUT"
    touch "$HIT"   # (used by this run: kept by ccache_setup.sh --prune)
    exit 0
fi
$CC1 -o - - < "$SRC" | cat - <(echo -e ".text\n\t.align\t2, 0") | $AS -o "$SRC.o" -   # (stdin, as the rule: the same ".file")
mkdir -p "$DIR/${KEY:0:2}"
cp "$SRC.o" "$HIT.$$" && mv -f "$HIT.$$" "$HIT"
mv -f "$SRC.o" "$OUT"
