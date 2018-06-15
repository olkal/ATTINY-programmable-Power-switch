#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <EEPROM.h>
#include "config.h"

#define DEBOUNCE_MS                 30
#define POWER_OFF                   0x00
#define POWER_ON                    0x01
#define POWER_OFF_DELAY_WAIT        0x02
#define POWER_OFF_REQUEST           0x03
#define BATTERY_EMPTY               0x00
#define BATTERY_VERY_LOW            0x01
#define BATTERY_LOW                 0x02
#define BATTERY_GOOD                0x03
#define CELL_COUNT_MIN              0x01
#define CELL_COUNT_MAX              0x03

byte mosfetPin = 0;
byte ledPin = 1;
byte buttonPin = 2;
byte auxPin = 3;
byte battSensePin = A2;

int lastBattVal;
boolean lastButtonState = 1;
boolean buttonHold;
unsigned long buttonHoldTime;
unsigned long lastButtonAction;
unsigned long lastReadBattVoltage;
byte ledValue;
long ledTime;
long powerOnTime;
byte powerState = 0; //0=off, 1=on, 2=delayed shut down
byte battStatus = BATTERY_GOOD;
unsigned int voltageCalFactor;
byte cellCount;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(mosfetPin, OUTPUT);
  analogReference(INTERNAL);
  voltageCalFactor = word(EEPROM.read(0), EEPROM.read(1));
  cellCount = EEPROM.read(2);
  setPower(POWER_ON);
  delay(10);
  setConfig();
  for (byte c = 0; c < cellCount; c++) {
    digitalWrite(ledPin, 1);
    delay(200);
    digitalWrite(ledPin, 0);
    delay(200);
  }
  delay(200);
}

void loop() {
  checkSleep();
  checkTimerPwrOff();
  ledControl();
  if (powerState == POWER_OFF_DELAY_WAIT) {
    if ((lastButtonAction + AUX_SHTD_DELAY) < millis()) {
      setPower(POWER_OFF);
    }
  }
  else {
    button();
  }

  if ((lastReadBattVoltage + 250) < millis()) {
    readBattVoltage();
    lastReadBattVoltage = millis();
  }
  handleBattVoltage();
}

//**********************************

void ISR_wake()
{
}

void checkSleep() {
  if ((powerState == POWER_OFF) && (digitalRead(buttonPin) == HIGH)) {
    if ((lastButtonAction + 2000) < millis()) {
      if (SLEEP_ENABLE) {
        CPU_sleep();
      }
    }
  }
}

void checkTimerPwrOff() {
  if (TIMER_PWR_OFF_DELAY) {
    if (millis() > (powerOnTime + TIMER_PWR_OFF_DELAY)) {
      setPower(POWER_OFF_REQUEST);
    }
  }
}

void CPU_sleep()
{
  pinMode(auxPin, OUTPUT);
  pinMode(battSensePin, OUTPUT);
  digitalWrite(battSensePin, LOW);
  digitalWrite(auxPin, LOW);
  digitalWrite(ledPin, LOW);
  ADCSRA &= ~(1 << ADEN); //Disable ADC
  //byte gtccr = GTCCR;
  //GTCCR = 0;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(0, ISR_wake, LOW);
  sleep_cpu(); //go to sleep
  detachInterrupt(0); //wake up
  sleep_disable(); 
  pinMode(battSensePin, INPUT);
  ADCSRA |= (1 << ADEN); //Enable ADC
  //GTCCR = gtccr;
  lastReadBattVoltage -= 250;
  lastButtonState = HIGH;
  pinMode(auxPin, INPUT);
}

void button() {
  boolean _buttonState = digitalRead(buttonPin);
  if (_buttonState != lastButtonState) {
    if (_buttonState == LOW) {
      if ((lastButtonAction + DEBOUNCE_MS) < millis()) {
        buttonHold = true;
      }
    }
    else {
      buttonHold = false;
    }
    lastButtonAction = millis();
    lastButtonState = _buttonState;
  }

  if (buttonHold) {
    if (powerState == POWER_OFF) {
      if ((lastButtonAction + BUTTON_PWR_ON_HOLD_TIME) <= millis()) {
        setPower(POWER_ON);
        buttonHold = false;
      }
    }
    else if (powerState == POWER_ON) {
      if ((lastButtonAction + BUTTON_PWR_OFF_HOLD_TIME) <= millis()) {
        setPower(POWER_OFF_REQUEST);
        buttonHold = false;
      }
    }
  }
}

void setPower(byte newlevel) {
  if (newlevel == POWER_OFF) {
    digitalWrite(mosfetPin, LOW);
    powerState = POWER_OFF;
  }
  else if (newlevel == POWER_ON) {
    digitalWrite(mosfetPin, HIGH);
    powerState = POWER_ON;
    powerOnTime = millis();
  }

  else if (newlevel == POWER_OFF_REQUEST) {
    if (powerState == POWER_ON) {
      if (AUX_IS_SHTD_SIGNAL_OUT) {
        powerState = POWER_OFF_DELAY_WAIT; //wait until power-off delay has counted down (RPI shut-down) 
        lastButtonAction = millis();
        auxControl();
      }
      else {
        digitalWrite(mosfetPin, LOW);
        powerState = POWER_OFF;
      }
    }
  }
}

