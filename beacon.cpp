/*
 * VHS Beacon
 *
 * This beacon will act as a "capture point" that will express
 * in one of 3 states - Neutral, Red, or Blue.
 *
 * The beacon will indicate it's state using a multicolour LED
 * and via an infrared pulse.
 *
 * The beacon can be captured by sending an infrared pulse upwards
 * from underneath the beacon.
 *
 * Infrared Protocol
 *
 *   The communication occurs via infrared pulse, on a 37.9khz 
 *   carrier wave with 2 bits of information.  A logical one is
 *   formed by a space of 600µS (1T) and a pulse of 1200µS (2T).
 *   A logical zero is formed by a space of 1T and a pulse of 1T.
 *
 *   Protocol: START, COMMAND, TEAM
 *     START - after a space of at least 3T, a 3T pulse indicates 
 *             the start of the message.
 *     COMMAND - a 0 bit indicates a capture message, sent from a 
 *               robot to a beacon.
 *             - a 1 bit indicates a status message, sent from a
 *               beacon to a robot.
 *     TEAM - a 0 bit indicates the red team
 *          - a 1 bit indicates the blue team
 *
 *   Example: Blue robot wishes to capture a beacon
 *
 *      ____  __  ___
 *      |   | | | |  |
 *   ___|   |_| |_|  |___
 *
 *   A beacon indicates a neutral state with a 10ms high pulse
 *   followed by a 1ms space.
 *
 */

/* For the infrared */
#define IR_PROTOCOL_PERIOD 600
#define IR_CARRIER_PERIOD   26
#define IR_PULSE_OFF 1
#define IR_PULSE ON  0
int IR_tx_pin = 2;
int IR_rx_pin = 8;

/* For the Multicolour LEDs */
#define LED_HIGH 56 
#define LED_LOW  0 
#define LED_R_pin 9
#define LED_G_pin 10
#define LED_B_pin 11
int red = 0;
int blue = 0;
int green = 0;
bool ir_on = 0;

void update_LED (void) {
    analogWrite(LED_R_pin, red);
    analogWrite(LED_G_pin, green);
    analogWrite(LED_B_pin, blue);
}

void setup () {
    pinMode(5, INPUT); /* pot */
    pinMode(IR_tx_pin, OUTPUT);
    pinMode(LED_R_pin, OUTPUT);
    pinMode(LED_G_pin, OUTPUT);
    pinMode(LED_B_pin, OUTPUT);

    green = LED_HIGH; update_LED();
    delay(800);
}


void flash_ir(void) {
    static unsigned long ir_time  = 0;
    static bool         ir_level = 0;
    unsigned long now = micros();

    /* if ir_time gets too far behind, bring it up to date */
    if (now > ir_time + 100) {
        ir_time = now;
    }

    if (now > ir_time + IR_CARRIER_PERIOD) {
        ir_level = !ir_level;
        digitalWrite(IR_tx_pin, ir_level);
        ir_time += IR_CARRIER_PERIOD;
    }
}

void send_IR_for_micros (unsigned long t) {
    unsigned long done_at = micros() + t;
    while(micros() < done_at) {
        flash_ir();
    }
}

void send_header (void) {
    digitalWrite(IR_tx_pin, 0);
    delayMicroseconds(3 * IR_PROTOCOL_PERIOD);
    send_IR_for_micros(3 * IR_PROTOCOL_PERIOD);
}

void send_IR_one (void) {
    digitalWrite(IR_tx_pin, 0);
    delayMicroseconds(IR_PROTOCOL_PERIOD);
    send_IR_for_micros(IR_PROTOCOL_PERIOD * 2);
}

void send_IR_zero (void) {
    digitalWrite(IR_tx_pin, 0);
    delayMicroseconds(IR_PROTOCOL_PERIOD);
    send_IR_for_micros(IR_PROTOCOL_PERIOD);
}

void send_neutral_status (void) {
    send_IR_for_micros(10000); /* on for 10ms */
    delayMicroseconds(1000);   /* off for 1ms */
}

bool wait_for_header() {
    unsigned long timeout = micros() + 5000;

    unsigned long now = micros();
    while (now < timeout) {
        /* first, wait until the IR is off for 3t */
        unsigned long header_low_at = 0;
        while (now < timeout) {
            int state = digitalRead(IR_rx_pin);
            if (state == IR_PULSE_OFF) {
                if (header_low_at == 0) {
                    /* just went low */
                    header_low_at = now;
                }
                else if (now > header_low_at + 3*IR_PROTOCOL_PERIOD) {
                    /* it's been 3t since the pulse has been off */
                    break;
                }
            }
            else {
                /* IR went high again, reset timer */
                header_low_at = 0;
            }

            now = micros();
        }

        /* now look for the next IR pulse of 3s */
        unsigned long header_high_at = 0;
        while (now < timeout) {
            int state = digitalRead(IR_rx_pin);
            if (state == IR_PULSE_ON) {
                if (header_high_at == 0) {
                    /* just went high */
                    header_high_at = now;
                }
                else if (now > header_high_at + 3*IR_PROTOCOL_PERIOD) {
                    /* it's been 3t since the pulse has been off */
                    return(1);
                }
            }
            else {
                /* pulse is still off, so keep waiting longer */
            }
            now = micros();
        }
    }
    return(0);
}

int read_bit (void) {
    /* TODO */
}

void loop () {
    red = green = blue = 0;

    /* Send out our state */
    send_header();
    send_IR_one(); /* Status */
    if (red) {
        send_IR_zero();
    }
    else if (blue) {
        send_IR_one();
    }
    else {
        send_neutral_status();
    }

    /* Look for IR commands */
    if (wait_for_header()) {
        int command_bit = read_bit();
        if (command_bit == INVALID_BIT) {
            return;
        }
        /* We got a header, now read a byte */
        delayMicroseconds(IR_PROTOCOL_PERIOD/2);
        /* Read the first half */
        int state = digitalRead(IR_rx_pin);
    }
    // wait for low
    // wait for high for 3t
    // read 

    /* Old code 
    blue = LED_HIGH;
    update_LED();
    int wave_duration;
    if ((millis()/10000) % 2) {
        wave_duration = 100000;
    }
    else {
        wave_duration = 200000;
    }
    delayMicroseconds(wave_duration/2);

    blue = 0; red = LED_HIGH;
    update_LED();
    unsigned long on_until = micros() + wave_duration/2;
#define WAIT_TIME 9
    while (micros() < on_until) {
        digitalWrite(IR_tx_pin, HIGH);
        delayMicroseconds(WAIT_TIME);
        digitalWrite(IR_tx_pin, LOW);
        delayMicroseconds(WAIT_TIME);
    }
    */
}
