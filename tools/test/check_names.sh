#!/bin/bash
# Flags player / Pokémon name copies that may be left without an EOS.
#   StringCopyN / StringAppendN copy exactly N bytes and add no EOS. A name that fills its whole
#   length (SATOSHI, TAKESHI: 7 letters) has no EOS in those bytes, so the text runs on into whatever
#   follows. Seen as a garbled name in the CO-OP menu, and a crash in GetStringWidth once a stray
#   0xF7 (CHAR_DYNAMIC) turned up in the overrun (1.3.2, four players on one PC).
#   Safe forms: `*StringAppendN(dst, name, LEN) = EOS;`, `dst[LEN] = EOS;` within the next 3 lines, a copy of
#   LEN + 1 bytes, or a buffer cleared (memset / Clear...) on the line before.
# usage: tools/test/check_names.sh [FILES...]   (default: src/**/*.c). Exit code = findings.
cd "$(dirname "$0")/../.." || exit 99
[ $# -gt 0 ] || set -- $(find src -name '*.c')
python3 - "$@" <<'EOF'
import re, sys
call = re.compile(r'String(?:Copy|Append)N\s*\((.*)(NAME_LENGTH|MP_NAME_LEN)')
found = 0
for path in sys.argv[1:]:
    lines = open(path, encoding='utf-8', errors='replace').read().split('\n')
    for i, line in enumerate(lines):
        m = call.search(line)
        if not m or 'NAME_LENGTH + 1' in line or '_LEN + 1' in line:
            continue                         # copies the EOS too
        if re.search(r'^\s*\*\s*String(?:Copy|Append)N.*=\s*EOS', line):
            continue                         # *StringAppendN(...) = EOS
        after = '\n'.join(lines[i + 1:i + 4])
        before = lines[i - 1] if i else ''
        if re.search(r'\[[^\]]*(NAME_LENGTH|NAME_LEN)[^\]]*\]\s*=\s*EOS', after):
            continue                         # dst[LEN] = EOS just after (an if / else pair too)
        if 'memset' in before or 'Clear' in before:
            continue                         # the buffer was filled with EOS just before
        print('%s:%d: name copy without an EOS: %s' % (path, i + 1, line.strip()))
        found += 1
print('%d name cop%s without an EOS' % (found, 'y' if found == 1 else 'ies'))
sys.exit(min(found, 99))
EOF
