// The Stadium hue math as the bluemoonfalls "Stadium hues" page (hue-shift.js) does it, for check_hues.py:
// reads the game's "[hue] ..." lines (mp huecheck) on stdin and prints the ones the page would answer differently.
// RGBToHSL / HSLToRGB and the NV / slider math are the page's, unchanged except for taking plain arguments.
const nameHexValues = {
    "A": "41", "B": "42", "C": "43", "D": "44", "E": "45", "F": "46", "G": "47", "H": "48",
    "I": "49", "J": "4A", "K": "4B", "L": "4C", "M": "4D", "N": "4E", "O": "4F", "P": "50",
    "Q": "51", "R": "52", "S": "53", "T": "54", "U": "55", "V": "56", "W": "57", "X": "58",
    "Y": "59", "Z": "5A",
    "a": "61", "b": "62", "c": "63", "d": "64", "e": "65", "f": "66", "g": "67", "h": "68",
    "i": "69", "j": "6A", "k": "6B", "l": "6C", "m": "6D", "n": "6E", "o": "6F", "p": "70",
    "q": "71", "r": "72", "s": "73", "t": "74", "u": "75", "v": "76", "w": "77", "x": "78",
    "y": "79", "z": "7A",
    " ": "20", "(": "28", ")": "29", ":": "3A", ";": "3B", "[": "5B", "]": "5C", "-": "2D",
    "?": "3F", "!": "21", ".": "2E", "/": "2F", ",": "2C",
    "Ä": "C4", "Ö": "D6", "Ü": "DC", "ä": "E4", "ö": "F6", "ü": "FC",
    "*": "D7", "#": "BC", "$": "BD", "%": "A9", "&": "BE",
    "0": "30", "1": "31", "2": "32", "3": "33", "4": "34", "5": "35", "6": "36", "7": "37",
    "8": "38", "9": "39"
};

function nv(id, ot, nick) {
    const otHex = Array.from(ot).map(x => nameHexValues[x]);
    const nickHex = Array.from(nick).map(x => nameHexValues[x]);
    const idHex = id.toString(16).padStart(4, '0').match(/.{1,2}/g);
    const allHex = otHex.concat(nickHex, idHex);
    return parseInt(allHex.reduce((prev, current) => (parseInt(prev, 16) + parseInt(current, 16)).toString(16)).slice(-2), 16);
}

// calcHue's math, then what an <input type=range min=-minHue max=maxHue step=1> does with the value
function degrees(NV, minHue, maxHue) {
    const range = minHue + maxHue;
    const increment = range / 255;
    const def = (255 / range) * minHue;
    const hue = (NV - def) * increment;
    if (!isFinite(hue))
        return 0;
    let v = Math.floor(hue + 0.5);   // nearest step, halves up
    if (v < -minHue) v = -minHue;
    if (v > maxHue) v = maxHue;
    return v;
}

function RGBToHSL(r, g, b) {
    r /= 255; g /= 255; b /= 255;
    let cmin = Math.min(r,g,b), cmax = Math.max(r,g,b), delta = cmax - cmin, h = 0, s = 0, l = 0;
    if (delta == 0) h = 0;
    else if (cmax == r) h = ((g - b) / delta) % 6;
    else if (cmax == g) h = (b - r) / delta + 2;
    else h = (r - g) / delta + 4;
    h = Math.round(h * 60);
    if (h < 0) h += 360;
    l = (cmax + cmin) / 2;
    s = delta == 0 ? 0 : delta / (1 - Math.abs(2 * l - 1));
    s = Number.parseInt(s * 100);
    l = Number.parseInt(l * 100);
    return [h, s, l];
}

function HSLToRGB(h, s, l) {
    s /= 100; l /= 100;
    let c = (1 - Math.abs(2 * l - 1)) * s, x = c * (1 - Math.abs((h / 60) % 2 - 1)), m = l - c/2, r = 0, g = 0, b = 0;
    if (0 <= h && h < 60) { r = c; g = x; b = 0; }
    else if (60 <= h && h < 120) { r = x; g = c; b = 0; }
    else if (120 <= h && h < 180) { r = 0; g = c; b = x; }
    else if (180 <= h && h < 240) { r = 0; g = x; b = c; }
    else if (240 <= h && h < 300) { r = x; g = 0; b = c; }
    else if (300 <= h && h < 360) { r = c; g = 0; b = x; }
    r = Math.round((r + m) * 255); g = Math.round((g + m) * 255); b = Math.round((b + m) * 255);
    return [r, g, b];
}

// hueShift() on one GBA color (5 bits a channel, shown as c<<3|c>>2; back to the nearest 5-bit value)
function shiftColor(c, hue) {
    let hueDiff = hue;
    if (hueDiff < 0) hueDiff += 360;
    const up = v => (v << 3) | (v >> 2);
    const hsl = RGBToHSL(up(c & 31), up((c >> 5) & 31), up((c >> 10) & 31));
    hsl[0] += hueDiff;
    hsl[0] = hsl[0] % 360;
    const rgb = HSLToRGB(hsl[0], hsl[1], hsl[2]).map(v => Math.floor((Math.min(255, Math.max(0, v)) * 31 + 127) / 255));
    return rgb[0] | (rgb[1] << 5) | (rgb[2] << 10);
}

const lines = require('fs').readFileSync(0, 'utf8').split('\n');
let checked = 0, bad = 0;
for (const line of lines) {
    let m, want, got;
    if ((m = line.match(/^\[hue\] nv (.*)\|(.*)\|(\d+) = (\d+)$/))) {
        want = nv(+m[3], m[1], m[2]); got = +m[4];
    } else if ((m = line.match(/^\[hue\] deg (\d+) (\d+) (\d+) (\d+) = (-?\d+)$/))) {
        want = degrees(+m[2], +m[3], +m[4]); got = +m[5];
    } else if ((m = line.match(/^\[hue\] mon (.*)\|(.*)\|(\d+) (\d+) (\d+) = (-?\d+)$/))) {
        want = degrees(nv(+m[3], m[1], m[2]), +m[4], +m[5]); got = +m[6];
    } else if ((m = line.match(/^\[hue\] col (-?\d+) (\d+) = (\d+)$/))) {
        want = shiftColor(+m[2], +m[1]); got = +m[3];
    } else
        continue;
    checked++;
    if (want !== got) {
        bad++;
        if (bad <= 20) console.log('differs: ' + line + ' (page: ' + want + ')');
    }
}
console.log('checked ' + checked + ', differ ' + bad);
process.exit(bad || !checked ? 1 : 0);
