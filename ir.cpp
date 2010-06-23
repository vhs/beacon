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

#define IR_PROTOCOL_PERIOD 600
#define IR_CARRIER_PERIOD   26
#define IR_PULSE_OFF 1
#define IR_PULSE_ON  0

void flash_ir(int IR_tx_pin) {
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

void send_IR_for_micros (int IR_tx_pin, unsigned long t) {
    unsigned long done_at = micros() + t;
    while(micros() < done_at) {
        flash_ir(IR_tx_pin);
    }
}

void send_header (int IR_tx_pin) {
    digitalWrite(IR_tx_pin, 0);
    delayMicroseconds(3 * IR_PROTOCOL_PERIOD);
    send_IR_for_micros(IR_tx_pin, 3 * IR_PROTOCOL_PERIOD);
}

void send_IR_one (int IR_tx_pin) {
    digitalWrite(IR_tx_pin, 0);
    delayMicroseconds(IR_PROTOCOL_PERIOD);
    send_IR_for_micros(IR_tx_pin, IR_PROTOCOL_PERIOD * 2);
}

void send_IR_zero (int IR_tx_pin) {
    digitalWrite(IR_tx_pin, 0);
    delayMicroseconds(IR_PROTOCOL_PERIOD);
    send_IR_for_micros(IR_tx_pin, IR_PROTOCOL_PERIOD);
}

void send_neutral_status (int IR_tx_pin) {
    send_IR_for_micros(IR_tx_pin, 10000); /* on for 10ms */
    delayMicroseconds(1000);   /* off for 1ms */
}

bool wait_for_header(int IR_rx_pin, unsigned long timeout_us) {
    unsigned long timeout = micros() + timeout_us;

    unsigned long now = micros();
    while (now < timeout) {
        /* first, wait until the IR is off for a while */
        /* the spec says 3T, but lets just wait 1T */
        unsigned long header_low_at = 0;
        while (now < timeout) {
            int state = digitalRead(IR_rx_pin);
            if (state == IR_PULSE_OFF) {
                if (header_low_at == 0) {
                    /* just went low */
                    header_low_at = now;
                }
                else if (now > header_low_at + IR_PROTOCOL_PERIOD) {
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

/* Waits up to <timeout> seconds for a header and then reads the message
 *
 * Returns:
 * r = status red
 * b = status blue
 * R = capture for red
 * B = capture for blue
 * NULL if no message was read
 */
int read_message (int IR_rx_pin, unsigned long timeout) {
    if (wait_for_header(IR_rx_pin, timeout) == 0) return NULL;

    delayMicroseconds(IR_PROTOCOL_PERIOD/2);
    /* start of bit should always be low */
    if (digitalRead(IR_rx_pin)) return NULL;
    /* read the second half of the bit */
    delayMicroseconds(IR_PROTOCOL_PERIOD);
    /* second half always must be high */
    if (!digitalRead(IR_rx_pin)) return NULL;

    delayMicroseconds(IR_PROTOCOL_PERIOD);
    bool is_status = 0;
    if (digitalRead(IR_rx_pin)) {
        is_status = 1;
        /* now wait for the start of the next bit */
        delayMicroseconds(IR_PROTOCOL_PERIOD);
        if (digitalRead(IR_rx_pin)) return NULL;
    }
    else {
        is_status = 0;
    }

    delayMicroseconds(IR_PROTOCOL_PERIOD);
    /* second half always must be high */
    if (!digitalRead(IR_rx_pin)) return NULL;

    delayMicroseconds(IR_PROTOCOL_PERIOD);
    if (digitalRead(IR_rx_pin)) {
        return 'b' + (is_status ? 0 : 32) ;
    }
    else {
        return 'r' + (is_status ? 0 : 32) ;
    }
}

