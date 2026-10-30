#!/bin/bash
# Moemon Fight: tests of the asset ingestion (tools/fight/ingest.py) on a copy of mugen/: a first run records every
# asset; a second does nothing; a changed file, a new character, a removed stage, an interrupted run, --force and a
# proposal are each noticed and handled, and repairs aren't made twice.
cd "$(dirname "$0")/../.."
T=${TMPDIR:-/tmp}/moemon_ingest_test.$$
trap 'rm -rf "$T"' EXIT
mkdir -p "$T" && cp -r mugen "$T/" && rm -f "$T"/mugen/processed_assets.* "$T"/mugen/ingestion_log.txt
I="python3 tools/fight/ingest.py --root $T/mugen --quiet"
fail=0
ok() { echo "ok   $1"; }
bad() { echo "FAIL $1"; fail=1; }
$I && [ -s "$T/mugen/processed_assets.json" ] && [ -s "$T/mugen/processed_assets.idx" ] && ok "first run records the assets" || bad "first run"
$I --check && ok "second run: nothing to do" || bad "second run found work"
before=$(wc -l < "$T/mugen/ingestion_log.txt")
$I && [ "$(wc -l < "$T/mugen/ingestion_log.txt")" = "$before" ] && ok "nothing logged when nothing changed" || bad "logged without changes"
echo "; touched" >> "$T/mugen/chars/ssf2x_ken/ken.cns"
out=$($I --check 2>&1); echo "$out" | grep -q "char:chars/ssf2x_ken" && ! echo "$out" | grep -q "char:chars/ssf2x_ryu" \
    && ok "a changed file rechecks only its character" || bad "changed file: $out"
$I
cp -r "$T/mugen/chars/ssf2x_ryu" "$T/mugen/chars/newfighter"
mv "$T/mugen/chars/newfighter/ssf2x_ryu.def" "$T/mugen/chars/newfighter/newfighter.def"
sed -i -E 's/^type = null;(DisplayToClipboard|AppendToClipboard) ;switched off by ingestion[^\r]*/type = \1/' "$T/mugen/chars/newfighter/option.txt"
rm "$T/mugen/stages/sf2-ussr.def"
$I --check | grep -q "would process char:chars/newfighter" && ok "a new character folder is found" || bad "new character"
$I --check | grep -q "would drop stage:stages/sf2-ussr.def" && ok "a removed stage is noticed" || bad "removed stage"
$I && grep -q "removed from the pack" "$T/mugen/ingestion_log.txt" && ! grep -q '"stage:stages/sf2-ussr.def"' "$T/mugen/processed_assets.json" \
    && ok "the removed stage leaves the record" || bad "removed stage kept"
grep -q "debug text switched off in chars/newfighter" "$T/mugen/ingestion_log.txt" && ok "repairs the new character" || bad "no repair"
n=$(grep -c "debug text switched off in chars/ssf2x_ryu/" "$T/mugen/ingestion_log.txt")
$I --force && [ "$(grep -c "debug text switched off in chars/ssf2x_ryu/" "$T/mugen/ingestion_log.txt")" = "$n" ] \
    && grep -q "forced" "$T/mugen/ingestion_log.txt" && ok "--force processes again without repairing twice" || bad "--force"
MOEMON_INGEST_STOP_AFTER=3 $I --force; [ $? = 3 ] && grep -q '"in progress"' "$T/mugen/processed_assets.json" && ok "an interrupted run leaves its asset in progress" || bad "interrupt"
$I --check | grep -q "interrupted last time" && $I && $I --check && ok "the next run finishes it" || bad "resume"
sed -i 's/^value = 930 ;Moemon Fight/value = 935 ;Moemon Fight/' "$T/mugen/chars/ssf2x_feilong/feilong.cns"
$I && grep -q "proposed (not applied; --apply-proposals): chars/ssf2x_feilong/feilong.cns:[0-9]* 935 -> 930" "$T/mugen/ingestion_log.txt" \
    && grep -q "^value = 935" "$T/mugen/chars/ssf2x_feilong/feilong.cns" && ok "a likely typo is proposed, not applied" || bad "proposal"
$I --force --apply-proposals && grep -q "^value = 930" "$T/mugen/chars/ssf2x_feilong/feilong.cns" && ok "--apply-proposals applies it" || bad "apply"
[ $fail = 0 ] && echo "all ok"
exit $fail
