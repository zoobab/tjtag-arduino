
#include <unistd.h>
#include <stdio.h>
#include "jt_mods.h"

int main(void)
{
    unsigned char byte;
    jtMod_init();

    for (byte = 0; byte < 32; byte++)
    {
        jtMod_outp(byte);
        usleep(500 * 1000); // half second
    }

    /*
    while (1)
    {
        jtMod_inp(&byte);

        printf("Input byte: 0x%X\n", byte);
        usleep(500 * 1000); // half second
    }
    */

    jtMod_done();

    return 0;
}
