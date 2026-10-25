/* The game's 8-bit samples and programmable waves (gen_soundfont.py -> sf_pcm.bin). */
    .section .rodata
    .balign 16
    .globl sf_pcm
sf_pcm:
    .incbin SF_PCM_BIN
    .balign 16
#if defined(__linux__) && defined(__ELF__)
    .section .note.GNU-stack,"",%progbits
#endif
