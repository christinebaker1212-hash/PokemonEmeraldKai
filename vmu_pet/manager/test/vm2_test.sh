#!/bin/sh
# Moemon VMU manager: every VM2 command against the pretend VM2 (fake_vm2.py), through vm2cli (the manager's own code).
set -e
cd "$(dirname "$0")/.."
B=../build/manager
T=../build/vm2test
rm -rf $T && mkdir -p $T/sd/GID001
python3 -c "import sys;sys.path.insert(0,'../tools');import vmufs;open('$T/sd/VMU001.BIN','wb').write(vmufs.Image().raw)"
printf 'SONIC ADVENTURE 2' > $T/sd/GID001/title.txt
python3 test/fake_vm2.py $T/sd $T/int.bin $T/port & PID=$!
trap 'kill $PID 2>/dev/null' EXIT
sleep 1
C="$B/vm2cli --port $(cat $T/port)"
$C info | grep -q "firmware 1.5.5"                       && echo "PASS info"
$C ls | grep -q "GID001.*SONIC ADVENTURE 2"              && echo "PASS list (folder titles)"
$C get '\VMU001.BIN' $T/a.bin && cmp -s $T/a.bin $T/sd/VMU001.BIN && echo "PASS read a file"
$C put $T/a.bin '\GID001\GAME.VMU' && cmp -s $T/sd/GID001/GAME.VMU $T/a.bin && echo "PASS write a file into a folder"
$C mv '\GID001\GAME.VMU' '\GID001\OLD.VMU' && [ -f $T/sd/GID001/OLD.VMU ] && echo "PASS rename"
$C cp '\VMU001.BIN' '\GID001\TWO.BIN' && $C rm '\GID001\TWO.BIN' && [ ! -f $T/sd/GID001/TWO.BIN ] && echo "PASS copy, delete"
$C mkdir '\NEW' && $C rmdir '\NEW' && [ ! -d $T/sd/NEW ] && echo "PASS folders"
$C writeint $T/a.bin && $C readint $T/b.bin && cmp -s $T/a.bin $T/b.bin && echo "PASS internal memory"
$C clock && echo "PASS clock"
