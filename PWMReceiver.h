/*
  PWMReceiver.h - Library for asynchronous PWM signal processing on the most of the arduino pins.

  For docs see PWMReceiver.cpp
  
  Created by Petr Újezdský, March 1, 2017.
  Released into the public domain.
*/

#ifndef PWMReceiver_h
#define PWMReceiver_h

#define PINS_COUNT 16

typedef void (*OnChangeFunc)(unsigned long);
typedef boolean (*FilterFunc)(unsigned long);
typedef unsigned long (*TransformFunc)(unsigned long);

class PWMReceiver {
  
  public:
    void attach(uint8_t arduinoPin, OnChangeFunc onChangeFunc);
    void attach(uint8_t arduinoPin, OnChangeFunc onChangeFunc, FilterFunc filterFunc);
    void attach(uint8_t arduinoPin, OnChangeFunc onChangeFunc, FilterFunc filterFunc, TransformFunc transformFunc);
    void detach(uint8_t arduinoPin);
    void lookForChanges();
    static boolean any(unsigned long);
    static boolean none(unsigned long);
    static unsigned long identity(unsigned long);
  private:
    OnChangeFunc _onChangeFunctions[PINS_COUNT];
    FilterFunc _filterFunctions[PINS_COUNT];
    TransformFunc _transformFunctions[PINS_COUNT];

    static unsigned long _waveStarts[];
    volatile static unsigned long _currentValues[];
    volatile static uint16_t _changedFlags;

    static void _handleChange();

    static void _void(unsigned long);
};

#endif