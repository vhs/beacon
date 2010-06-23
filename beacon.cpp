int IR_tx_pin = 7;
int IR_rx_pin = 4;

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


void loop () {
    /* Send out our state */
    send_header(IR_tx_pin);
    send_IR_one(IR_tx_pin); /* Status */
    if (red) {
        send_IR_zero(IR_tx_pin);
    }
    else if (blue) {
        send_IR_one(IR_tx_pin);
    }
    else {
        send_neutral_status(IR_tx_pin);
    }

    /* Look for IR commands */
    char msg = read_message(IR_rx_pin, 100000);
    if (msg == 'R') {
        red = LED_HIGH; blue = 0;
    }
    else if (msg == 'B') {
        blue = LED_HIGH; red = 0;
    }
}
