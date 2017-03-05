/*
  Example with full usage of the library. It showcases how the onChange, filter and transform handlers can be used.

  How to play:
  - connect pin 8 with ground
  - generate pulse by disconnecting it from the ground for some time.
    If the pulse duration is below 1s, it will be allowed for processing (filter function), converted from microseconds to miliseconds (transform function)
    and pritned out to Serial (onChange function)

  Note that if you use some mechanical button, it can generate more than 1 impulse due to it's internal spring rebound.

  You can create the impulse also by connecting pin 8 to ground via pull-down resistor and via button to the +5V. You generate the pulse by PUSHING the button
  instead of releasing it. In this case uncomment the "INPUT_PULLUP" line.
  I have provided the "inverse" example because of simplicity - there is no INPUT_PULLDOWN choice so you have to have extra resistor at hands.
 */

#include <PWMReceiver.h>

const int PIN = 8;

PWMReceiver pwm;

void setup() {
  Serial.begin(9600);
  Serial.println("PWMReceiver example with full usage");

  // enable pullup resistor to keep value at HIGH when not connected to ground
  pinMode(PIN, INPUT_PULLUP);
  
  // attach PWM receiver to given arduino PIN with custom handlers
  pwm.attach(PIN, onChange, filter, transform);
}

void loop() {
  // let the PWMReceiver library call the handlers
  pwm.lookForChanges();
  
  // report stuff to Serial
  report();
}

/**
 * Handler function called when the value change is detected from within the main loop().
 * Since this is not called from within the ISR but from the main loop(), it:
 * - can be computational intense
 * - is thread safe, you do not need to worry about variable overwrite during it's reading
 *
 * value    - current value
 */
void onChange(unsigned long value) {
  Serial.print("Detected pulse, duration: ");
  Serial.print(value);
  Serial.println(" ms");
}

/**
 * Filter function for discarding / accepting input values. If this function returns false, the onChange handler above
 * will not be called at all.
 *
 * value      - current value to test
 *
 * return     - true to accept given input value, false to discarding it
 */
boolean filter(unsigned long value) {
  // 1s = 1mil microseconds
  return value < (long) 1000000;
}

/**
 * Transforms input value from microseconds to miliseconds
 */
unsigned long transform(unsigned long value) {
  return value / 1000;
}



// counter for how many loops are processed per second
long loops = 0;

// last timestamp for report output
unsigned long previousMillis = 0;

// delay between text reports to Serial
const long reportInterval = 3000;

void report() {
  loops++;
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= reportInterval) {
    previousMillis = currentMillis;

    Serial.print("Doing ");
    Serial.print(loops * 1000 / reportInterval);
    Serial.println(" loops / s");
    
    loops = 0;
  }
}
