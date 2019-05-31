
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "chip8emu.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Logging */
enum { C8E_LOG_DEBUG, C8E_LOG_INFO, C8E_LOG_WARN, C8E_LOG_ERR, C8E_LOG_FATAL };
#define _chip8emu_log_debug(emu, ...)   _chip8emu_log_forward(emu, C8E_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define _chip8emu_log_info(emu, ...)    _chip8emu_log_forward(emu, C8E_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define _chip8emu_log_warn(emu, ...)    _chip8emu_log_forward(emu, C8E_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define _chip8emu_log_error(emu, ...)   _chip8emu_log_forward(emu, C8E_LOG_ERR, __FILE__, __LINE__, __VA_ARGS__)
#define _chip8emu_log_fatal(emu, ...)   _chip8emu_log_forward(emu, C8E_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

static void _chip8emu_log_forward(chip8emu* emu, int level, const char *file, int line, const char *fmt, ...)
{
    char message[255] = {0};

    va_list args;

    va_start(args, fmt);
    vsnprintf(message, 255, fmt, args);
    va_end(args);

    emu->log(emu, level, file, line, message);
}

static void _dummy_logger(chip8emu *emu, int log_level, const char *file, int line, const char* message) {
    (void)emu; (void)log_level; (void)file; (void)line; (void)message;
}

static int _default_rand(void) {
    return rand();
}

/* opcode handling prototypes */
static int _chip8emu_opcode_handler_0(chip8emu* emu);
static int _chip8emu_opcode_handler_1(chip8emu* emu);
static int _chip8emu_opcode_handler_2(chip8emu* emu);
static int _chip8emu_opcode_handler_3(chip8emu* emu);
static int _chip8emu_opcode_handler_4(chip8emu* emu);
static int _chip8emu_opcode_handler_5(chip8emu* emu);
static int _chip8emu_opcode_handler_6(chip8emu* emu);
static int _chip8emu_opcode_handler_7(chip8emu* emu);
static int _chip8emu_opcode_handler_8(chip8emu* emu);
static int _chip8emu_opcode_handler_9(chip8emu* emu);
static int _chip8emu_opcode_handler_A(chip8emu* emu);
static int _chip8emu_opcode_handler_B(chip8emu* emu);
static int _chip8emu_opcode_handler_C(chip8emu* emu);
static int _chip8emu_opcode_handler_D(chip8emu* emu);
static int _chip8emu_opcode_handler_E(chip8emu* emu);
static int _chip8emu_opcode_handler_F(chip8emu* emu);

static uint8_t _chip8emu_timer_delay(chip8emu* emu);
static void _chip8emu_timer_delay_set(chip8emu* emu, uint8_t val);

static void _chip8emu_timer_sound_set(chip8emu* emu, uint8_t val);

/* Default font set */
static uint8_t chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
  0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
  0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
  0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
  0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
  0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
  0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
  0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
  0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
  0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
  0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
  0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
  0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
  0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
  0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
  0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};


/* chip8emu initialization */
chip8emu *chip8emu_new(void)
{
    chip8emu* emu = malloc(sizeof (chip8emu));

    emu->pc     = 0x200;  /* Program counter starts at 0x200 */
    emu->opcode = 0;      /* Reset current opcode */
    emu->I      = 0;      /* Reset index register */
    emu->sp     = 0;      /* Reset stack pointer */

    memset(emu->gfx, 0, 64 * 32);      /* Clear display */
    memset(emu->stack, 0, 16 * sizeof(uint16_t));         /* Clear stack */
    memset(emu->V, 0, 16);             /* Clear registers V0-VF */
    memset(emu->memory, 0, 4096);      /* Clear memory */

    /* Load fontset */
    for(int i = 0; i < 80; ++i)
        emu->memory[i] = chip8_fontset[i];

    /* Reset timers */
    emu->delay_timer = 0;
    emu->sound_timer = 0;

    emu->draw = 0;
    emu->keystate = 0;
    emu->beep = 0;
    emu->rand = &_default_rand;
    emu->log = &_dummy_logger;

    emu->opcode_handlers[0x0] = &_chip8emu_opcode_handler_0;
    emu->opcode_handlers[0x1] = &_chip8emu_opcode_handler_1;
    emu->opcode_handlers[0x2] = &_chip8emu_opcode_handler_2;
    emu->opcode_handlers[0x3] = &_chip8emu_opcode_handler_3;
    emu->opcode_handlers[0x4] = &_chip8emu_opcode_handler_4;
    emu->opcode_handlers[0x5] = &_chip8emu_opcode_handler_5;
    emu->opcode_handlers[0x6] = &_chip8emu_opcode_handler_6;
    emu->opcode_handlers[0x7] = &_chip8emu_opcode_handler_7;
    emu->opcode_handlers[0x8] = &_chip8emu_opcode_handler_8;
    emu->opcode_handlers[0x9] = &_chip8emu_opcode_handler_9;
    emu->opcode_handlers[0xA] = &_chip8emu_opcode_handler_A;
    emu->opcode_handlers[0xB] = &_chip8emu_opcode_handler_B;
    emu->opcode_handlers[0xC] = &_chip8emu_opcode_handler_C;
    emu->opcode_handlers[0xD] = &_chip8emu_opcode_handler_D;
    emu->opcode_handlers[0xE] = &_chip8emu_opcode_handler_E;
    emu->opcode_handlers[0xF] = &_chip8emu_opcode_handler_F;

    return emu;
}

