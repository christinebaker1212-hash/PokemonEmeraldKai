// Moemon Co-op: typing on a real keyboard (naming screen, server name/password)
#ifndef GUARD_PLATFORM_TEXT_H
#define GUARD_PLATFORM_TEXT_H

#define TYPED_BACKSPACE '\b'
#define TYPED_ENTER     '\n'

// While on, letter/number keys type text instead of pressing GBA buttons.
// Arrow keys keep working. Enter confirms, Backspace deletes, Ctrl+V pastes.
void Platform_SetTextInput(int on);
int Platform_IsTextInputOn(void);
// Next typed ASCII character (or TYPED_*), 0 when none
int Platform_PopTypedChar(void);
// Test harness: queue text as if typed
void Platform_QueueTypedText(const char *text);

#endif // GUARD_PLATFORM_TEXT_H
