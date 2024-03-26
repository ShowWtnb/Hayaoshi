#ifndef PLAYER_LED_CONTROLLER_H
#define PLAYER_LED_CONTROLLER_H
#include "Arduino.h"

#define BLINK_INTERVAL 500

class PlayerLedController
{
public:
  PlayerLedController(int ser, int rclk, int srclk);
  void init();
  void handleEvent();
  void onLed(int ledPin);
  void offLed(int ledPin);
  void onLedByte(byte ledPin);
  void offLedByte(byte ledPin);
  void onBlinkLed(int ledPin);
  void offBlinkLed(int ledPin);
  void clearAllLeds();

private:
  void updateShiftRegister(byte leds);
  void reset();
  byte stLED = 0;
  byte stBlink = 0;
  int ser;
  int rclk;
  int srclk;
  int stBlinking = 0;
  unsigned long startMillis = 0;
  
};

#endif // PLAYER_LED_CONTROLLER_H