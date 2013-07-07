#ifndef ClickButton_H
#define ClickButton_H

#if (ARDUINO <  100)
#include <WProgram.h>
#else
#include <Arduino.h>
#endif


#define CLICKBTN_PULLUP HIGH


class ClickButton
{
  public:
    ClickButton(uint8_t buttonPin);
    ClickButton(uint8_t buttonPin, boolean active);
    ClickButton(uint8_t buttonPin, boolean active, boolean internalPullup);
    int Update();
    int click;                    // click code to return depending on button presses / clicks
    boolean depressed;            // the currently debounced button (press) state (presumably it is not sad :)
    int maxPresses;
    long debounceTime;
    long multiclickTime;
    long heldDownTime;
  private:
    uint8_t pin;                  // Arduino pin connected to the button
    boolean active;               // Type of button - active LOW or HIGH
    boolean btnFlick;             // Current appearant button state (which may flicker, or flick of a switch, etc..)
    boolean lastState;            // previous button reading
    int buttonPresses;            // Number of times button is pressed within CLICK_MULTICLICK_DELAY milliseconds
    long lastBounceTime;          // the last time the button input pin was toggled, due to noise or a press
};

#endif

