/*
  PWMReceiver.cpp - Library for asynchronous PWM signal processing on the most of the arduino pins.

  It is:
  * Quick - uses interrupts and event driven style of coding for the best performance and straightforward usage
  * Thread safe - the values accessed from within events are thread safe so you can use them without any worries of corrupted data
  * Ready to be computational heavy - it is asynchronous so you can do any computations as complex as you want (filter, transform and onChange handlers)
  
  It function as follows:

  1. Interrupt is enabled on the given pin and listens for PWM signals. It immediately updates internal value with read PWM value, independently on the main loop() function.
  2. In the main loop() function when you call lookForChanges(), it looks whether some change occured on any of the registered pins. If it occured, the pin related functions are
  invoked, in the following order:
    a) the filter function     - if it returns "true", the invocation continues. If it returns "false", the invocation stops here and the next pin is examined
    b) the transform function  - transforms PWM value to anything you want
    c) the onChange function    - do any stuff you want with the transformed value
  
  Note
  In case your loop() function will cycle slower than the PWM signals come in, you will miss these values altogether, reading only the most recent one in the next loop() cycle.
  This is because of the asynchronous nature.
  
  Depends on the EnableInterrupt library.
  
  Created by Petr Újezdský, March 1, 2017.
  Released into the public domain.
*/

#include "Arduino.h"
#include "PWMReceiver.h"

#define EI_ARDUINO_INTERRUPTED_PIN
#include <EnableInterrupt.h>

///////////////////////////////
// variables changed via ISR //
///////////////////////////////

// this is not volatile because it is accessed only from ISR
static unsigned long PWMReceiver::_waveStarts[PINS_COUNT];

// volatile variables - accessed from both main loop and ISR
volatile static unsigned long PWMReceiver::_currentValues[PINS_COUNT];
volatile static uint16_t PWMReceiver::_changedFlags = 0;


/**
 * Attaches PWM receiver on given pin.
 * Defaults with 'any' filter function and 'identity' transform function.
 * 
 * arduinoPin     - arduino pin to attach the receiver to
 * onChangeFunc   - function to call when the value has changed
 */
void PWMReceiver::attach(uint8_t arduinoPin, OnChangeFunc onChangeFunc) {
  PWMReceiver::attach(arduinoPin, onChangeFunc, any);
}

/**
 * Attaches PWM receiver on given pin.
 * Defaults with 'identity' transform function.
 * 
 * arduinoPin     - arduino pin to attach the receiver to
 * onChangeFunc   - function to call when the value has changed
 * filterFunc     - function to filter out input values
 */
void PWMReceiver::attach(uint8_t arduinoPin, OnChangeFunc onChangeFunc, FilterFunc filterFunc) {
  PWMReceiver::attach(arduinoPin, onChangeFunc, filterFunc, identity);
}

/**
 * Attaches PWM receiver on given pin.
 * 
 * arduinoPin     - arduino pin to attach the receiver to
 * onChangeFunc   - function to call when the value has changed
 * filterFunc     - function to filter out input values
 * transformFunc  - function to transform input values
 */
void PWMReceiver::attach(uint8_t arduinoPin, OnChangeFunc onChangeFunc, FilterFunc filterFunc, TransformFunc transformFunc) {
  enableInterrupt(arduinoPin, _handleChange, CHANGE);
  _onChangeFunctions[arduinoPin] = onChangeFunc;
  _filterFunctions[arduinoPin] = filterFunc;
  _transformFunctions[arduinoPin] = transformFunc;
}

/**
 * Detaches PWM receiver from given pin.
 * 
 * arduinoPin     - arduino pin to detach the receiver from
 */
void PWMReceiver::detach(uint8_t arduinoPin) {
  disableInterrupt(arduinoPin);
  _onChangeFunctions[arduinoPin] = _void;
  _filterFunctions[arduinoPin] = none;
  _transformFunctions[arduinoPin] = identity;
}

/**
 * Checks the current state of PWM values on attached pins. In case that any value has changed,
 * the following actions are performed:
 *   - checks current value via bound filter function, when it returns "true", continue next
 *   - transforms current value via bound transform function
 *   - calls bound onChange handler with transformed value
 */
void PWMReceiver::lookForChanges() {
  // unsafe read, but that is fine - the flags are only turned ON from _handleChange()
  if(_changedFlags) {
    // turn off interrupts to perform thread safe reads of shared variables
    noInterrupts();

    // make local copies of the shared variables
    uint16_t flags = _changedFlags;
    unsigned long values[sizeof(_currentValues)];
    memcpy(values, _currentValues, sizeof(_currentValues));

    // reset the flags, we will process all changed pins
    _changedFlags = 0;
    
    // enable interrupts again
    interrupts();

    // go through all pins and check for the change
    for (int pin = 0; pin < PINS_COUNT; pin++) {
      if (flags & 2 << pin) {
        OnChangeFunc onChangeFunc = _onChangeFunctions[pin];
        FilterFunc filterFunc = _filterFunctions[pin];
        TransformFunc transformFunc = _transformFunctions[pin];
        unsigned long currentValue = values[pin];

        if ((*filterFunc) (currentValue)) {
          (*onChangeFunc) ((*transformFunc) (currentValue));
        }
      }
    }    
  }
}

/**
 * The ISR registered for pin change. This is called when the pin's state changes from
 * LOW to HIGH or from HIGH to LOW.
 */
static void PWMReceiver::_handleChange() {
  // arduinoInterruptedPin variable is injected via EI_ARDUINO_INTERRUPTED_PIN
  uint8_t interruptedPin = arduinoInterruptedPin;
  
  if (arduinoPinState > 0) {
    // LOW -> HIGH - the beginning of the PWM signal wave
   PWMReceiver::_waveStarts[interruptedPin] = micros();
  } else {
    // HIGH -> LOW - the end of the PWM signal wave, measure elapsed time in microseconds
    _currentValues[interruptedPin] = micros() - PWMReceiver::_waveStarts[interruptedPin];
    _changedFlags |= 2 << interruptedPin;
  }
}

/**
 * Filter function that returns true for any input.
 */
static boolean PWMReceiver::any(unsigned long value) {
  return true;
}

/**
 * Filter function that returns false for any input.
 */
static boolean PWMReceiver::none(unsigned long value) {
  return false;
}

/**
 * Identity transform function that returns the input as is.
 */
static unsigned long PWMReceiver::identity(unsigned long value) {
  return value;
}

/**
 * Void function that does nothing.
 */
static void PWMReceiver::_void(unsigned long value) {}
