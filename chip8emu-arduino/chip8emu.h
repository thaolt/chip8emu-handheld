#ifndef CHIP8EMU_H_
#define CHIP8EMU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define C8ERR_OK 0

typedef struct chip8emu chip8emu;


struct chip8emu
{
    uint8_t   memory[4096];
    uint8_t   gfx[64 * 32];
    uint8_t   V[16];        /* registers from V0 .. VF */

    uint16_t  I;            /* index register */
    uint16_t  pc;           /* program counter */
    uint16_t  opcode;

    uint8_t   delay_timer;
    uint8_t   sound_timer;

    uint16_t  stack[16];
    uint16_t  sp;           /* stack pointer */

    /* opcode handling functions, can be overrided */
    int  (*opcode_handlers[0x10])(chip8emu *);
    
    /* random number generator */
    int (*rand)(void);
    
    /* API callbacks */
    void (*draw)(chip8emu *);
    bool (*keystate)(chip8emu *, uint8_t);
    void (*beep)(chip8emu *);
    void (*log)(chip8emu *, int log_level, const char *file, int line, const char* message);
};

chip8emu* chip8emu_new(void);
void chip8emu_free(chip8emu*);
int chip8emu_load_code(chip8emu *emu, uint8_t* code, long code_size);
int chip8emu_load_rom(chip8emu* emu, const char* filename);
void chip8emu_exec_cycle(chip8emu *emu);
void chip8emu_timer_tick(chip8emu *emu);

#ifdef __cplusplus
}
#endif

#endif /* CHIP8EMU_H_ */
