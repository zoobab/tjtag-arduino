#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "jt_mods.h"
//include "tjtag.h"

/***********************
 * Copied from tjtag.h:
 ***********************/
// --- Xilinx Type Cable ---

#define TDI     0
#define TCK     1
#define TMS     2
#define TDO     4

// ---Wiggler Type Cable ---

#define WTDI      3
#define WTCK      2
#define WTMS      1
#define WTDO      7
#define WTRST_N   4
#define WSRST_N   0
/***********************/


//define BAUDRATE B9600
#define BAUDRATE B115200

typedef
enum __arduOp
{
    OP_RESET = 0,
    OP_SEND  = 1,
    OP_READ  = 2,
    OP_RSVD  = 3
}
arduOp;

// Set to non-zero to enable debug.
int jtMod_debug = 0;

#define SYSCALL(expr)                                        \
{                                                            \
    int sc_rv = (expr);                                      \
    if (sc_rv < 0)                                           \
    {                                                        \
        char buf[128];                                       \
        snprintf(buf, 128,                                   \
            "Syscall(%s:%d: '" #expr "' failed, rv = %d",    \
            __FILE__,                                        \
            __LINE__,                                        \
            sc_rv);                                          \
        perror(buf);                                         \
    }                                                        \
    else if (jtMod_debug)                                    \
    {                                                        \
        fprintf(stderr,                                      \
                "Syscall: '" #expr "', rv = %d\n",           \
                sc_rv);                                      \
    }                                                        \
    /*usleep(100000);*/ \
}


#define ARDUOP_BITS (0x60) // 0110 0000
#define ARDUOP_LSHIFT (5)
#define ARDUOP_RESET  (OP_RESET << ARDUOP_LSHIFT) // currently zero...
#define ARDUOP_READ   (OP_READ  << ARDUOP_LSHIFT) // currently B0100 0000

#define ASSERT(expr) ASSERT_WFL(expr, __FILE__, __LINE__)