void auxControl() {
  pinMode(auxPin, OUTPUT);
  boolean _val = AUX_SHTD_SIGNAL_HIGH_LOW;
  digitalWrite(auxPin, _val);
  delay(AUX_SHTD_SIGNAL_LENGTH);
  _val = ~_val; //flip
  digitalWrite(auxPin, _val);
  pinMode(auxPin, INPUT);
}

void ledControl() {
  int _ledontime;
  int _ledofftime;
  boolean _ledBlink;
  if (powerState == POWER_OFF) {
    ledValue = LED_OFF_BRIGHTNESS;
  }
  else if (powerState == POWER_ON) {
    if (battStatus == BATTERY_GOOD) {
      _ledBlink = 0;
      ledValue = LED_ON_BRIGHTNESS;
    }
    else if (battStatus == BATTERY_LOW) {
      _ledBlink = 1;
      _ledontime = LED_BLINK_ON_TIME * 2;
      _ledofftime = LED_BLINK_OFF_TIME;
    }
    else if (battStatus == BATTERY_EMPTY) {
      _ledBlink = 1;
      _ledontime = LED_BLINK_ON_TIME;
      _ledofftime = LED_BLINK_OFF_TIME;
    }
  }
  else if (powerState == POWER_OFF_DELAY_WAIT) {
    _ledBlink = 1;
    _ledontime = LED_BLINK_FAST_ON_TIME;
    _ledofftime = LED_BLINK_FAST_OFF_TIME;
  }
  if (_ledBlink) {
    if (ledTime < millis()) {
      if (ledValue < LED_ON_BRIGHTNESS) {
        ledValue = LED_ON_BRIGHTNESS;
        ledTime = millis() + _ledontime;
      }
      else {
        ledValue = LED_OFF_BRIGHTNESS;
        ledTime = millis() + _ledofftime;
      }
    }
  }
  analogWrite(ledPin, ledValue);
}

int readBattVoltage() {
  if (powerState) { //mosfet must be switched on for analog read battery voltage
    int _val;
    int _newval;
    _newval = analogRead(battSensePin);
    if (lastBattVal < _newval) { //use the highest of the two last conversions
      _val = _newval;
    }
    else {
      _val = lastBattVal;
    }

    _val = ((long)_val * (long)voltageCalFactor) / 100L;
    if (_val > (cellCount * CELL_VOLTAGE_LOW)) {
      battStatus = BATTERY_GOOD;
    }
    else if (_val > (cellCount * CELL_VOLTAGE_VERY_LOW)) {
      battStatus = BATTERY_LOW;
    }
    else if (_val > (cellCount * CELL_VOLTAGE_EMPTY)) {
      battStatus = BATTERY_VERY_LOW;
    }
    else {
      battStatus = BATTERY_EMPTY;
    }
    lastBattVal = _newval;
    return _val;
  }
}

void handleBattVoltage() {
  if ((battStatus == BATTERY_LOW) && BATTERY_LOW_AUTO_OFF) {
    if (millis() < (powerOnTime + 3000)) {
      setPower(POWER_OFF);
    }
    else {
      setPower(POWER_OFF_REQUEST);
    }
  }
  if ((battStatus == BATTERY_VERY_LOW) && BATTERY_VERY_LOW_AUTO_OFF) {
    if (millis() < (powerOnTime + 3000)) {
      setPower(POWER_OFF);
    }
    else {
      setPower(POWER_OFF_REQUEST);
    }
  }
  else if ((battStatus == BATTERY_EMPTY) && BATTERY_EMPTY_AUTO_OFF) {
    setPower(POWER_OFF);
  }
}

void setConfig() {
  if (digitalRead(buttonPin) == LOW) {
    boolean _buttonhold = 1;
    boolean _done = 0;
    digitalWrite(ledPin, 0);
    long _button_release_time = 0;
    while (1) {
      if ((millis() > 5000) && (millis() < 5050)) digitalWrite(ledPin, 1);
      else if ((millis() > 10000) && (millis() < 10050)) digitalWrite(ledPin, 1);
      else digitalWrite(ledPin, 0);

      if ((digitalRead(buttonPin) == HIGH) && (_button_release_time == 0)) {
        _button_release_time = millis();
      }

      else if (_button_release_time > 10000) {
        digitalWrite(ledPin, 1);
        calibrateBattVoltage();
        while (1) {
          digitalWrite(ledPin, 0);
          delay(1000);
          digitalWrite(ledPin, 1);
          delay(1000);
        }
      }
      else if (_button_release_time > 5000) {
        digitalWrite(ledPin, 1);
        int v = readBattVoltage();
        int _cellcount = v / CELL_VOLTAGE_LOW;
        if ((_cellcount > CELL_COUNT_MIN) && (_cellcount <= CELL_COUNT_MAX)) {
          EEPROM.write(2, (_cellcount & 0xFF));
          cellCount = EEPROM.read(2);
          while (1) {
            for (byte c = 0; c < cellCount; c++) {
              digitalWrite(ledPin, 1);
              delay(200);
              digitalWrite(ledPin, 0);
              delay(200);
            }
            delay(1000);
          }
        }
      }
    }
  }
}

int calibrateBattVoltage() {
  setPower(POWER_ON);
  delay(100);
  int _val = 0;
  for (byte i = 0; i < 10; i++) { //repeated readings
    _val += analogRead(battSensePin);
    delay(100);
  }
  _val = ((long)AUTO_CAL_VOLTAGE * 1000) / _val;

  EEPROM.write(0, (_val >> 8 & 0xFF)); //save new value, high byte
  EEPROM.write(1, (_val & 0xFF)); //save new value, low byte
}