void chip8emu_free(chip8emu *emu)
{
    free(emu);
}

/* ******************** Opcode handling implementation ******************** */
int _chip8emu_opcode_handler_0(chip8emu* emu) {
    switch (emu->opcode) {
    case 0x00E0: /* clear screen */
        memset(emu->gfx, 0, 64*32);
        emu->pc += 2;
        emu->draw(emu);
        break;

    case 0x00EE: /* subroutine return */
        emu->pc = emu->stack[--emu->sp & 0xF] + 2;
        break;

    default: /* 0NNN: call program at NNN address */
        _chip8emu_log_error(emu, "OpCode 0NNN is not implemented");
        break;
    }
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_1(chip8emu* emu) {
     /* 1NNN: absolute jump */
    emu->pc = emu->opcode & 0xFFF;
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_2(chip8emu* emu) {
    /* 2NNN: call subroutine */
    emu->stack[emu->sp] = emu->pc;
    ++emu->sp;
    emu->pc = emu->opcode & 0x0FFF;
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_3(chip8emu* emu) {
    /* 3XNN: Skips the next instruction if VX equals NN */
    if (emu->V[(emu->opcode & 0x0F00) >> 8] == (emu->opcode & 0x00FF)) {
        emu->pc += 4;
    } else {
        emu->pc += 2;
    }
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_4(chip8emu* emu) {
    /* 4XNN: Skips the next instruction if VX doesn't equal NN */
    if (emu->V[(emu->opcode & 0x0F00) >> 8] != (emu->opcode & 0x00FF)) {
        emu->pc += 4;
    } else {
        emu->pc += 2;
    }
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_5(chip8emu* emu) {
    /* 5XY0: Skips the next instruction if VX equals VY */
    if (emu->V[(emu->opcode & 0x0F00) >> 8] == emu->V[(emu->opcode & 0x00F0) >> 4]) {
        emu->pc += 4;
    } else {
        emu->pc += 2;
    }
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_6(chip8emu* emu) {
    /* 6XNN: Sets VX to NN */
    emu->V[(emu->opcode & 0x0F00) >> 8] = (emu->opcode & 0x00FF);
    emu->pc += 2;
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_7(chip8emu* emu) {
    /* 7XNN: Adds NN to VX */
    emu->V[(emu->opcode & 0x0F00) >> 8] += (emu->opcode & 0x00FF);
    emu->pc += 2;
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_8(chip8emu* emu) {
    switch (emu->opcode & 0x000F) {
    case 0x0000: /* 8XY0: Vx = Vy  */
        emu->V[(emu->opcode & 0x0F00) >> 8] = emu->V[(emu->opcode & 0x00F0) >> 4];
        emu->pc += 2;
        break;
    case 0x0001: /* 8XY1: Vx = Vx | Vy */
        emu->V[(emu->opcode & 0x0F00) >> 8] |= emu->V[(emu->opcode & 0x00F0) >> 4];
        emu->pc += 2;
        break;
    case 0x0002: /* 8XY2: Vx = Vx & Vy*/
        emu->V[(emu->opcode & 0x0F00) >> 8] &= emu->V[(emu->opcode & 0x00F0) >> 4];
        emu->pc += 2;
        break;
    case 0x0003: /* 8XY3: Vx = Vx XOR Vy */
        emu->V[(emu->opcode & 0x0F00) >> 8] ^= emu->V[(emu->opcode & 0x00F0) >> 4];
        emu->pc += 2;
        break;
    case 0x0004: /* 8XY4: Vx += Vy; Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't */
        if(emu->V[(emu->opcode & 0x00F0) >> 4] > (0xFF - emu->V[(emu->opcode & 0x0F00) >> 8]))
            emu->V[0xF] = 1; /* carry over */
        else
            emu->V[0xF] = 0;
        emu->V[(emu->opcode & 0x0F00) >> 8] += emu->V[(emu->opcode & 0x00F0) >> 4];
        emu->pc += 2;
        break;
    case 0x0005: /* 8XY5: Vx -= Vy; VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't */
        if (emu->V[(emu->opcode & 0x00F0) >> 4] > emu->V[(emu->opcode & 0x0F00) >> 8]) {
            emu->V[0xF] = 0;
        } else {
            emu->V[0xF] = 1;
        }
        emu->V[(emu->opcode & 0x0F00) >> 8] -= emu->V[(emu->opcode & 0x00F0) >> 4];
        emu->pc += 2;
        break;
    case 0x0006: /* 8XY6: Vx>>=1; Stores the least significant bit of VX in VF and then shifts VX to the right by 1 */
        emu->V[0xF] = emu->V[(emu->opcode & 0x0F00) >> 8] & 0x1;
        emu->V[(emu->opcode & 0x0F00) >> 8] >>= 1;
        emu->pc += 2;
        break;
    case 0x0007: /* 8XY7: Vx=Vy-Vx; Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't */
        if (emu->V[(emu->opcode & 0x0F00) >> 8] > (int)emu->V[(emu->opcode & 0x00F0) >> 4]) {
            emu->V[0xF] = 0;
        } else {
            emu->V[0xF] = 1;
        }
        emu->V[(emu->opcode & 0x0F00) >> 8] = emu->V[(emu->opcode & 0x00F0) >> 4] - emu->V[(emu->opcode & 0x0F00) >> 8];
        emu->pc += 2;
        break;
    case 0x000E: /* 8XYE: Vx<<=1; Stores the most significant bit of VX in VF and then shifts VX to the left by 1 */
        emu->V[0xF] = emu->V[(emu->opcode & 0x0F00) >> 8] >> 7;
        emu->V[(emu->opcode & 0x0F00) >> 8] <<= 1;
        emu->pc += 2;
        break;
    }
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_9(chip8emu* emu) {
    /* 9XY0: Skips the next instruction if VX doesn't equal VY */
    if(emu->V[(emu->opcode & 0x0F00) >> 8] != emu->V[(emu->opcode & 0x00F0) >> 4]) {
        emu->pc += 4;
    } else {
        emu->pc += 2;
    }
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_A(chip8emu* emu) {
    /* ANNN: Sets I to the address NNN */
    emu->I = emu->opcode & 0x0FFF;
    emu->pc += 2;
    return C8ERR_OK;
}
int _chip8emu_opcode_handler_B(chip8emu* emu) {
    /* BNNN: Jumps to the address NNN plus V0 */
    emu->pc = (emu->opcode & 0x0FFF) + emu->V[0];
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_C(chip8emu* emu) {
    /* CXNN: Vx=rand() & NN */
    emu->V[(emu->opcode & 0x0F00) >> 8] = (emu->rand() % (0xFF + 1)) & (emu->opcode & 0x00FF);
    emu->pc += 2;
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_D(chip8emu* emu) {
    /* DXYN: draw(Vx,Vy,N); draw at X,Y width 8, height N sprite from I register */
    uint8_t xo = emu->V[(emu->opcode & 0x0F00) >> 8]; /* x origin */
    uint8_t yo = emu->V[(emu->opcode & 0x00F0) >> 4];
    uint8_t height = emu->opcode & 0x000F;
    uint8_t sprite[0x10] = {0};

    memcpy(sprite, emu->memory + (emu->I * sizeof (uint8_t)), height);

    emu->V[0xF] = 0;
    for (uint8_t y = 0; y < height; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            int dx = (xo + x) % 64; /* display x or dest x*/
            int dy = (yo + y) % 32;
            if ((sprite[y] & (0x80 >> x)) != 0) { /* 0x80 -> 10000000b */
                if (!emu->V[0xF] && emu->gfx[(dx + (dy * 64))])
                    emu->V[0xF] = 1;
                emu->gfx[dx + (dy * 64)] ^= 1;
            }
        }
    }

    emu->draw(emu);
    emu->pc += 2;
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_E(chip8emu* emu) {
    switch (emu->opcode & 0x00FF) {
    case 0x009E: /* EX9E: Skips the next instruction if the key stored in VX is pressed */
        if(emu->keystate(emu, emu->V[(emu->opcode & 0x0F00) >> 8])) {
            emu->pc += 4;
        } else {
            emu->pc += 2;
        }
        break;
    case 0x00A1: /* EXA1: Skips the next instruction if the key stored in VX isn't pressed */
        if(!emu->keystate(emu, emu->V[(emu->opcode & 0x0F00) >> 8])) {
            emu->pc += 4;
        } else {
            emu->pc += 2;
        }
        break;
    default:
        _chip8emu_log_error(emu, "Unknown opcode: 0x%X\n", emu->opcode);
    }
    return C8ERR_OK;
}

int _chip8emu_opcode_handler_F(chip8emu* emu) {
    switch (emu->opcode & 0x00FF) {
    case 0x0007: /* FX07: Sets VX to the value of the delay timer */
        emu->V[(emu->opcode & 0x0F00) >> 8] = _chip8emu_timer_delay(emu);;
        emu->pc += 2;
        break;
    case 0x000A: /* FX0A: A key press is awaited, and then stored in VX. (blocking) */
        for (uint8_t i = 0; i < 0x10; i++) {
            if (emu->keystate(emu, i)) {
                emu->V[(emu->opcode & 0x0F00) >> 8] = i;
                emu->pc += 2;
                break;
            }
        }
        break;
    case 0x0015: /* FX15: Sets the delay timer to VX */
        _chip8emu_timer_delay_set(emu, emu->V[(emu->opcode & 0x0F00) >> 8]);
        emu->pc += 2;
        break;
    case 0x0018: /* FX18: Sets the sound timer to VX */
        _chip8emu_timer_sound_set(emu, emu->V[(emu->opcode & 0x0F00) >> 8]);
        emu->pc += 2;
        break;
    case 0x001E: /* FX1E: Add VX to I register */
        emu->I += emu->V[(emu->opcode & 0x0F00) >> 8];
        emu->pc += 2;
        break;
    case 0x0029: /* FX29: I=sprite_addr[Vx]; Sets I to the location of the sprite for the character in VX */
        emu->I = emu->V[(emu->opcode & 0x0F00) >> 8] * 5;
        emu->pc += 2;
        break;
    case 0x0033: /* FX33: Store a Binary Coded Decimal (BCD) of register VX to memory started from I */
        emu->memory[emu->I]     = emu->V[(emu->opcode & 0x0F00) >> 8] / 100;
        emu->memory[emu->I + 1] = (emu->V[(emu->opcode & 0x0F00) >> 8] / 10) % 10;
        emu->memory[emu->I + 2] = emu->V[(emu->opcode & 0x0F00) >> 8] % 10;
        emu->pc += 2;
        break;
    case 0x0055: /* FX55: */
        for (int i = 0; i <= ((emu->opcode & 0x0F00) >> 8); i++) {
            emu->memory[emu->I+i] = emu->V[i];
        }

        emu->I += ((emu->opcode & 0x0F00) >> 8) + 1;
        emu->pc += 2;
        break;
    case 0x0065: /* FX65: */
        for (int i = 0; i <= ((emu->opcode & 0x0F00) >> 8); i++) {
            emu->V[i] = emu->memory[emu->I + i];
        }
        emu->I += ((emu->opcode & 0x0F00) >> 8) + 1;
        emu->pc += 2;
        break;
    default:
        _chip8emu_log_error(emu, "Unknown opcode: 0x%X\n", emu->opcode);
    }
    return C8ERR_OK;
}
/* ******************** /Opcode handling implementation ******************** */

void chip8emu_exec_cycle(chip8emu *emu)
{
    emu->opcode = (uint16_t) (emu->memory[emu->pc] << 8 | emu->memory[emu->pc + 1]);

    emu->opcode_handlers[(emu->opcode & 0xF000) >> 12](emu);
}


int chip8emu_load_code(chip8emu *emu, uint8_t *code, long code_size)
{
    for(int i = 0; i < code_size; ++i)
      emu->memory[0x200 + i] = code[i];
    return C8ERR_OK;
}


int chip8emu_load_rom(chip8emu *emu, const char *filename)
{
    uint16_t filelen = 0;
    FILE *f = fopen(filename, "r");

    if (f == NULL) {
        _chip8emu_log_error(emu, "file %s does not exist\n", filename);
        return 1;
    }
    uint8_t c;
    while (fread(&c, 1, 1, f) != 0) {
        emu->memory[filelen + 512] = c;
        filelen++;
    }
    fclose(f);

    return C8ERR_OK;
}

void chip8emu_timer_tick(chip8emu *emu)
{
    if(emu->delay_timer > 0)
        --emu->delay_timer;

    if(emu->sound_timer > 0) {
        if(emu->sound_timer == 1)
            emu->beep(emu);
        --emu->sound_timer;
    }
}

static uint8_t _chip8emu_timer_delay(chip8emu* emu)
{
    uint8_t ret = emu->delay_timer;
    return ret;
}

static void _chip8emu_timer_delay_set(chip8emu* emu, uint8_t val)
{
    emu->delay_timer = val;
}

static void _chip8emu_timer_sound_set(chip8emu* emu, uint8_t val)
{
    emu->sound_timer = val;
}

#ifdef __cplusplus
}
#endif
