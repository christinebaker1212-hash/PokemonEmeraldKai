#!/usr/bin/env python3
"""The DANCE machine's art and sounds (docs/RHYTHM_DECISIONS.md #6, #7, #11): DDR EXTREME's look, cut down to the
GBA's 240x160, into graphics/rhythm/rhythm.bin, which src/rhythm/rh_assets.c builds in.

Sources (the downloads in tools/rhythm/README.md):
  THEME     Inorizushi's DDR-EXTREME-JP-SM5 theme (its Graphics, BGAnimations, Fonts, Sounds)
  BEWARE    Beware's StepMania EXTREME build's stepmania/ folder (its NoteSkins/dance and the katzeextreme_beware
            announcer)

Pictures are shrunk with an area filter (each output pixel the average of what it covers, alpha-weighted), the
640x480 theme's by a third (the VS cabinet's scale by height) unless a size below says otherwise so it can be read.

rhythm.bin: "RHY1", u32 count, then count entries of (char name[64], u16 kind (0 picture, 1 sound), u16 w, u16 h,
u16 frames, u32 offset, u32 size), then the data: pictures RGBA8 (straight alpha), frames stacked downwards;
sounds as Ogg Vorbis files.
usage: THEME=... BEWARE=... python3 tools/rhythm/gen_assets.py
"""
import io, os, struct, subprocess, sys
from PIL import Image

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
THEME = os.environ.get('THEME', '')
BEWARE = os.environ.get('BEWARE', '')
OUT = os.path.join(ROOT, 'graphics/rhythm/rhythm.bin')

entries = []   # (name, kind, w, h, frames, data)


