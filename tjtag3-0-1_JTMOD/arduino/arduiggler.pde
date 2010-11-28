/* Arduino Wiggler Cable Simulator */
/* Intended for use with tjtag3-0-1 with JTMOD patch. */

typedef
enum __arduOp
{
    OP_RESET = 0,
    OP_SEND  = 1,
    OP_READ  = 2,
    OP_RSVD  = 3
}
arduOp;                                                                                                                                                      


void setup(void)
{
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);

    // Pins 0-7 are part of PORTD
    // pins 0 and 1 are RX and TX, respectively

    pinMode(2, OUTPUT);     // WRST_N
    pinMode(3, OUTPUT);     // WTMS
    pinMode(4, OUTPUT);     // WTCK
    pinMode(5, OUTPUT);     // WTDI
    pinMode(6, OUTPUT);     // WTRST_N

    pinMode(7, INPUT);      // WTDO

    //Serial.begin(9600);
    Serial.begin(115200);

    /*
     * Before starting...
     * Say something invalid so that if the Arduino
     * gets reset in the middle of a run, the tjtag
     * program will know that something's gone wrong.
     *
     * This is invalid because our simple protocol
     * says that bits 6-0 should never be set except
     * when responding to a reset command with 0x42.
     */
    Serial.write(0x7F);
}

void loop(void)
{
    // Wait until byte available.
    while ( ! Serial.available() );

    int value = Serial.read();
    unsigned char byte = value;

    //Serial.print("byte received: ");
    //Serial.println(byte, HEX);

    unsigned char op = (byte & B01100000) >> 5 ;

    //Serial.print("opcode interpreted: ");
    //Serial.println(op, HEX);


    switch ( op )
    {
        case OP_RESET:
            {
                //Serial.println("  OP_RESET");
                // Clear all outputs.
                digitalWrite(2, LOW);
                digitalWrite(3, LOW);
                digitalWrite(4, LOW);
                digitalWrite(5, LOW);
                digitalWrite(6, LOW);

                // Respond that we're ready to go.
                Serial.write(0x42);

                // Clear out any other incoming data.
                Serial.flush();
            }
            break;
            
        case OP_SEND:
            {
                //Serial.println("  OP_SEND");

                // Set output pins to requested values.
                PORTD = (unsigned char)( (byte & B00011111) << 2 );

                // If needed:  put a Serial.write() here so that
                // we can "wait" for the pins to be set.
                Serial.write(0x4B);
            }
            break;

        case OP_READ:
            {
                //Serial.println("  OP_READ");

                unsigned char readByte = digitalRead(7) == HIGH ?     // WTDO
                                            B10000000 :     // send back a 1 in bit 7
                                            B00000000;      // send back a 0 in bit 7

                Serial.write(readByte);
            }
            break;

        default:
            {
                // BAD OPCODE
                // Send invalid data in response.
                Serial.write(0x7F);
            }
    };

    //Serial.println();
    //Serial.println();
}
