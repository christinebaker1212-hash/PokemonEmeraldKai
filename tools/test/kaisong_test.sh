#!/bin/bash
# Custom songs (.kaisong, src/mp/mp_kaisong.c) end to end: Moemon Music Studio exports a test song, the game plays it
# from a music folder, and its audio must equal the studio's own render sample for sample.
# usage: tools/test/kaisong_test.sh   (NOBUILD=1 skips both builds, OUT=dir). Exit code = number of failed checks.
#   1. a song made without the GBA's limits (30 tracks: past the 16 a music player had, 64 voices) for one place (mus_route101@route101) and the GBA
#      accurate one (10 tracks, 5 voices) everywhere: on Route 101 the place's plays, bit-exact; a warp to Route 102
#      (the same song) switches to the everywhere version, which is bit-exact too (played on its own: a warp's sounds
#      leave Game Boy channel state the studio doesn't have, as they do for the game's own songs)
#   2. the game's own Route 101 exported and played back: the same audio as the built-in song
#   3. a file naming a sample the game doesn't have: skipped with a log line, the built-in song plays
#   4. a version 2 file (its own sample) loads and plays
#   5. scenes: one song replaced for several scenes and places at once (BATTLE SIMULATOR, a GYM, the three card games);
#      each plays only its own file, and a new scene is found from its source alone (tools/kaisong/test_scenes.py)
#   6. INFO gain= (tools/kaisong/loudness.py --write): the exported Route 101 with gain=-6.0 plays 6 dB under the game's
#   7. the same music in two files for two floors (the GAME CENTER's 1F and 2F, two game songs): it plays on up the
#      stairs and back, and changes outside
cd "$(dirname "$0")/../.." || exit 99
REPO=$(pwd)
OUT=${OUT:-/tmp/moemon_kaisong_test}
STUDIO=tools/moemon_music/build/linux/moemon_music_studio
rm -rf "$OUT"; mkdir -p "$OUT"
if [ -z "$NOBUILD" ]; then
    make linux -j"$(nproc)" > "$OUT/build.log" 2>&1 || { echo "game build failed, see $OUT/build.log"; exit 99; }
    make -C tools/moemon_music -j"$(nproc)" > "$OUT/studio_build.log" 2>&1 || { echo "studio build failed"; exit 99; }
fi
fails=0
check() { local what=$1; shift; if "$@"; then echo "  ok   $what"; else echo "  FAIL $what"; fails=$((fails+1)); fi; }
has() { grep -qF -- "$2" "$1"; }

# the studio's side: a 15-track test MIDI, exported both ways, and rendered by its engine
python3 tools/moemon_music/test/make_test_midi.py "$OUT/big.mid" 25
printf 'gba_accurate=1\n' > "$OUT/acc.ini"; printf 'gba_accurate=0\n' > "$OUT/free.ini"
mkdir -p "$OUT/m1" "$OUT/m2" "$OUT/m3" "$OUT/m4" "$OUT/m5" "$OUT/m6" "$OUT/m7" "$OUT/empty"
MMS_DEFAULTS="$OUT/free.ini" $STUDIO --export "$OUT/big.mid" "$OUT/m1/mus_route101@route101.kaisong" > /dev/null
MMS_DEFAULTS="$OUT/acc.ini" $STUDIO --export "$OUT/big.mid" "$OUT/m1/mus_route101.kaisong" > /dev/null
cp "$OUT/m1/mus_route101.kaisong" "$OUT/m5/"
MMS_DEFAULTS="$OUT/free.ini" $STUDIO --raw "$OUT/big.mid" "$OUT/free.f32" 1400 > /dev/null
MMS_DEFAULTS="$OUT/acc.ini" $STUDIO --raw "$OUT/big.mid" "$OUT/acc.f32" 1400 > /dev/null
MMS_DEFAULTS="$OUT/acc.ini" $STUDIO --export sound/songs/midi/mus_route101.mid "$OUT/m2/mus_route101.kaisong" > /dev/null
python3 - "$OUT" <<'EOF'
# 3: a file that names a sample this game doesn't have; 4: a version 2 file with its own sample
import struct, sys, math
out = sys.argv[1]
d = bytearray(open(out + '/m2/mus_route101.kaisong', 'rb').read())
i = d.find(b'DirectSoundWaveData_')
d[i + 20:i + 23] = b'zzz'
open(out + '/m3/mus_route101.kaisong', 'wb').write(d)
d = open(out + '/m2/mus_route101.kaisong', 'rb').read()
hdr, chunks, p = bytearray(d[:32]), [], 32
while p < len(d):
    cid, n = d[p:p + 4], struct.unpack_from('<I', d, p + 4)[0]
    chunks.append([cid, bytearray(d[p + 8:p + 8 + n])])
    p += 8 + n + (-n & 3)