def area(im, w, h):
    """Shrink with an area filter, alpha-weighted (no dark fringes at the edges)."""
    im = im.convert('RGBA')
    pre = Image.new('RGBA', im.size)
    px = im.load()
    out = pre.load()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = px[x, y]
            out[x, y] = (r * a // 255, g * a // 255, b * a // 255, a)
    small = pre.resize((w, h), Image.BOX)
    sp = small.load()
    for y in range(h):
        for x in range(w):
            r, g, b, a = sp[x, y]
            sp[x, y] = (min(255, r * 255 // a), min(255, g * 255 // a), min(255, b * 255 // a), a) if a else (0, 0, 0, 0)
    return small


def frames_of(path, cols, rows):
    im = Image.open(path).convert('RGBA')
    fw, fh = im.width // cols, im.height // rows
    return [im.crop((c * fw, r * fh, c * fw + fw, r * fh + fh)) for r in range(rows) for c in range(cols)]


def crop_alpha(im, pad=1):
    box = im.getbbox()
    if not box:
        return im
    x0, y0, x1, y1 = box
    return im.crop((max(0, x0 - pad), max(0, y0 - pad), min(im.width, x1 + pad), min(im.height, y1 + pad)))


def add(name, ims, w=None, h=None, scale=None):
    """Pictures (one or more frames, all the same size) shrunk to w x h (or by scale, or to height h)."""
    if not isinstance(ims, list):
        ims = [ims]
    sw, sh = ims[0].size
    if scale is not None:
        w, h = max(1, round(sw * scale)), max(1, round(sh * scale))
    elif w is None:
        w = max(1, round(sw * h / sh))
    elif h is None:
        h = max(1, round(sh * w / sw))
    data = b''.join(area(im, w, h).tobytes() for im in ims)
    entries.append((name, 0, w, h, len(ims), data))


def add_sound(name, path):
    if path.lower().endswith('.ogg'):
        data = open(path, 'rb').read()
    else:
        data = subprocess.run(['ffmpeg', '-v', 'quiet', '-i', path, '-c:a', 'libvorbis', '-q:a', '5', '-f', 'ogg', '-'],
                              check=True, capture_output=True).stdout
    entries.append((name, 1, 0, 0, 1, data))


def T(*p):
    return os.path.join(THEME, *p)


def N(*p):
    return os.path.join(BEWARE, 'NoteSkins', 'dance', *p)


ARROW = 20
QUANTS = ['4th', '8th', '12th', '16th', '24th', '32nd', '48th', '64th']


def pick(fr, n):
    """n frames spread over an animation"""
    return [fr[i * len(fr) // n] for i in range(n)]


def note_skins():
    for q, qn in enumerate(QUANTS):
        add('vivid_%d' % q, pick(frames_of(N('default', 'Down Tap Note %s 8x8.png' % qn), 8, 8), 8), ARROW, ARROW)
        add('note_%d' % q, frames_of(N('note', 'Down Tap Note %s 4x1.png' % qn), 4, 1), ARROW, ARROW)
        add('solo_%d' % q, pick(frames_of(N('solo', 'Down Tap Note %s 8x4.png' % qn), 8, 4), 8), ARROW, ARROW)
    add('flat', pick(frames_of(N('flat', 'Down Tap Note 8x8.png'), 8, 8), 8), ARROW, ARROW)
    for state, sn in (('on', 'Active'), ('off', 'Inactive')):
        add('hold_head_' + state, Image.open(N('default', 'Down Hold Head %s.png' % sn)), ARROW, ARROW)
        add('hold_body_' + state, Image.open(N('default', 'Down Hold Body %s.png' % sn)), ARROW, ARROW * 2)
        add('hold_cap_' + state, Image.open(N('default', 'Down Hold BottomCap %s.png' % sn)), ARROW, ARROW)
    add('receptor', Image.open(N('default', 'Down Receptor Waiting.png')), ARROW, ARROW)
    add('receptor_go', frames_of(N('default', '_Down Receptor Go 2x1.png'), 2, 1), ARROW, ARROW)
    add('flash_bright', Image.open(N('default', 'Down Tap Explosion Bright.png')), 30, 30)
    add('flash_dim', Image.open(N('default', 'Down Tap Explosion Dim.png')), 30, 30)


def theme():
    G = lambda *p: T('Graphics', *p)
    B = lambda *p: T('BGAnimations', *p)
    F = lambda *p: T('Fonts', *p)
    # gameplay
    add('judge', frames_of(G('Player judgment', 'Judgment 1x6.png'), 1, 6), h=10)
    add('holdjudge', frames_of(G('HoldJudgment label 1x2.png'), 1, 2), h=10)
    add('combo_label', crop_alpha(Image.open(G('Player combo', '_label.png'))), h=7)
    add('combo_digits', frames_of(F('Combo numbers 5x3.png'), 5, 3)[:10], h=17)
    add('score_digits', frames_of(F('ScoreDisplayNormal Text 5x3.png'), 5, 3)[:10], h=9)
    add('eval_digits', frames_of(F('ScreenEvaluation ScoreNumber 5x3.png'), 5, 3)[:10], h=10)
    add('small_digits', frames_of(F('ScreenEvaluation JudgmentLineNumber 5x3.png'), 5, 3)[:10], h=7)
    add('meter_digits', frames_of(F('DifficultyMeter meter 5x3.png'), 5, 3)[:10], h=7)
    add('bpm_digits', frames_of(F('BPMDisplay bpm 5x3.png'), 5, 3)[:15], h=8)
    add('life_frame', Image.open(G('ScreenGameplay LifeFrame', 'normal.png')), 240, 16)
    add('score_frame', crop_alpha(Image.open(G('ScreenGameplay ScoreFrame', 'normal.png')), 0), w=240)
    for s in ('1st', '2nd', '3rd', '4th', 'Final'):
        add('stage_small_' + s.lower(), Image.open(G('ScreenGameplay StageDisplay', 'Stage_%s.png' % s)), h=8)
    for s in ('1', '2', '3', '4', 'final'):
        add('stage_' + s, crop_alpha(Image.open(B('ScreenStageInformation in', '_stage %s.png' % s))), h=22)
    add('ready', crop_alpha(Image.open(B('ScreenGameplay ready', 'ready.png'))), h=20)
    add('herewego', crop_alpha(Image.open(B('ScreenGameplay go', 'go.png'))), h=20)
    add('cleared', crop_alpha(Image.open(B('ScreenGameplay out', 'cleared.png'))), h=30)
    add('failed', crop_alpha(Image.open(B('ScreenGameplay failed', 'failed.png'))), h=30)
    add('gameover', crop_alpha(Image.open(B('ScreenGameOver background', 'gameover.png'))), w=150)
    add('grades', frames_of(G('ScreenEvaluation grades 1x8.png'), 1, 8), h=26)
    add('grades_small', frames_of(G('ScreenEvaluation grades 1x8.png'), 1, 8), h=12)
    add('caution', Image.open(B('ScreenCaution background', 'en_centered.png')), scale=1 / 3)
    # the menus' green background, the 16:9 picture's middle 3:2
    bg = Image.open(B('ScreenWithMenuElements background', 'back 16_9.png')).convert('RGBA')
    add('menu_bg', bg.crop(((bg.width - 720) // 2, 0, (bg.width - 720) // 2 + 720, 480)), 240, 160)
    add('footer', Image.open(B('footer.png')), w=240)
    add('style_single', Image.open(B('ScreenSelectStyle decorations', 'en_single.png')), scale=1 / 3)
    add('style_versus', Image.open(B('ScreenSelectStyle decorations', 'en_versus.png')), scale=1 / 3)
    for c in ('BEGINNER', 'LIGHT', 'STANDARD', 'HEAVY', 'ONI'):
        add('card_' + c.lower(), Image.open(B('ScreenSelectDifficulty decorations', 'Cards', 'en', c + ' P1.png')),
            scale=1 / 3)
    add('diff_icons', frames_of(B('ScreenSelectMusic decorations', 'diff', '_difficulty icons 1x6.png'), 1, 6), h=12)
    add('eval_frame', Image.open(B('ScreenEvaluation decorations', 'frame1.png')), scale=1 / 3)
    add('banner_frame', Image.open(B('ScreenSelectMusic decorations', 'BannerFrame.png')), scale=1 / 2)


def sounds():
    S = lambda *p: T('Sounds', *p)
    add_sound('snd_menu_music', S('_common menu music (loop).ogg'))
    add_sound('snd_change', S('_common change.ogg'))
    add_sound('snd_row', S('_common row.ogg'))
    add_sound('snd_start', S('Common start.ogg'))
    add_sound('snd_back', S('Common back.mp3'))
    add_sound('snd_coin', S('Common coin.ogg'))
    add_sound('snd_wheel', S('MusicWheel change.mp3'))
    add_sound('snd_sort', S('MusicWheel sort.mp3'))
    add_sound('snd_expand', S('MusicWheel expand.mp3'))
    add_sound('snd_harder', S('ScreenSelectMusic difficulty harder.ogg'))
    add_sound('snd_diffchange', S('ScreenSelectdifficulty change.mp3'))
    add_sound('snd_swoosh', S('_swoosh.ogg'))
    add_sound('snd_prompt', S('_prompt.ogg'))
    add_sound('snd_unlock', S('_unlock.ogg'))
    # the announcer: every line, named by its folder ("ann:gameplay ready:0")
    A = os.path.join(BEWARE, 'Announcers', 'katzeextreme_beware')
    for folder in sorted(os.listdir(A)):
        d = os.path.join(A, folder)
        if not os.path.isdir(d):
            continue
        files = sorted(f for f in os.listdir(d) if f.lower().endswith('.ogg') and os.path.getsize(os.path.join(d, f)) > 200)
        for i, f in enumerate(files):
            add_sound('ann:%s:%d' % (folder, i), os.path.join(d, f))


def main():
    if not os.path.isdir(THEME) or not os.path.isdir(BEWARE):
        sys.exit('set THEME (DDR-EXTREME-JP-SM5) and BEWARE (the build\'s stepmania folder): tools/rhythm/README.md')
    note_skins()
    theme()
    sounds()
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    head = b'RHY1' + struct.pack('<I', len(entries))
    table = b''
    blob = b''
    base = len(head) + len(entries) * 80
    for name, kind, w, h, n, data in entries:
        assert len(name) < 64, name
        while (base + len(blob)) % 4:
            blob += b'\0'
        table += struct.pack('<64sHHHHII', name.encode(), kind, w, h, n, base + len(blob), len(data))
        blob += data
    open(OUT, 'wb').write(head + table + blob)
    pics = sum(1 for e in entries if e[1] == 0)
    print('%s: %d pictures, %d sounds, %d KB' % (OUT, pics, len(entries) - pics, (len(head) + len(table) + len(blob)) // 1024))


if __name__ == '__main__':
    main()
