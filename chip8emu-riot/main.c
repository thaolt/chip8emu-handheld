#include <stdio.h>
#include <pthread.h>
#include "chip8emu.h"

#define FACTORIAL_PARAM     (6U)
#define FACTORIAL_EXPECTED  (720U)

void *run(void *parameter) {
    size_t n = (size_t) parameter;
    size_t factorial = 1;

    printf("pthread: parameter = %u\n", (unsigned int) n);

    if (n > 0) {
        for (size_t i = 1; i <= n; i++) {
            factorial *= i;
        }
    }

    printf("pthread: factorial = %u\n", (unsigned int) factorial);
    pthread_exit((void *)factorial);

    return NULL;
}

int main(void)
{
    chip8emu *emudev = chip8emu_new();
    puts("Hello World!");
    pthread_t th_id;
    pthread_attr_t th_attr;

    size_t arg = FACTORIAL_PARAM;
    printf("main: parameter = %u\n", (unsigned int) arg);

    pthread_attr_init(&th_attr);
    pthread_create(&th_id, &th_attr, run, (void *) arg);
    size_t res;
    pthread_join(th_id, (void **) &res);
    printf("main: factorial = %u\n", (unsigned int) res);

    if (res == FACTORIAL_EXPECTED) {
        puts("SUCCESS");
    }
    else {
        puts("FAILURE");
    }

    uint32_t cycles = 0;
    while (true) {
        cycles++;
        chip8emu_exec_cycle(emudev);
        if (!(cycles%8)) chip8emu_timer_tick(emudev);
	/* printf("%4X\n", emudev->opcode); */
    }
    return 0;
}

