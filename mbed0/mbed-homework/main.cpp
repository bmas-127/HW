#include "mbed.h"


DigitalOut redLED(LED_RED);

DigitalOut greenLED(LED_GREEN);

void Led(DigitalOut& ledpin, int times){
    for(int i=0; i<times; ++i) { //blink for 10 times

       ledpin = !ledpin; // toggle led

       wait(0.2f);

   }
}


int main()

{

   redLED = 1;

   greenLED = 1;

   while (true) {

       Led(redLED, 6);

       Led(greenLED, 4);

   }

}