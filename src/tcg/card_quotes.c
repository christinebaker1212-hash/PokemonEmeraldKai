/* What card players say before and after a game (include/tcg/quotes.h): the three card games share it. {GAME} is
 * "duel" (POKeMON cards, Yu-Gi-Oh!) or "game" (TRIPLE TRIAD), {GAMES} the plural, {DECK} "deck" or "hand". Lines are
 * at most 36 letters, two to a page (tools/tcg/test_quotes.c checks). Friendly and competitive: never unkind. */
#include "tcg/quotes.h"
#include <stdio.h>
#include <string.h>

enum { S_ANY, S_FIRST, S_YOU_AHEAD, S_THEY_AHEAD, S_EVEN };
#define H QUOTE_HELLO
#define W QUOTE_THEY_WON
#define L QUOTE_THEY_LOST
struct Line
{
    unsigned char voice, when, situation;
    const char *text;
};
static const struct Line sLines[] = {
    /* ---- KID ---- */
    {QUOTE_VOICE_KID, H, S_ANY, "Ooh, a {GAME}! I just fixed up my\n{DECK}. Let's try it out!"},
    {QUOTE_VOICE_KID, H, S_ANY, "I practiced all morning!\nReady? Here I go!"},
    {QUOTE_VOICE_KID, H, S_ANY, "My {DECK} is my treasure!\nLet's have a great {GAME}!"},
    {QUOTE_VOICE_KID, H, S_ANY, "Rock, paper... no, {CARDS}!\nLet's play!"},
    {QUOTE_VOICE_KID, H, S_FIRST, "You're new around here, right?\nLet's be card friends!"},
    {QUOTE_VOICE_KID, H, S_FIRST, "I've never seen you play before!\nShow me your best stuff!"},
    {QUOTE_VOICE_KID, H, S_YOU_AHEAD, "You keep beating me... but this\ntime I've got a plan!"},
    {QUOTE_VOICE_KID, H, S_YOU_AHEAD, "I studied all your moves!\nThis time it's my turn to win!"},
    {QUOTE_VOICE_KID, H, S_THEY_AHEAD, "Back for more? Hehe, I'm on a\nroll! Don't go easy, OK?"},
    {QUOTE_VOICE_KID, H, S_THEY_AHEAD, "I won last time! But you looked\nreally close. Let's go!"},
    {QUOTE_VOICE_KID, H, S_EVEN, "We're tied up! Winner of this\none is the champ!"},
    {QUOTE_VOICE_KID, W, S_ANY, "Yay! I won! That was so fun!\nLet's play again sometime!"},
    {QUOTE_VOICE_KID, W, S_ANY, "Wow, that was close! My {DECK}\npulled through!"},
    {QUOTE_VOICE_KID, W, S_FIRST, "I won our first {GAME}!\nYou were really good, though!"},
    {QUOTE_VOICE_KID, W, S_EVEN, "Hehe! Now we're tied!\nNext one decides it!"},
    {QUOTE_VOICE_KID, W, S_THEY_AHEAD, "Another win for me! But you're\ngetting better every time!"},
    {QUOTE_VOICE_KID, L, S_ANY, "Aww, I lost! You're amazing!\nTeach me that move sometime!"},
    {QUOTE_VOICE_KID, L, S_ANY, "No fair, you're so good!\nI'm gonna practice even harder!"},
    {QUOTE_VOICE_KID, L, S_FIRST, "You beat me on the first try!\nYou have to play me again!"},
    {QUOTE_VOICE_KID, L, S_YOU_AHEAD, "You win again! Someday I'll\ncatch up to you, just watch!"},
    {QUOTE_VOICE_KID, L, S_EVEN, "Argh, we're even now!\nThe next {GAME} is a big one!"},

    /* ---- STYLISH ---- */
    {QUOTE_VOICE_STYLISH, H, S_ANY, "A {GAME}? Let's make it a stylish\none."},
    {QUOTE_VOICE_STYLISH, H, S_ANY, "Every card in my {DECK} is a\nfavorite. Shall we?"},
    {QUOTE_VOICE_STYLISH, H, S_ANY, "Winning is nice. Winning with\nflair is better. Let's play!"},
    {QUOTE_VOICE_STYLISH, H, S_ANY, "I picked my cards to match my\noutfit. Ready when you are!"},
    {QUOTE_VOICE_STYLISH, H, S_FIRST, "A new face! Let me see what kind\nof player you are."},
    {QUOTE_VOICE_STYLISH, H, S_FIRST, "First time we've crossed {CARDS}?\nLet's make it memorable."},
    {QUOTE_VOICE_STYLISH, H, S_YOU_AHEAD, "You've had the upper hand so far.\nI've been working on that."},
    {QUOTE_VOICE_STYLISH, H, S_YOU_AHEAD, "Your style is winning, I'll admit.\nTime for mine to shine!"},
    {QUOTE_VOICE_STYLISH, H, S_THEY_AHEAD, "Back again? I like your spirit.\nLet's see a new trick."},
    {QUOTE_VOICE_STYLISH, H, S_EVEN, "Even so far. I do love a\nclose rivalry."},
    {QUOTE_VOICE_STYLISH, W, S_ANY, "And that's how it's done.\nThat was a lovely {GAME}!"},
    {QUOTE_VOICE_STYLISH, W, S_ANY, "Mm, a fine finish! You kept me\non my toes, though."},
    {QUOTE_VOICE_STYLISH, W, S_FIRST, "A win for our first {GAME}!\nYou made it look good, too."},
    {QUOTE_VOICE_STYLISH, W, S_EVEN, "Level again! I knew we'd make\na great pair of rivals."},
    {QUOTE_VOICE_STYLISH, L, S_ANY, "Well played! You've got real\nstyle at the table."},
    {QUOTE_VOICE_STYLISH, L, S_ANY, "Oh! I didn't see that coming.\nBravo, truly."},
    {QUOTE_VOICE_STYLISH, L, S_FIRST, "Beaten on our first {GAME}...\nYou're one to watch!"},
    {QUOTE_VOICE_STYLISH, L, S_YOU_AHEAD, "You've done it again! I'll be\nback with a fresh look."},
    {QUOTE_VOICE_STYLISH, L, S_EVEN, "We're even now. How exciting!\nRematch soon?"},

    /* ---- TOUGH ---- */
    {QUOTE_VOICE_TOUGH, H, S_ANY, "Hah! Cards build muscle too.\nMental muscle! Let's go!"},
    {QUOTE_VOICE_TOUGH, H, S_ANY, "I play hard and I play fair.\nGive me your best!"},
    {QUOTE_VOICE_TOUGH, H, S_ANY, "My {DECK} is solid as a boulder!\nTry and move it!"},
    {QUOTE_VOICE_TOUGH, H, S_ANY, "Nothing like a good {GAME} after\ntraining! Let's do this!"},
    {QUOTE_VOICE_TOUGH, H, S_FIRST, "Haven't seen you at the table\nbefore! Let's test your grit!"},
    {QUOTE_VOICE_TOUGH, H, S_FIRST, "A new challenger! I respect\nthat. Don't hold back!"},
    {QUOTE_VOICE_TOUGH, H, S_YOU_AHEAD, "You've got me beat so far.\nI've trained hard for this!"},
    {QUOTE_VOICE_TOUGH, H, S_YOU_AHEAD, "Back for another round! This\ntime I won't go down easy!"},
    {QUOTE_VOICE_TOUGH, H, S_THEY_AHEAD, "Ready for another workout?\nI'll go all out again!"},
    {QUOTE_VOICE_TOUGH, H, S_EVEN, "Even match so far! Let's break\nthe tie, fair and square!"},
    {QUOTE_VOICE_TOUGH, W, S_ANY, "Ha ha! A hard-won victory!\nYou gave me a real fight!"},
    {QUOTE_VOICE_TOUGH, W, S_ANY, "Phew! That was a workout!\nGood {GAME}, my friend!"},
    {QUOTE_VOICE_TOUGH, W, S_FIRST, "I win this one! But you've got\nfire. Come back stronger!"},
    {QUOTE_VOICE_TOUGH, W, S_EVEN, "That evens it up! Let's keep\nthis rivalry going!"},
    {QUOTE_VOICE_TOUGH, L, S_ANY, "Ugh! You got me! That was\nsome strong play!"},
    {QUOTE_VOICE_TOUGH, L, S_ANY, "Outmuscled at cards! Ha! I'll\nhit the training ground."},
    {QUOTE_VOICE_TOUGH, L, S_FIRST, "Beat me on the first go?\nNow that's real strength!"},
    {QUOTE_VOICE_TOUGH, L, S_YOU_AHEAD, "You win again! You're tough as\nnails, you know that?"},
    {QUOTE_VOICE_TOUGH, L, S_EVEN, "And we're tied! Next time,\nI'm bringing everything!"},

    /* ---- SEA ---- */
    {QUOTE_VOICE_SEA, H, S_ANY, "A {GAME}? Perfect weather for it!\nLet's set sail!"},
    {QUOTE_VOICE_SEA, H, S_ANY, "I've played {CARDS} on every beach\nin HOENN. Let's go!"},
    {QUOTE_VOICE_SEA, H, S_ANY, "Waves come and go, but a good\n{GAME} lasts! Ready?"},
    {QUOTE_VOICE_SEA, H, S_ANY, "My {DECK} rides the tide!\nHop aboard!"},
    {QUOTE_VOICE_SEA, H, S_FIRST, "A new sailor at my table!\nWelcome aboard!"},
    {QUOTE_VOICE_SEA, H, S_FIRST, "Haven't seen you on these waters.\nLet's see how you swim!"},
    {QUOTE_VOICE_SEA, H, S_YOU_AHEAD, "You've been riding a big wave.\nTime for the tide to turn!"},
    {QUOTE_VOICE_SEA, H, S_THEY_AHEAD, "Back on deck, are you?\nLet's make some waves!"},
    {QUOTE_VOICE_SEA, H, S_EVEN, "Even waters so far! Let's see\nwho reaches shore first!"},
    {QUOTE_VOICE_SEA, W, S_ANY, "Land ho! That {GAME} was smooth\nsailing for me!"},
    {QUOTE_VOICE_SEA, W, S_ANY, "Whew, rough seas there! You\nnearly capsized me!"},
    {QUOTE_VOICE_SEA, W, S_FIRST, "A win to start! You've got good\nsea legs, though."},
    {QUOTE_VOICE_SEA, W, S_EVEN, "That brings us level! The sea\nloves a close race."},
    {QUOTE_VOICE_SEA, L, S_ANY, "You sank my {DECK}! What a\nfine {GAME} that was!"},
    {QUOTE_VOICE_SEA, L, S_ANY, "Swept away by the current!\nWell played!"},
    {QUOTE_VOICE_SEA, L, S_FIRST, "First voyage and you win!\nYou're a natural!"},
    {QUOTE_VOICE_SEA, L, S_YOU_AHEAD, "You win again! You must have\nthe wind at your back."},
    {QUOTE_VOICE_SEA, L, S_EVEN, "Tied up like a ship at port!\nCome back for another!"},

    /* ---- REFINED ---- */
    {QUOTE_VOICE_REFINED, H, S_ANY, "Ah, a {GAME}. Shall we enjoy\na civilized match?"},
    {QUOTE_VOICE_REFINED, H, S_ANY, "I've played for many years.\nI'd love to see your approach."},
    {QUOTE_VOICE_REFINED, H, S_ANY, "A good {GAME} is like fine tea.\nBest enjoyed slowly."},
    {QUOTE_VOICE_REFINED, H, S_ANY, "Let us both play our very best,\nwhatever the outcome."},
    {QUOTE_VOICE_REFINED, H, S_FIRST, "A pleasure to meet you.\nShall we begin?"},
    {QUOTE_VOICE_REFINED, H, S_FIRST, "A new opponent! How delightful.\nAfter you."},
    {QUOTE_VOICE_REFINED, H, S_YOU_AHEAD, "You have bested me before.\nI've been studying, you see."},
    {QUOTE_VOICE_REFINED, H, S_YOU_AHEAD, "Again you visit! I hope to\ngive you a better match."},
    {QUOTE_VOICE_REFINED, H, S_THEY_AHEAD, "Back again? Persistence is a\nvirtue. Let us play."},
    {QUOTE_VOICE_REFINED, H, S_EVEN, "Our record stands even.\nHow fitting. Shall we?"},
    {QUOTE_VOICE_REFINED, W, S_ANY, "A splendid {GAME}. You made me\nwork for every card."},
    {QUOTE_VOICE_REFINED, W, S_ANY, "Experience wins out, this time.\nYou played wonderfully."},
    {QUOTE_VOICE_REFINED, W, S_FIRST, "Thank you for a lovely first\n{GAME}. Do come again."},
    {QUOTE_VOICE_REFINED, W, S_EVEN, "And now we're even. What a\ncharming rivalry."},
    {QUOTE_VOICE_REFINED, L, S_ANY, "Marvelous! You played with real\nskill. I tip my hat."},
    {QUOTE_VOICE_REFINED, L, S_ANY, "Oh my! You've a fine mind for\nthe game. Well done."},
    {QUOTE_VOICE_REFINED, L, S_FIRST, "Defeated at our first meeting!\nYou have a bright future."},
    {QUOTE_VOICE_REFINED, L, S_YOU_AHEAD, "Once more you prevail. It is\nan honor to play you."},
    {QUOTE_VOICE_REFINED, L, S_EVEN, "Even once more. Splendid!\nI await our next match."},

    /* ---- BRAINY ---- */
    {QUOTE_VOICE_BRAINY, H, S_ANY, "I've calculated my odds.\nLet's test the hypothesis!"},
    {QUOTE_VOICE_BRAINY, H, S_ANY, "Every {GAME} is new data.\nShall we begin?"},
    {QUOTE_VOICE_BRAINY, H, S_ANY, "My {DECK} is optimized. Mostly.\nLet's find out!"},
    {QUOTE_VOICE_BRAINY, H, S_ANY, "I sense a fascinating {GAME}\nahead of us..."},
    {QUOTE_VOICE_BRAINY, H, S_FIRST, "A new subject! Er, opponent.\nLet's play!"},
    {QUOTE_VOICE_BRAINY, H, S_FIRST, "I have no data on you yet.\nHow exciting!"},
    {QUOTE_VOICE_BRAINY, H, S_YOU_AHEAD, "You've won more often. I've\nadjusted my strategy!"},
    {QUOTE_VOICE_BRAINY, H, S_YOU_AHEAD, "My notes on you fill a whole\nbook now. Let's play!"},
    {QUOTE_VOICE_BRAINY, H, S_THEY_AHEAD, "The numbers favor me, but\nnumbers can change. Ready?"},
    {QUOTE_VOICE_BRAINY, H, S_EVEN, "A perfect 50 percent!\nLet's break the symmetry."},
    {QUOTE_VOICE_BRAINY, W, S_ANY, "Just as I predicted! Well,\nmostly. Good {GAME}!"},
    {QUOTE_VOICE_BRAINY, W, S_ANY, "Fascinating! Your play gave me\nso much to think about."},
    {QUOTE_VOICE_BRAINY, W, S_FIRST, "A first win for my research!\nYou were a worthy test."},
    {QUOTE_VOICE_BRAINY, W, S_EVEN, "The results are even again.\nMore trials are needed!"},
    {QUOTE_VOICE_BRAINY, L, S_ANY, "Astonishing! That wasn't in\nmy calculations at all!"},
    {QUOTE_VOICE_BRAINY, L, S_ANY, "I lost, but I learned. That\nmakes it worthwhile!"},
    {QUOTE_VOICE_BRAINY, L, S_FIRST, "Beaten on the first trial!\nYou're quite a variable."},
    {QUOTE_VOICE_BRAINY, L, S_YOU_AHEAD, "Another loss... Your method\nis simply superior!"},
    {QUOTE_VOICE_BRAINY, L, S_EVEN, "Even again. The experiment\nmust continue!"},

    /* ---- TEAM (Aqua and Magma, at the card table: playful rivals) ---- */
    {QUOTE_VOICE_TEAM, H, S_ANY, "Our TEAM trains with {CARDS} too!\nLet's see what you've got!"},
    {QUOTE_VOICE_TEAM, H, S_ANY, "No plans to foil here, kid.\nJust a {GAME}. Let's go!"},
    {QUOTE_VOICE_TEAM, H, S_ANY, "My boss says {CARDS} build focus.\nTime to focus!"},
    {QUOTE_VOICE_TEAM, H, S_FIRST, "Never dueled a hero type before.\nThis'll be fun!"},
    {QUOTE_VOICE_TEAM, H, S_YOU_AHEAD, "You keep winning! I've been\npracticing on my breaks!"},
    {QUOTE_VOICE_TEAM, H, S_THEY_AHEAD, "Back for another loss? Kidding!\nLet's have a good one!"},
    {QUOTE_VOICE_TEAM, H, S_EVEN, "Even score! The TEAM is\ncounting on me this time!"},
    {QUOTE_VOICE_TEAM, W, S_ANY, "Ha! A win for the TEAM!\nThat was a good {GAME}, though."},
    {QUOTE_VOICE_TEAM, W, S_FIRST, "First round to me! You're not\nbad for a do-gooder."},
    {QUOTE_VOICE_TEAM, W, S_EVEN, "Evened it up! Can't wait to\ntell the others!"},
    {QUOTE_VOICE_TEAM, L, S_ANY, "Argh! Beaten again! You're\nreally something, kid."},
    {QUOTE_VOICE_TEAM, L, S_ANY, "Nooo! Don't tell the boss\nabout this, OK?"},
    {QUOTE_VOICE_TEAM, L, S_FIRST, "Beat me first try?! Fine...\nThat was a fair win."},
    {QUOTE_VOICE_TEAM, L, S_YOU_AHEAD, "Another one for you! I'll\nneed a whole new {DECK}..."},
    {QUOTE_VOICE_TEAM, L, S_EVEN, "Tied again! This rivalry is\ngetting serious!"},

    /* ---- STAFF (the clerks and nurses who play on their breaks) ---- */
    {QUOTE_VOICE_STAFF, H, S_ANY, "On my break, I'd love a {GAME}!\nLet's play!"},
    {QUOTE_VOICE_STAFF, H, S_FIRST, "Welcome! A {GAME} with a\ncustomer? Why not!"},
    {QUOTE_VOICE_STAFF, H, S_YOU_AHEAD, "You're a regular winner here!\nI'll do my best!"},
    {QUOTE_VOICE_STAFF, H, S_THEY_AHEAD, "Welcome back! Ready for\nanother round?"},
    {QUOTE_VOICE_STAFF, W, S_ANY, "Thank you for playing!\nPlease come again!"},
    {QUOTE_VOICE_STAFF, L, S_ANY, "What a {GAME}! Thank you, and\nplease come again!"},
    {QUOTE_VOICE_STAFF, L, S_YOU_AHEAD, "You win again! You're our\nbest customer!"},
};

