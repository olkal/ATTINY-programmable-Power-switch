/*
   ATTINY fuse settings:
   1MHz (internal)
   BOD 2.7V (BOD 1.8V can be used for ATTINY85V) 
*/
/*
  Instructions to auto calibrate internal voltmeter:
    1. Disconnect load side of the switch
    2. Press and hold button
    3. Connect 5.00V power supply on battery side
    4. Release button after the second LED blink (10 seconds)
    5. LED will start to blink slowly when calibration is done
    6. Disconnect and reconnect adjustable power supply
    7. Test the calibration by adjusting the supply voltage while observing LED

  Instructions to configure the battery cell count:
    1. Disconnect load side of the switch
    2. Press and hold button
    3. Connect the battery
    4. Release button after the first LED blink (5 seconds)
    5. LED will start to blink the number of cells detected
    6. Disconnect and reconnect battery    
*/

//configue mcu sleep and low battery warning/power off levels:
#define SLEEP_ENABLE                1       //Enable mcu deep sleep when power off (off-state, low power consumption) 
#define BATTERY_LOW_AUTO_OFF        0       //Enable auto power off if voltage is less than CELL_VOLTAGE_LOW 
#define BATTERY_VERY_LOW_AUTO_OFF   1       //Enable auto power off if voltage is less than CELL_VOLTAGE_VERY_LOW
#define BATTERY_EMPTY_AUTO_OFF      1       //Enable imidiate power off if voltage is less than CELL_VOLTAGE_EMPTY
#define AUTO_CAL_VOLTAGE            500     //supplied voltage to VIN when performing auto calibration, unit 1/100 volt
#define CELL_VOLTAGE_EMPTY          250     //bat level - imidiate shut off if voltage is below, unit 1/100 volt
#define CELL_VOLTAGE_VERY_LOW       300     //bat level - controlled shut off if voltage is below, unit 1/100 volt
#define CELL_VOLTAGE_LOW            360     //bat level - warning if voltage is below, unit 1/100 volt

//configure button behaviour:
#define BUTTON_PWR_ON_HOLD_TIME     0       //press and hold button to turn power on (ms) 0=disable
#define BUTTON_PWR_OFF_HOLD_TIME    0       //press and hold button to turn power off (ms) 0=disable

//configure LED
#define LED_OFF_BRIGHTNESS          0       //max 255
#define LED_ON_BRIGHTNESS           127     //max 255
#define LED_BLINK_ON_TIME           500     //low battery blink on time (ms)
#define LED_BLINK_OFF_TIME          500     //low battery blink off time (ms)
#define LED_BLINK_FAST_ON_TIME      40
#define LED_BLINK_FAST_OFF_TIME     60

//configure safe shut down of Raspberry Pi before power off:
#define AUX_IS_SHTD_SIGNAL_OUT      1       //enable aux pin as signal for safe shut down of Raspberry Pi or similar  
#define AUX_SHTD_SIGNAL_HIGH_LOW    0       //0=shtd signal pull low, 1=shtd signal pull high (1 can be combined with a LED)
#define AUX_SHTD_SIGNAL_LENGTH      500     //signal to Raspberry duration (ms)
#define AUX_SHTD_DELAY              14000    //delay, from Raspberry Pi shut down is initiated until power off (ms)

//configure timer - power off after xx time has elapsed
#define TIMER_PWR_OFF_DELAY         0   //timer delay from power on until power off (ms), 0=disable






