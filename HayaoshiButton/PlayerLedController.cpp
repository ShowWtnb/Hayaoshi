#include "PlayerLedController.h"
#include "Arduino.h"

PlayerLedController::PlayerLedController(int a_ser, int a_rclk, int a_srclk)
{
    ser = a_ser;
    rclk = a_rclk;
    srclk = a_srclk;
}
void PlayerLedController::init()
{
    // Set all the pins of 74HC595 as OUTPUT
    pinMode(ser, OUTPUT);
    pinMode(rclk, OUTPUT);
    pinMode(srclk, OUTPUT);

    //
    clearAllLeds();
}
void PlayerLedController::handleEvent()
{
    if (stBlink > 0)
    {
        if (stBlinking == 0)
        {
            unsigned long current = millis();
            if ((current - startMillis) > BLINK_INTERVAL)
            {
                startMillis = current;
                offLedByte(stBlink);
                stBlinking = 1;
            }
        }
        else if (stBlinking == 1)
        {
            unsigned long current = millis();
            if ((current - startMillis) > BLINK_INTERVAL)
            {
                startMillis = current;
                onLedByte(stBlink);
                stBlinking = 0;
            }
        }
    }
}
void PlayerLedController::onLed(int ledPin)
{
    if (ledPin < 0)
    {
        return;
    }
    // Serial.print("onLed ");
    // Serial.println(ledPin);
    stLED |= 1 << ledPin;
    updateShiftRegister(stLED);
}
void PlayerLedController::offLed(int ledPin)
{
    if (ledPin < 0)
    {
        return;
    }
    // Serial.print("offLed ");
    // Serial.println(ledPin);
    byte invert = ~(1 << ledPin);
    stLED &= invert;
    updateShiftRegister(stLED);
}
void PlayerLedController::onLedByte(byte ledPin)
{
    if (ledPin < 0)
    {
        return;
    }
    Serial.print("onLedByte ");
    Serial.println(ledPin);
    stLED |= ledPin;
    updateShiftRegister(stLED);
}
void PlayerLedController::offLedByte(byte ledPin)
{
    if (ledPin < 0)
    {
        return;
    }
    Serial.print("offLedByte ");
    Serial.println(ledPin);
    byte invert = ~(ledPin);
    stLED &= invert;
    updateShiftRegister(stLED);
}
void PlayerLedController::onBlinkLed(int ledPin)
{
    if (ledPin < 0)
    {
        return;
    }
    Serial.print("onBlinkLed ");
    Serial.println(ledPin);
    stBlink |= 1 << ledPin;
    onLed(ledPin);
}
void PlayerLedController::offBlinkLed(int ledPin)
{
    if (ledPin < 0)
    {
        return;
    }
    Serial.print("offBlinkLed ");
    Serial.println(ledPin);
    reset();
    byte invert = ~(1 << ledPin);
    stBlink &= invert;
    offLed(ledPin);
}
void PlayerLedController::clearAllLeds()
{
    stLED = 0;
    stBlink = 0;
    byte leds = 0;
    updateShiftRegister(leds);
}

void PlayerLedController::reset()
{
    stBlinking = 0;
    startMillis = 0;
}

void PlayerLedController::updateShiftRegister(byte leds)
{
    if (leds < 0)
    {
        return;
    }
    digitalWrite(rclk, LOW);
    shiftOut(ser, srclk, LSBFIRST, leds);
    digitalWrite(rclk, HIGH);
}