for c in chunks:
    if c[0] == b'VGRP':   # every sample voice of group 0 plays the song's own sine instead
        for v in range(128):
            o = 4 + v * 16
            if c[1][o + 8] == 1:
                c[1][o + 8] = 6; c[1][o + 10:o + 12] = b'\0\0'
size = 4000
pcm = bytes((int(100 * math.sin(2 * math.pi * k / 40)) & 0xFF) for k in range(size))
name = b'sine'
smpl = struct.pack('<IH', 1, len(name)) + name
smpl += b'\0' * (-len(smpl) & 3)
smpl += struct.pack('<HHIII', 0, 0x4000, 13379 << 10, 0, size) + pcm
smpl += b'\0' * (-len(smpl) & 3)
chunks += [[b'SMPL', bytearray(smpl)], [b'WAVE', bytearray(struct.pack('<I', 0))]]
struct.pack_into('<H', hdr, 8, 2)
struct.pack_into('<I', hdr, 16, len(chunks))
o = bytes(hdr)
for cid, body in chunks:
    o += cid + struct.pack('<I', len(body)) + bytes(body) + b'\0' * (-len(body) & 3)
open(out + '/m4/mus_route101.kaisong', 'wb').write(o)
# 6: the export with a gain
import shutil
sys.path.insert(0, 'tools/kaisong'); import loudness
shutil.copy(out + '/m2/mus_route101.kaisong', out + '/m6/mus_route101.kaisong')
loudness.write_gain(out + '/m6/mus_route101.kaisong', -6.0)
# 7: one music for two floors
for f in ('mus_game_corner@rustboro_city_game_center_1f', 'mus_poke_mart@rustboro_city_game_center_2f'):
    shutil.copy(out + '/m2/mus_route101.kaisong', out + '/m7/' + f + '.kaisong')
EOF

cat > "$OUT/cmp.py" <<'EOF'
# cmp.py GAME.f32 STUDIO.f32 [SECONDS]: the studio's render must appear in the game's audio, every sample equal
import numpy as np, sys
g = np.fromfile(sys.argv[1], dtype=np.float32); s = np.fromfile(sys.argv[2], dtype=np.float32)
n = 4096
c = np.nonzero(g == s[0])[0] if len(s) else []
idx = next((i for i in c if i % 2 == 0 and i + n <= len(g) and np.array_equal(g[i:i + n], s[:n])), None)
if idx is None:
    print('the song never starts in the game audio'); sys.exit(1)
m = min(len(g) - idx, len(s), int(float(sys.argv[3]) * 42048) * 2 if len(sys.argv) > 3 else len(s)) // 2 * 2
d = np.count_nonzero(g[idx:idx + m] != s[:m])
print('%.1f s compared, %d samples differ, rms %.3f' % (m / 2 / 42048, d, float(np.sqrt((s[:m] ** 2).mean()))))
sys.exit(1 if d or m < 42048 * 2 * 15 else 0)
EOF
cat > "$OUT/loud.py" <<'EOF'
import numpy as np, sys
g = np.fromfile(sys.argv[1], dtype=np.float32)[-42048 * 2 * 5:]
print('rms %.3f' % float(np.sqrt((g ** 2).mean())))
sys.exit(0 if np.sqrt((g ** 2).mean()) > 0.01 else 1)
EOF

