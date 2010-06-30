#include <IRremote.h>
IRsend irsend;

int IR_tx_pin = 10;                                                             
int red_button  = 19;                                                           
int blue_button = 14;      

#define CAPTURE_RED  0
#define CAPTURE_BLUE 1
#define STATUS_RED   2
#define STATUS_BLUE  3
                                                
                                                                                
void setup () {                                                                 
    pinMode(red_button, INPUT);  digitalWrite(red_button, HIGH);                
    pinMode(blue_button, INPUT); digitalWrite(blue_button, HIGH);               
    Serial.begin(9600);                                                         
}                                                                               
                                                                                
void loop () {                                                                  
    static char state = 'n';                                                    
                                                                                
    if (digitalRead(red_button) == LOW) {                                       
        state = 'r';                                                            
    }                                                                           
    if (digitalRead(blue_button) == LOW) {                                      
        state = 'b';                                                            
    }                                                                           
                                                                                
    if (state == 'n') return;                                                   
                       
    if (state == 'r') {                                             
      irsend.sendSony(CAPTURE_RED, 2);
    }                                                                           
    else {                                                                      
      irsend.sendSony(CAPTURE_BLUE, 2);
    }       
    delay(45);    
} 
