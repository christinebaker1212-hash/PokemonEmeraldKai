"""Compare the assembled rip with the ROM: every label ending in _<ROM address> is checked byte for
byte up to the next label, with relocated (pointer) fields masked."""
import re
import subprocess
import sys


def main(rom_path, obj):
    rom = open(rom_path, 'rb').read()
    data = subprocess.run(['objcopy', '-O', 'binary', '--only-section=.data', obj, '/dev/stdout'],
                          capture_output=True).stdout
    if not data:
        data = subprocess.run(['objcopy', '-O', 'binary', '--only-section=.text', obj, '/dev/stdout'],
                              capture_output=True).stdout
    syms = []
    for line in subprocess.run(['nm', '-n', obj], capture_output=True, text=True).stdout.splitlines():
        p = line.split()
        if len(p) == 3 and p[1].lower() in 'td':
            syms.append((int(p[0], 16), p[2]))
    relocs = set()
    for line in subprocess.run(['readelf', '-rW', obj], capture_output=True, text=True).stdout.splitlines():
        m = re.match(r'^([0-9a-f]{8})\s', line)
        if m:
            o = int(m.group(1), 16)
            relocs.update(range(o, o + 4))
    ok = bad = 0
    shown = 0
    for i, (off, name) in enumerate(syms):
        m = re.search(r'_([0-9A-F]{7})$', name)
        if not m:
            continue
        addr = int(m.group(1), 16) - 0x08000000
        end = syms[i + 1][0] if i + 1 < len(syms) else len(data)
        n = end - off
        if '_Text_' in name and 0xFF in data[off:end]:
            n = data.index(0xFF, off, end) - off + 1     # unreferenced text can sit between two labels
        mism = [k for k in range(n) if off + k not in relocs and (addr + k >= len(rom) or data[off + k] != rom[addr + k])]
        # a goto we inserted where ROM code falls through is allowed at the very end
        if mism and mism[0] >= n - 5 and data[off + n - 5] == 0x05:
            mism = []
        if mism:
            bad += 1
            if shown < 15:
                k = mism[0]
                print('MISMATCH %s +%d: rip %s rom %s' % (name, k, data[off + k:off + k + 8].hex(), rom[addr + k:addr + k + 8].hex()))
                shown += 1
        else:
            ok += 1
    print('byte check: %d blocks match the ROM, %d differ' % (ok, bad))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
