#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

#include "time.h"

#include "pthread.h"

#ifndef BOARD_NATIVE
#include "periph/gpio.h"
#include "periph/i2c.h"
#endif /* !BOARD_NATIVE */

#ifdef MODULE_MTD_SDCARD
#include "mtd_sdcard.h"
#include "sdcard_spi.h"
#include "sdcard_spi_params.h"
#endif

#include "fs/fatfs.h"
#include "vfs.h"
#include "mtd.h"

#include "xtimer.h"
#include "u8g2.h"

#include "shell_commands.h"
#include "shell.h"

#include "chip8emu.h"


#if FATFS_FFCONF_OPT_FS_NORTC == 0
#include "periph/rtc.h"
#endif

#define SDCARD_MOUNT_POINT "/sdcard"

static fatfs_desc_t fatfs = {
    .vol_idx = 0
};

static vfs_mount_t sdcard_vfs_mount = {
    .mount_point = SDCARD_MOUNT_POINT,
    .fs = &fatfs_file_system,
    .private_data = (void *)&fatfs,
};

#define SDCARD_SPI_NUM (sizeof(sdcard_spi_params) / sizeof(sdcard_spi_params[0]))
mtd_dev_t *fatfs_mtd_devs[FF_VOLUMES];
extern sdcard_spi_t sdcard_spi_devs[SDCARD_SPI_NUM];
mtd_sdcard_t mtd_sdcard_devs[SDCARD_SPI_NUM];
/* always default to first sdcard*/
static mtd_dev_t *mtd1 = (mtd_dev_t*)&mtd_sdcard_devs[0];

#ifndef BOARD_NATIVE
    /**
     * @brief   RIOT-OS pin maping of U8g2 pin numbers to RIOT-OS GPIO pins.
     * @note    To minimize the overhead, you can implement an alternative for
     *          u8x8_gpio_and_delay_riotos.
     */
    static gpio_t pins[] = {
        [U8X8_PIN_I2C_CLOCK] = GPIO_PIN(PORT_B, 8),
        [U8X8_PIN_I2C_DATA] = GPIO_PIN(PORT_B, 9),
        [U8X8_PIN_RESET] = GPIO_UNDEF
    };

    /**
     * @brief   Bit mapping to indicate which pins are set.
     */
    static uint32_t pins_enabled = (
        (1 << U8X8_PIN_I2C_CLOCK) +
        (1 << U8X8_PIN_I2C_DATA) +
        (1 << U8X8_PIN_RESET)
    );
#endif /* !BOARD_NATIVE */

void *shell_thread(void *parameter) {
    (void) parameter;
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    return NULL;
}
/** @} */


u8g2_t u8g2;

int chip8emu_opcode_handler_D(chip8emu* emu) {
    /* DXYN: draw(Vx,Vy,N); draw at X,Y width 8, height N sprite from I register */
    uint8_t xo = emu->V[(emu->opcode & 0x0F00) >> 8]; /* x origin */
    uint8_t yo = emu->V[(emu->opcode & 0x00F0) >> 4];
    uint8_t height = emu->opcode & 0x000F;
    uint8_t sprite[0x10] = {0};

    memcpy(sprite, emu->memory + (emu->I * sizeof (uint8_t)), height);

    emu->V[0xF] = 0;
    for (uint8_t y = 0; y < height; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            int dx = (xo + x); /* display x or dest x*/
            int dy = (yo + y);
            if ((sprite[y] & (0x80 >> x)) != 0) { /* 0x80 -> 10000000b */
                if (!emu->V[0xF] && emu->gfx[(dx + (dy * 64))])
                    emu->V[0xF] = 1;
                emu->gfx[dx + (dy * 64)] ^= 1;
                u8g2_SetDrawColor(&u8g2, emu->gfx[dx + (dy * 64)]);
                u8g2_DrawBox(&u8g2, dx*2, dy*2, 2, 2);
            }
        }
    }
    emu->pc += 2;
    return C8ERR_OK;
}

void draw_callback(chip8emu* cpu) {
    (void) cpu;
}