/* The special players, as themselves (by the name the game shows) */
struct SpecialLine
{
    const char *who;
    unsigned char when, situation;
    const char *text;
};
static const struct SpecialLine sSpecial[] = {
    {"ROXANNE", H, S_FIRST, "A new student at my table!\nShow me what you've studied."},
    {"ROXANNE", H, S_YOU_AHEAD, "I've reviewed every {GAME} we've\nplayed. Today, I pass the test!"},
    {"ROXANNE", H, S_THEY_AHEAD, "Back for another lesson?\nI'll teach it well!"},
    {"ROXANNE", W, S_ANY, "Class dismissed! You learn fast,\nthough. Study and return!"},
    {"ROXANNE", L, S_ANY, "Remarkable! You've earned top\nmarks today."},
    {"ROXANNE", L, S_YOU_AHEAD, "You've outscored me again!\nI'll be taking notes on you."},
    {"BRAWLY", H, S_FIRST, "A new wave to ride! Let's\nsee your big moves!"},
    {"BRAWLY", H, S_YOU_AHEAD, "You've wiped me out before!\nThis time I'm riding high!"},
    {"BRAWLY", H, S_THEY_AHEAD, "Paddling back out for more?\nRespect! Let's go!"},
    {"BRAWLY", W, S_ANY, "Cowabunga! What a ride!\nYou almost knocked me off!"},
    {"BRAWLY", L, S_ANY, "Wiped out! That was a monster\nwave you threw at me!"},
    {"WATTSON", H, S_FIRST, "Wahahah! A fresh challenger!\nLet's spark things up!"},
    {"WATTSON", H, S_YOU_AHEAD, "Wahahah! You've shocked me\nbefore! Not today!"},
    {"WATTSON", H, S_THEY_AHEAD, "Wahahah! Back for a jolt?\nI admire that!"},
    {"WATTSON", W, S_ANY, "Wahahahah! Fully charged!\nThat was electrifying fun!"},
    {"WATTSON", L, S_ANY, "Wahahah! I've been shorted out!\nWhat a powerful player!"},
    {"FLANNERY", H, S_FIRST, "Okay! Let's do this! I'm not\nnervous at all! Really!"},
    {"FLANNERY", H, S_YOU_AHEAD, "You've beaten me before, but\nmy fire's hotter now!"},
    {"FLANNERY", H, S_THEY_AHEAD, "Back again? My cards are still\nwarm! Let's go!"},
    {"FLANNERY", W, S_ANY, "I won! I mean... of course I\nwon! That was a great {GAME}!"},
    {"FLANNERY", L, S_ANY, "Ooh, you put out my flame!\nI'll burn even hotter next time!"},
    {"NORMAN", H, S_FIRST, "A {GAME} with you... I didn't\nexpect this. Let's begin."},
    {"NORMAN", H, S_YOU_AHEAD, "You've grown so much. Today,\nyour father plays seriously."},
    {"NORMAN", H, S_THEY_AHEAD, "Back to challenge your old man?\nGood. Show me your growth."},
    {"NORMAN", W, S_ANY, "Well played. You're closer than\nyou think. Keep going."},
    {"NORMAN", L, S_ANY, "You've surpassed me here, too.\nI couldn't be prouder."},
    {"WINONA", H, S_FIRST, "Let us glide into a graceful\n{GAME} together."},
    {"WINONA", H, S_YOU_AHEAD, "You've soared past me before.\nToday I fly higher."},
    {"WINONA", H, S_THEY_AHEAD, "You return to the skies!\nLet's take flight."},
    {"WINONA", W, S_ANY, "The winds favored me today.\nYou flew beautifully."},
    {"WINONA", L, S_ANY, "You soared above me! A truly\nelegant victory."},
    {"TATE & LIZA", H, S_FIRST, "TATE: A new challenger...\nLIZA: ...let's read their cards!"},
    {"TATE & LIZA", H, S_YOU_AHEAD, "TATE: You beat us before...\nLIZA: ...but we've practiced!"},
    {"TATE & LIZA", H, S_THEY_AHEAD, "TATE: You're back...\nLIZA: ...for another try!"},
    {"TATE & LIZA", W, S_ANY, "TATE: We win! LIZA: That was\nfun! TATE & LIZA: Again soon!"},
    {"TATE & LIZA", L, S_ANY, "TATE: You beat us both!\nLIZA: We didn't see that coming!"},
    {"JUAN", H, S_FIRST, "Ah, a new partner for this\nelegant dance of {CARDS}."},
    {"JUAN", H, S_YOU_AHEAD, "Your art has bested mine before.\nAllow me a new piece."},
    {"JUAN", H, S_THEY_AHEAD, "You return to my stage!\nLet us perform once more."},
    {"JUAN", W, S_ANY, "A masterpiece, if I may say.\nYour play was lovely, too."},
    {"JUAN", L, S_ANY, "Magnificent! Your art outshines\nmine today. Bravo!"},
    {"SIDNEY", H, S_FIRST, "Heh, fresh face! Let's make\nthis one a good show!"},
    {"SIDNEY", H, S_YOU_AHEAD, "You've had my number, huh?\nHeh. Not today!"},
    {"SIDNEY", W, S_ANY, "Heh! That's how I play.\nYou gave me a real run, though."},
    {"SIDNEY", L, S_ANY, "Heh, you got me! I like your\nstyle. Let's do it again!"},
    {"PHOEBE", H, S_FIRST, "Ahaha! My spirits are curious\nabout you. Let's play!"},
    {"PHOEBE", H, S_YOU_AHEAD, "Ahaha! You've spooked me before!\nMy turn now!"},
    {"PHOEBE", W, S_ANY, "Ahaha! The spirits smiled on me!\nThat was so much fun!"},
    {"PHOEBE", L, S_ANY, "Ahaha! You beat me fair and\nsquare! The spirits approve!"},
    {"GLACIA", H, S_FIRST, "Let us see if your {CARDS}\nare as warm as your heart."},
    {"GLACIA", H, S_YOU_AHEAD, "You've melted my defenses\nbefore. Not this time."},
    {"GLACIA", W, S_ANY, "Cool and calm wins the day.\nYou were a worthy opponent."},
    {"GLACIA", L, S_ANY, "Your passion melted my ice.\nWonderfully played."},
    {"DRAKE", H, S_FIRST, "Show me you have what it takes\nto win! Come!"},
    {"DRAKE", H, S_YOU_AHEAD, "You've won before. But a\ndragon never yields!"},
    {"DRAKE", W, S_ANY, "Hah! My will held strong!\nYours was close, though!"},
    {"DRAKE", L, S_ANY, "Superb! You have the will of\na true champion!"},
    {"WALLACE", H, S_FIRST, "Let us make this a {GAME} as\nbeautiful as the sea!"},
    {"WALLACE", H, S_YOU_AHEAD, "You've dazzled me before.\nAllow me to return the favor!"},
    {"WALLACE", W, S_ANY, "How elegant! Thank you for\nsuch a beautiful {GAME}."},
    {"WALLACE", L, S_ANY, "Breathtaking! Your play was\nthe true masterpiece today."},
    {"STEVEN", H, S_FIRST, "Like stones, every {DECK} has\nits shine. Let's see yours."},
    {"STEVEN", H, S_YOU_AHEAD, "You've polished your play since\nlast time, I bet. Me too!"},
    {"STEVEN", H, S_THEY_AHEAD, "Back again? I always enjoy\nour {GAMES}. Let's go!"},
    {"STEVEN", W, S_ANY, "A good {GAME}! You're a rare\nfind, you know."},
    {"STEVEN", L, S_ANY, "Splendid! You're the rarest\ngem I've found yet."},
    {"RIVAL", H, S_FIRST, "Our first card {GAME}? Bring it!\nNo holding back!"},
    {"RIVAL", H, S_YOU_AHEAD, "You've won more... for now!\nI've got a new plan!"},
    {"RIVAL", H, S_THEY_AHEAD, "Ready to try again? I'll\nkeep my lead!"},
    {"RIVAL", H, S_EVEN, "We're dead even! This one\ncounts double! Kidding!"},
    {"RIVAL", W, S_ANY, "Yes! I win! That was such a\ngood {GAME}! Again soon?"},
    {"RIVAL", L, S_ANY, "Aw, you got me! You're getting\nreally good at this!"},
    {"WALLY", H, S_FIRST, "I-I've been practicing!\nPlease play with me!"},
    {"WALLY", H, S_YOU_AHEAD, "You're so strong... but I want\nto get stronger too!"},
    {"WALLY", H, S_THEY_AHEAD, "I won last time, didn't I?\nI'll try my hardest again!"},
    {"WALLY", W, S_ANY, "I... I won! Thank you for\nplaying with me!"},
    {"WALLY", L, S_ANY, "You're amazing! Playing you\nmakes me want to grow!"},
    {"MAXIE", H, S_FIRST, "A {GAME} of strategy. Very well.\nShow me your thinking."},
    {"MAXIE", H, S_YOU_AHEAD, "You have outplanned me before.\nI've revised my plans."},
    {"MAXIE", W, S_ANY, "A sound plan, well executed.\nYours was not bad either."},
    {"MAXIE", L, S_ANY, "Fascinating... You saw further\nahead than I did."},
    {"ARCHIE", H, S_FIRST, "Bwahahaha! A new crewmate at\nthe table! Let's go!"},
    {"ARCHIE", H, S_YOU_AHEAD, "Bwahaha! You've beached me\nbefore! Here comes the tide!"},
    {"ARCHIE", W, S_ANY, "Bwahahaha! The sea wins!\nYou've got guts, though!"},
    {"ARCHIE", L, S_ANY, "Bwahaha! You out-sailed me!\nWhat a {GAME}!"},
    {"ANABEL", H, S_FIRST, "I'd like to see how you think.\nLet's begin."},
    {"ANABEL", H, S_YOU_AHEAD, "You've read me well before.\nLet's see if you still can."},
    {"ANABEL", W, S_ANY, "...I read you well this time.\nThank you. That was fun."},
    {"ANABEL", L, S_ANY, "...Your thoughts were clearer\nthan mine. Well done."},
    {"TUCKER", H, S_FIRST, "Ladies and gentlemen, a new\nchallenger! Let's put on a show!"},
    {"TUCKER", H, S_YOU_AHEAD, "The crowd loves you! Let's see\nif they'll cheer for me today!"},
    {"TUCKER", W, S_ANY, "And the crowd goes wild!\nWhat a show you put on!"},
    {"TUCKER", L, S_ANY, "Encore! Encore! You stole the\nshow! Bravo!"},
    {"SPENSER", H, S_FIRST, "Heh! Let's see if your roots\ngo deep, kid."},
    {"SPENSER", H, S_YOU_AHEAD, "You've beaten me before, kid.\nThe old man's not done!"},
    {"SPENSER", W, S_ANY, "Heh! Experience counts.\nYou've got a good head, kid."},
    {"SPENSER", L, S_ANY, "Hah! You beat me fair!\nYou're quite the player, kid."},
    {"GRETA", H, S_FIRST, "All right! Let's get fired up!\nGive me everything!"},
    {"GRETA", H, S_YOU_AHEAD, "You beat me before! I'm all\nfired up for a rematch!"},
    {"GRETA", W, S_ANY, "Yeah! What a fight! You were\nawesome, too!"},
    {"GRETA", L, S_ANY, "Wow! You've got guts!\nThat was a blast!"},
    {"NOLAND", H, S_FIRST, "A {GAME}? Sure! Let's see\nwhat you know!"},
    {"NOLAND", H, S_YOU_AHEAD, "You've got my number! I've\ntuned up my {DECK} since!"},
    {"NOLAND", W, S_ANY, "Ha! Knowledge pays off!\nGood {GAME}, though!"},
    {"NOLAND", L, S_ANY, "Ha ha! You know your stuff!\nI learned a thing or two."},
    {"LUCY", H, S_FIRST, "...A new opponent. Fine."},
    {"LUCY", H, S_YOU_AHEAD, "...You won before. Not again."},
    {"LUCY", W, S_ANY, "...I won. You were... good."},
    {"LUCY", L, S_ANY, "...You beat me. ...Good {GAME}."},
    {"MR. STONE", H, S_FIRST, "Ah, a new player! DEVON loves\nnew talent. Let's play!"},
    {"MR. STONE", W, S_ANY, "Ho ho! A fine {GAME}! You're\nwelcome at DEVON anytime!"},
    {"MR. STONE", L, S_ANY, "Ho ho! Splendid! You remind\nme of my son!"},
    {"MR. BRINEY", H, S_FIRST, "Ahoy! A new card mate!\nLet's play, shall we?"},
    {"MR. BRINEY", W, S_ANY, "Ahoy! The old sailor still has\nit! Good {GAME}, friend!"},
    {"MR. BRINEY", L, S_ANY, "Ahoy! You beat me fair and\nsquare! PEEKO, did you see?"},
    {"SCOTT", H, S_FIRST, "I'm always scouting talent.\nLet's see yours!"},
    {"SCOTT", W, S_ANY, "Nice! I'll keep an eye on\nyour progress!"},
    {"SCOTT", L, S_ANY, "Wow! I knew you had talent!\nRemarkable!"},
    {"PROF. BIRCH", H, S_FIRST, "Field research can wait!\nLet's play!"},
    {"PROF. BIRCH", W, S_ANY, "Ha ha! The professor wins!\nGreat {GAME}, though!"},
    {"PROF. BIRCH", L, S_ANY, "Ha ha! You beat me! You're\nfull of surprises!"},
    {"MOM", H, S_FIRST, "Oh, a {GAME} with my kid!\nDon't go easy on me!"},
    {"MOM", W, S_ANY, "Hehe, Mom still has it!\nYou played so well, dear."},
    {"MOM", L, S_ANY, "Oh my! You've gotten so good!\nI'm so proud of you!"},
    {"CARD QUEEN", H, S_FIRST, "A new challenger for the\nQUEEN! Let's play by my rules!"},
    {"CARD QUEEN", H, S_YOU_AHEAD, "You've taken my crown before.\nNot so easily today!"},
    {"CARD QUEEN", H, S_THEY_AHEAD, "Back for my court? I admire\nyour spirit! Let's play!"},
    {"CARD QUEEN", W, S_ANY, "The QUEEN keeps her crown!\nA fine game, all the same."},
    {"CARD QUEEN", L, S_ANY, "Oh my! You've played like\nroyalty! Well done!"},
};

