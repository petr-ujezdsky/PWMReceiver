# PWMReceiver
Arduino library for asynchronous PWM signal processing on the most of it's pins.

## It is:
* **Quick** - uses interrupts and event driven style of coding for the best performance and straightforward usage
* **Thread safe** - the values accessed from within events are thread safe so you can use them without any worries of corrupted data
* **Ready to be computational heavy** - it is asynchronous so you can do any computations as complex as you want (`filter`, `transform` and `onChange` handlers)
  
## How it works:
1. Interrupt is enabled on the given pin and listens for PWM signals. It immediately updates internal value with read PWM value, independently on the main loop() function.
1. In the main `loop()` function when you call `lookForChanges()`, it looks whether some change occured on any of the registered pins. If it occured, the pin related functions are invoked, in the following order:
   1. the filter function     - if it returns `true`, the invocation continues. If it returns `false`, the invocation stops here and the next pin is examined
   1. the transform function  - transforms PWM value to anything you want
   1. the onChange function    - do any stuff you want with the transformed value

#### Note
>In case your loop() function will cycle slower than the PWM signals come in, you will miss these values altogether, reading only the most recent one in the next loop() cycle. This is because of the asynchronous nature.

## The most simple example
```C
#include <PWMReceiver.h>

PWMReceiver pwm;

void setup() {
  Serial.begin(9600);
  Serial.println("PWMReceiver simple example");
  
  // attach PWM receiver to given arduino PIN with onChange handler defined below
  pwm.attach(8, onChange);
}

void loop() {
  // let the PWMReceiver library call the handlers
  pwm.lookForChanges();
  // do other stuff
}

void onChange(unsigned long value) {
  // do complex stuff with the value, eg. print it to serial
  Serial.print("Detected pulse, duration: ");
  Serial.print(value);
  Serial.println(" us");
}
```

Depends on the [EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt) library.