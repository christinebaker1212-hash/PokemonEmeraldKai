// Moemon Co-op: real keyboard typing for text entry screens
#ifdef PLATFORM_SDL2
#include <string.h>
#include <SDL2/SDL.h>
#include "platform_text.h"

#define QUEUE_SIZE 128

static int sOn;
static char sQueue[QUEUE_SIZE];
static int sHead, sCount;

static void Push(char c)
{
    if (sCount < QUEUE_SIZE)
    {
        sQueue[(sHead + sCount) % QUEUE_SIZE] = c;
        sCount++;
    }
}

void Platform_QueueTypedText(const char *text)
{
    for (; *text; text++)
    {
        unsigned char c = (unsigned char)*text;
        if (c == '\r')
            continue;
        if (c == '\t')
            c = ' ';
        if ((c >= 32 && c < 127) || c == TYPED_BACKSPACE || c == TYPED_ENTER)
            Push((char)c);
    }
}

void Platform_SetTextInput(int on)
{
    if (on == sOn)
        return;
    sOn = on;
    sHead = sCount = 0;
    if (on)
        SDL_StartTextInput();
    else
        SDL_StopTextInput();
}

int Platform_IsTextInputOn(void)
{
    return sOn;
}

int Platform_PopTypedChar(void)
{
    char c;
    if (sCount == 0)
        return 0;
    c = sQueue[sHead];
    sHead = (sHead + 1) % QUEUE_SIZE;
    sCount--;
    return (unsigned char)c;
}

// Called from the SDL event loop
void Platform_TextInputEvent(const char *utf8)
{
    if (sOn)
        Platform_QueueTypedText(utf8); // non-ASCII bytes are dropped
}

// Returns 1 when the key was used for typing and must not press a GBA button
int Platform_TextInputKeyDown(int sym, int mod)
{
    if (!sOn)
        return 0;
    if ((mod & KMOD_CTRL) && sym == SDLK_v)
    {
        char *clip = SDL_GetClipboardText();
        if (clip)
        {
            char *p;
            for (p = clip; *p; p++) // one line only
                if (*p == '\n' || *p == '\r')
                    *p = 0;
            Platform_QueueTypedText(clip);
            SDL_free(clip);
        }
        return 1;
    }
    if (mod & (KMOD_CTRL | KMOD_ALT | KMOD_GUI))
        return 0;
    if (sym == SDLK_BACKSPACE)
    {
        Push(TYPED_BACKSPACE);
        return 1;
    }
    if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER)
    {
        Push(TYPED_ENTER);
        return 1;
    }
    // Printable keys arrive as SDL_TEXTINPUT; keep them from pressing A/B/L/R
    return sym >= 32 && sym < 127;
}

#endif // PLATFORM_SDL2

// Moemon Co-op: the last letter keys pressed outside text entry
static char sLetters[32];
static int sLetterHead, sLetterCount;

// Typed anywhere outside text entry: "speed" / "slow" (AllowSmallTweaks = 1)
static void CheckTypedWords(char c)
{
    static char sTail[8];
    void Platform_SetSpeedUp(int on);
    int n = (int)strlen(sTail);
    if (n == (int)sizeof(sTail) - 1)
    {
        memmove(sTail, sTail + 1, n);
        n--;
    }
    sTail[n++] = c;
    sTail[n] = 0;
    if (n >= 5 && memcmp(sTail + n - 5, "speed", 5) == 0)
    {
        Platform_SetSpeedUp(1);
        sTail[0] = 0;
    }
    else if (n >= 4 && memcmp(sTail + n - 4, "slow", 4) == 0)
    {
        Platform_SetSpeedUp(0);
        sTail[0] = 0;
    }
}

void Platform_PushLetter(char c)
{
    CheckTypedWords(c);
    if (sLetterCount == (int)sizeof(sLetters))
    {
        sLetterHead = (sLetterHead + 1) % (int)sizeof(sLetters);
        sLetterCount--;
    }
    sLetters[(sLetterHead + sLetterCount) % (int)sizeof(sLetters)] = c;
    sLetterCount++;
}

int Platform_PopLetter(void)
{
    char c;
    if (sLetterCount == 0)
        return 0;
    c = sLetters[sLetterHead];
    sLetterHead = (sLetterHead + 1) % (int)sizeof(sLetters);
    sLetterCount--;
    return (unsigned char)c;
}

// Moemon Co-op: put text (an invite code) on the system clipboard
void Platform_SetClipboardText(const char *text)
{
    SDL_SetClipboardText(text);
}
