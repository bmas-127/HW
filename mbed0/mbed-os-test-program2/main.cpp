#include "mbed.h"
#include "led.h"


DigitalOut redLED(LED_RED);

DigitalOut greenLED(LED_GREEN);

void Led(DigitalOut& ledpin);


int main()

{

   redLED = 1;

   greenLED = 1;

   while (true) {

       Led(redLED);

       Led(greenLED);

   }

}