run() {   # run NAME MUSICDIR SCRIPT...
    local d="$OUT/$1" m=$2; shift 2
    mkdir -p "$d"; printf '%s\n' "$@" > "$d/a.txt"
    MOEMON_DEBUG_WARP=0,16,7,10 TMO=120 tools/test/run1.sh "$d" MOEMON_MUSIC_DIR="$m" MOEMON_AUDIO_DUMP="$d/audio.f32" > /dev/null
}
run free "$OUT/m1" "pressuntil A field 5000" "wait 30" "mp bgm" "mp playsong 359" "wait 1350" "mp warp 0,17,20,10" "wait 200" "mp bgm" "quit"
run acc "$OUT/m5" "pressuntil A field 5000" "wait 30" "mp playsong 359" "wait 1350" "mp bgm" "quit"
run game "$OUT/empty" "pressuntil A field 5000" "wait 30" "mp playsong 359" "wait 1350" "quit"
run export "$OUT/m2" "pressuntil A field 5000" "wait 30" "mp playsong 359" "wait 1350" "quit"
run gain "$OUT/m6" "pressuntil A field 5000" "wait 30" "mp playsong 359" "wait 1350" "quit"
mkdir -p "$OUT/floors"; printf '%s\n' "pressuntil A field 5000" "wait 60" "mp bgm" "mp warp 11,18,10,3" "wait 300" "mp bgm" "mp warp 11,17,3,3" \
    "wait 300" "mp bgm" "mp warp 0,3,20,20" "wait 300" "mp bgm" "quit" > "$OUT/floors/a.txt"
MOEMON_DEBUG_WARP=11,17,5,8 TMO=120 tools/test/run1.sh "$OUT/floors" MOEMON_MUSIC_DIR="$OUT/m7" > /dev/null
run broken "$OUT/m3" "pressuntil A field 5000" "wait 60" "mp bgm" "quit"
run own "$OUT/m4" "pressuntil A field 5000" "wait 400" "mp bgm" "quit"
echo "custom songs:"
check "the place's song plays on Route 101 (30 tracks, 64 voices)" has "$OUT/free/log.txt" "mus_route101@route101.kaisong plays as mus_route101 here (30 tracks, 64 voices"
check "  on a music player with all 64 tracks" has "$OUT/free/log.txt" "tracks 30 voices 64 status ffff player tracks 64"
check "  and sounds exactly like the studio" python3 "$OUT/cmp.py" "$OUT/free/audio.f32" "$OUT/free.f32" 22
check "  a warp to Route 102 switches to the everywhere one (10 tracks, 5 voices)" has "$OUT/free/log.txt" "[test] bgm song 359 tracks 10 voices 5"
check "the GBA accurate one sounds exactly like the studio" python3 "$OUT/cmp.py" "$OUT/acc/audio.f32" "$OUT/acc.f32"
check "the game's Route 101 exported plays like the built-in song" cmp -s "$OUT/game/audio.f32" "$OUT/export/audio.f32"
check "  (and it is loaded from the file)" has "$OUT/export/log.txt" "mus_route101.kaisong plays as mus_route101 everywhere (8 tracks, 5 voices"
check "a file with gain=-6.0 plays 6 dB under the built-in song" python3 - "$OUT/game/audio.f32" "$OUT/gain/audio.f32" <<'EOF'
import numpy as np, sys
a, b = (np.fromfile(f, dtype=np.float32)[-42048 * 2 * 15:] for f in sys.argv[1:3])
d = 20 * np.log10(np.sqrt((b ** 2).mean()) / np.sqrt((a ** 2).mean()))
print('    %.2f dB' % d)
sys.exit(0 if abs(d + 6) < 0.3 else 1)
EOF
check "one music on two floors plays on up the stairs and back" python3 - "$OUT/floors/log.txt" <<'EOF'
import re, sys
c = [(int(a), int(b)) for a, b in re.findall(r'\[test\] bgm song (\d+) .* clock (\d+)', open(sys.argv[1], errors='replace').read())]
print('    ', c)
# 1F (mus_game_corner), 2F, 1F: the same song and a clock that only goes on; outside: Rustboro's song
sys.exit(0 if len(c) == 4 and c[0][0] == c[1][0] == c[2][0] == 426 and c[0][1] < c[1][1] < c[2][1] and c[3][0] == 399 else 1)
EOF
check "a file naming an unknown sample is skipped" has "$OUT/broken/log.txt" "which this game doesn't have"
check "  and the built-in song plays" has "$OUT/broken/log.txt" "[test] bgm song 359 tracks 8 voices 5"
check "a file with its own sample loads" has "$OUT/own/log.txt" "mus_route101.kaisong plays as mus_route101 everywhere"
check "  and plays" python3 "$OUT/loud.py" "$OUT/own/audio.f32"
# Scenes: the same song replaced for several scenes and places at once; each game plays only the file for where it is
#   sim_tent: the BATTLE SIMULATOR's battle (the tent's own map has a file for that song too)
#   join_gym: ROXANNE's battle at her gym (the card games have files for that song too)
#   tt_online / ygo_online / tcg_concede_online: a friend's card game (all three card games have one for that song)
S="$OUT/scenes"; mkdir -p "$S"
for f in mus_vs_trainer@sim mus_vs_trainer@slateport_city_battle_tent_lobby mus_vs_gym_leader@rustboro_city_gym \
         mus_vs_gym_leader@tcg mus_vs_gym_leader@tt mus_rg_union_room@tcg mus_rg_union_room@ygo mus_rg_union_room@tt mus_rg_vs_trainer@tt; do
    cp "$OUT/m2/mus_route101.kaisong" "$S/$f.kaisong"