static unsigned Situation(unsigned when, unsigned wins, unsigned losses)
{
    unsigned played = wins + losses;
    if (when == QUOTE_HELLO ? !played : played == 1)
        return S_FIRST;
    if (wins > losses)
        return S_YOU_AHEAD;
    if (losses > wins)
        return S_THEY_AHEAD;
    return S_EVEN;
}
/* {GAME} and the like, for the game */
static void Fill(char *out, size_t size, const char *text, unsigned game)
{
    size_t n = 0;
    while (*text && n + 1 < size)
    {
        const char *put = NULL;
        size_t skip = 0;
        if (!strncmp(text, "{GAMES}", 7))
            put = game == QUOTE_GAME_TT ? "games" : "duels", skip = 7;
        else if (!strncmp(text, "{GAME}", 6))
            put = game == QUOTE_GAME_TT ? "game" : "duel", skip = 6;
        else if (!strncmp(text, "{DECK}", 6))
            put = game == QUOTE_GAME_TT ? "hand" : "deck", skip = 6;
        else if (!strncmp(text, "{CARDS}", 7))
            put = "cards", skip = 7;
        if (put)
        {
            while (*put && n + 1 < size)
                out[n++] = *put++;
            text += skip;
            continue;
        }
        out[n++] = *text++;
    }
    out[n] = 0;
}
int CardQuote(char *out, size_t size, const char *special, unsigned voice, unsigned game, unsigned when, unsigned wins,
              unsigned losses, uint32_t pick)
{
    unsigned situation = Situation(when, wins, losses), i, exact = 0, any = 0, k;
    const char *text = NULL;
    if (!size)
        return 0;
    out[0] = 0;
    /* (the same pick never gives the same line twice in a row for a record that changed) */
    pick = pick * 2654435761u + wins * 40503u + losses * 9973u;
    if (special)
    {
        for (i = 0; i < sizeof(sSpecial) / sizeof(sSpecial[0]); i++)
            if (!strcmp(sSpecial[i].who, special) && sSpecial[i].when == when)
                exact += sSpecial[i].situation == situation, any += sSpecial[i].situation == S_ANY;
        if (exact || any)
        {
            unsigned want = exact ? situation : S_ANY;
            k = (pick >> 7) % (exact ? exact : any);
            for (i = 0; i < sizeof(sSpecial) / sizeof(sSpecial[0]); i++)
                if (!strcmp(sSpecial[i].who, special) && sSpecial[i].when == when && sSpecial[i].situation == want &&
                    !k--)
                    text = sSpecial[i].text;
        }
        if (!text)
            voice = QUOTE_VOICE_REFINED;
    }
    if (!text)
    {
        if (voice >= QUOTE_VOICE_COUNT)
            return 0;
        exact = any = 0;
        for (i = 0; i < sizeof(sLines) / sizeof(sLines[0]); i++)
            if (sLines[i].voice == voice && sLines[i].when == when)
                exact += sLines[i].situation == situation, any += sLines[i].situation == S_ANY;
        if (!exact && !any)
            return 0;
        {
            /* the record's own line about half the time (always for a first meeting), else any of theirs */
            unsigned want = exact && (situation == S_FIRST || (pick >> 3) % 2 || !any) ? situation : S_ANY;
            k = (pick >> 9) % (want == situation ? exact : any);
            for (i = 0; i < sizeof(sLines) / sizeof(sLines[0]); i++)
                if (sLines[i].voice == voice && sLines[i].when == when && sLines[i].situation == want && !k--)
                    text = sLines[i].text;
        }
    }
    if (!text)
        return 0;
    Fill(out, size, text, game);
    return 1;
}
void CardQuote_OneLine(char *text)
{
    for (; *text; text++)
        if (*text == '\n' || *text == '\f')
            *text = ' ';
}
#ifdef CARD_QUOTES_TEST
/* tools/tcg/test_quotes.c: every line fits */
unsigned CardQuote_Count(void) { return sizeof(sLines) / sizeof(sLines[0]) + sizeof(sSpecial) / sizeof(sSpecial[0]); }
const char *CardQuote_Raw(unsigned i)
{
    return i < sizeof(sLines) / sizeof(sLines[0]) ? sLines[i].text : sSpecial[i - sizeof(sLines) / sizeof(sLines[0])].text;
}
#endif
#undef H
#undef W
#undef L