#define ASSERT_WFL(expr, file, line)                \
if ( !(expr) )                                      \
{                                                   \
    fprintf(stderr,                                 \
            "jtMod assert failed (%s:%d): '%s'\n",  \
            (file), (line),                         \
            (#expr) );                              \
                                                    \
    exit(42);                                       \
}
/* line after ASSERT_WFL macro */


const char* jt_portName = JTMOD_SERIAL_PORT_FILE_DEFAULT;
int fd = -1;
struct termios tio_old, tio_new;

int jtMod_init(void)
{

    // Read envt var to find name of port.
    const char * possiblePortName =
            getenv( JTMOD_SERIAL_PORT_FILE_ENVT_VAR );

    if ( possiblePortName != NULL )
    {
        jt_portName = possiblePortName;
    }
    
    // Open port.
    //fd = open( jt_portName, O_RDWR | O_NOCTTY | O_NONBLOCK );
    fd = open( jt_portName, O_RDWR | O_NOCTTY );

    if (fd < 0)
    {
        char buf[64];
        snprintf(buf, 64, "Failed to open '%s'",
                 jt_portName);
        buf[63] = '\0';
        perror(buf);
        exit(42);
    }

    printf("JTMOD: Opened port.\n");

    // Set up port settings.
    {
        tcgetattr( fd, &tio_old );  // save old

        memset( &tio_new, 0, sizeof(struct termios) );
        tio_new.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
        tio_new.c_iflag = IGNPAR;
        tio_new.c_oflag = 0;

        // "non-canonical" input -- no echo, no translation
        tio_new.c_lflag = 0;

        /*
         * for each read attempt, allow
         * us to block for up to 1 second while waiting for
         * characters
         * zero or more characters may be returned
         */

        tio_new.c_cc[VTIME] = 10;  /* inter-character timeout (deciseconds) */
        //tio_new.c_cc[VMIN]  = 0;   /* blocking read may return 1 or more chars */
        tio_new.c_cc[VMIN]  = 0;   /* blocking read may return 1 or more chars */

        tcflush  ( fd, TCIFLUSH );
        tcsetattr( fd,TCSANOW,&tio_new );
    }

    printf("JTMOD: Resetting Arduino.\n");

    // Send reset signal to Arduino,
    {
        unsigned char msg = ARDUOP_RESET;
        SYSCALL( write( fd, &msg, 1 ) );
    }

    {
        int tries = 0;
        while (1)
        {
            // wait for response before continuing.
            unsigned char msg;
            int bytesRead;
            SYSCALL( bytesRead = read( fd, &msg, 1 ) );

            if (jtMod_debug)
                printf("JTMOD:    Waiting...read %d bytes, 0x%X\n", bytesRead, msg);

            if ( bytesRead > 0 &&  msg == 0x42 )
                break;

            if (++tries > 10)
            {
                fprintf(stderr, "JTMOD:  Hmm.  Looks like the Arduino isn't responding.  Did you program it with the correct sketch?\n");
                exit(42);
            }
            else
            {
                sleep(1);
                //tcflush(fd, TCIOFLUSH);
            }
        }

    }

    printf("JTMOD: Ready.\n");
    return 0;
}

int jtMod_inp (unsigned char * p_byteToFill)
{
    // Read byte from port.
    unsigned char msg = ARDUOP_READ;
    int    bytesRead;
    int    tries;

    if (jtMod_debug)
        printf("JTMOD:  Entering jtMod_inp\n");

    SYSCALL( write( fd, &msg, 1 ) );

    tries = 0;
    bytesRead = -1;
    while (bytesRead <= 0 && tries < 2)
    {
        SYSCALL(bytesRead = read( fd, &msg, 1 ));
        tries++;
    }

    if (bytesRead < 0)
    {
        char buf[128];
        snprintf(buf, 128, "read from Arduino returned %d: ", bytesRead);
        buf[127] = '\0';
        perror(buf);
        exit(42);
    }

    // None of bits 6-0 should be set.
    if ( msg & 0x7F )
    {
        fprintf(stderr, "Invalid data (0x%X) on read from Arduino!\n", msg);
        exit(42);
    }

    *p_byteToFill = msg;
    *p_byteToFill ^= 0x80;      // it took me hours to find this

    if (jtMod_debug)
        printf("JTMOD:  Leaving jtMod_inp\n");

    return 0;
}

int jtMod_outp(unsigned char p_byteToSend)
{
    // Pack bits into port.
    unsigned char msg;
    int bytesRead;

    if (jtMod_debug)
        printf("JTMOD:  Entering jtMod_outp\n");

    // Populate bits 4,3,2,1,0
    msg = p_byteToSend;

    // Clear bits 7,6,5
    msg &= 0x1F;        // B0001 1111

    // Set bits 6,5 to '01'
    msg |= 0x20;        // B0010 0000

    // Send to Arduino
    SYSCALL( write( fd, &msg, 1 ) );

    // Wait for response before continuing.
    bytesRead = -1;
    while (bytesRead <= 0)
        SYSCALL( bytesRead = read( fd, &msg, 1) );

    if ( bytesRead != 1 || msg != 0x4B )
    {
        fprintf(stderr, "Invalid data (0x%X) while waiting for send to complete!\n", msg);
        exit(42);
    }

    if (jtMod_debug)
        printf("JTMOD:  Leaving jtMod_outp\n");

    return 0;
}

int jtMod_done(void)
{
    printf("JTMOD: Cleaning up.\n");

    // Send reset signal to Arduino,
    {
        unsigned char msg = ARDUOP_RESET;
        SYSCALL( write( fd, &msg, 1 ) );
    }

    // Reset and close port.
    tcsetattr(fd, TCSANOW, &tio_old);
    close(fd);
    printf("JTMOD: Done.\n");
    return 0;
}