done
scene_run() {   # scene_run SCENARIO: a copy of the scenario with the scenes' music folder
    local n=$1 d="$OUT/scene_$1"
    rm -rf "$d"; mkdir -p "$d"; cp tools/test/scenarios/$n/*.txt "$d/"
    ( unset MODE WARP WARP_B ENV_A ENV_B TMO NAME_A NAME_B
      . tools/test/scenarios/$n/conf
      export WARP WARP_B TMO NAME_A NAME_B FPS
      export ENV_A="$ENV_A MOEMON_MUSIC_DIR=$S MOEMON_MUSIC_LOG=1" ENV_B="$ENV_B MOEMON_MUSIC_DIR=$S MOEMON_MUSIC_LOG=1"
      tools/test/run2.sh "$d" 0 0 > "$d/run.txt" 2>&1 )
}
for n in sim_tent join_gym tt_online ygo_online tcg_concede_online; do scene_run $n & done; wait
loads() { grep -qF "kaisong: $2.kaisong plays as" "$OUT/scene_$1/a.log"; }
nloads() { ! loads "$1" "$2"; }
echo "scenes:"
check "the BATTLE SIMULATOR plays its own file" loads sim_tent mus_vs_trainer@sim
check "  not the tent map's (same song)" nloads sim_tent mus_vs_trainer@slateport_city_battle_tent_lobby
check "  its victory song is its own too" grep -qF "(mus_victory_trainer) for scene sim" "$OUT/scene_sim_tent/a.log"
check "  and the simulator still works" grep -qF "sim: streak 1 (best 1)" "$OUT/scene_sim_tent/a.log"
check "ROXANNE's battle plays her gym's file" loads join_gym mus_vs_gym_leader@rustboro_city_gym
check "  not the card games' (same song)" eval "nloads join_gym mus_vs_gym_leader@tcg && nloads join_gym mus_vs_gym_leader@tt"
check "TRIPLE TRIAD with a friend plays its own battle song" loads tt_online mus_rg_vs_trainer@tt
check "  not the other card games'" eval "nloads tt_online mus_rg_union_room@tcg && nloads tt_online mus_rg_union_room@ygo"
check "  and the map's own song after the game" grep -qF "(mus_route101) for the map" "$OUT/scene_tt_online/a.log"
check "YU-GI-OH! with a friend plays its own file" loads ygo_online mus_rg_union_room@ygo
check "  not the other card games'" eval "nloads ygo_online mus_rg_union_room@tcg && nloads ygo_online mus_rg_union_room@tt"
check "the TCG with a friend plays its own file" loads tcg_concede_online mus_rg_union_room@tcg
check "  not the other card games'" eval "nloads tcg_concede_online mus_rg_union_room@ygo && nloads tcg_concede_online mus_rg_union_room@tt"
check "a new scene is picked up from its source file alone" python3 tools/kaisong/test_scenes.py

for l in "$OUT"/*/log.txt "$OUT"/scene_*/?.log; do
    grep -q "Segmentation\|Aborted\|CRASH\|TIMEOUT" "$l" && { echo "  FAIL $(basename "$(dirname "$l")") crashed"; fails=$((fails+1)); }
done
echo "$fails failed, results in $OUT"
exit $fails