int chip8emu_opcode_handler_0(chip8emu* cpu) {
    switch (cpu->opcode) {
    case 0x00E0: /* clear screen */
        memset(cpu->gfx, 0, 64*32);
        cpu->pc += 2;
        u8g2_ClearBuffer(&u8g2);
        break;

    case 0x00EE: /* subroutine return */
        cpu->pc = cpu->stack[--cpu->sp & 0xF] + 2;
        break;

    default: /* 0NNN: call program at NNN address */
        // printf("OpCode 0NNN is not implemented\n");
        break;
    }
    return C8ERR_OK;
}

bool keystate_callback(chip8emu* cpu, uint8_t key) {
    (void) cpu; (void) key;
    return false;
}

void beep_callback(chip8emu* cpu) {
    (void) cpu;
}

void *disp_thread(void *parameter) {
    (void) parameter;
    while (1) {
        u8g2_SendBuffer(&u8g2);
        xtimer_usleep(16666);
    }
    return NULL;
}

chip8emu *cpu;

void *emu_thread(void *parameter) {
    (void) parameter;
    uint8_t cycles = 0;
    while (true) {
        cycles++;
        chip8emu_exec_cycle(cpu);
        if (!(cycles%8)) {
            chip8emu_timer_tick(cpu);
        }
        /* get user's key presses, e.g.: getchar() or SDL_GetKeyState .. */

        /* give some delay */
        xtimer_usleep(100);
    }
    return NULL;
}
    

int main(void)
{
    srand ( time(NULL) );
    // random_init(0x9dff63c0);
    xtimer_sleep(1);
    printf("%s\n", "first line on main");

    for(unsigned int i = 0; i < SDCARD_SPI_NUM; i++){
        mtd_sdcard_devs[i].base.driver = &mtd_sdcard_driver;
        mtd_sdcard_devs[i].sd_card = &sdcard_spi_devs[i];
        mtd_sdcard_devs[i].params = &sdcard_spi_params[i];
        fatfs_mtd_devs[i] = &mtd_sdcard_devs[i].base;
        mtd_init(&mtd_sdcard_devs[i].base);
    }
    fatfs_mtd_devs[fatfs.vol_idx] = mtd1;

    vfs_mount(&sdcard_vfs_mount);

    
    /* initialize the display */
    printf("Initializing display..");

#ifdef NATIVE_BOARD
    printf("SDL.\n");
    u8g2_SetupBuffer_SDL_128x64_4(&u8g2, &u8g2_cb_r0);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
#else
    printf("I2C.\n");
    // full screen buffer takes 1024 bytes
    u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_riotos_hw_i2c, u8x8_gpio_and_delay_riotos);

    u8g2_SetPins(&u8g2, pins, pins_enabled);
    u8g2_SetDevice(&u8g2, I2C_DEV(0));
    u8g2_SetI2CAddress(&u8g2, 0x3C);

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
#endif

    u8g2_ClearDisplay(&u8g2);
    u8g2_SetBitmapMode(&u8g2, false); /* no transparent */

    pthread_t thid_shell;
    pthread_attr_t thattr_shell;

    pthread_attr_init(&thattr_shell);
    pthread_create(&thid_shell, &thattr_shell, shell_thread, NULL);

    pthread_t thid_disp;
    pthread_attr_t thattr_disp;

    pthread_attr_init(&thattr_disp);
    pthread_create(&thid_disp, &thattr_disp, disp_thread, NULL);

    cpu = chip8emu_new();

    cpu->draw = &draw_callback;
    cpu->keystate = &keystate_callback;
    cpu->beep = &beep_callback;
    cpu->opcode_handlers[0x0] = &chip8emu_opcode_handler_0;
    cpu->opcode_handlers[0xD] = &chip8emu_opcode_handler_D;

    /* start drawing in a loop */
    printf("Loading ROM..\n");

    chip8emu_load_rom(cpu, "/sdcard/TETRIS");
    printf("Loading ROM..OK.\n");

    pthread_t thid_emu;
    pthread_attr_t thattr_emu;

    pthread_attr_init(&thattr_emu);
    pthread_create(&thid_emu, &thattr_emu, emu_thread, NULL);
    
    pthread_join(thid_emu,NULL);

    return 0;
}

