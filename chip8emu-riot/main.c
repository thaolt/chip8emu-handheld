#include <stdio.h>
#include "chip8emu.h"

int main(void)
{
    chip8emu *emudev = chip8emu_new();
    puts("Hello World!");
    chip8emu_start(emudev);
    chip8emu_free(emudev);
    return 0;
}

