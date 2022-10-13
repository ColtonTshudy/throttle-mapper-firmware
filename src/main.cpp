/**
 * @file main.cpp
 *
 * @brief This is the main class for DynoControl, a firmware for Arduino to
 * control a 100k digital potentiometer to act as a surrogate throttle for BOLT
 *
 * @ingroup default
 *
 * @author Colton Tshudy [please add your names here!]
 *
 * @version 10/12/2022
 */

#include <Arduino.h>
#include <Application.h>
#include <HAL.h>
#include <X9C10X.h>

Application app;       // Application struct
X9C10X pot(POT_MAX_R); // Digital potentiometer

/** =================================================
 * Setup before loop
 */
void setup()
{
  // Initializes the pins
  InitializePins(); 

  // Begins UART communication
  Serial.begin(9600);

  // Constructs the application struct
  app = Application_construct();

  // Potentiometer initialization
  pot.begin(INC_PIN, UD_PIN, CS_PIN);
  pot.setPosition(0, true);

  pot.setPosition(49, 1);

  delay(100);
  sprintln("Max Ohms: ", pot.getMaxOhm(), " Ohms");
  sprintln("High side ohms: ", pot.getOhm(), " Ohms");
  sprintln("Low side ohms: ", pot.getMaxOhm() - pot.getOhm(), " Ohms");
}

/** =================================================
 * Primary loop
 */
void loop()
{
  // Should blink every second, if not, the Arduino is hung
  WatchdogLED(&app);

  // Primary loop for application
  Application_loop(&app);
}

/**
 * First time setup for the Application
 */
Application Application_construct()
{
  Application app;

  // Timer initialization
  app.watchdog_timer = SWTimer_construct(US_IN_SECONDS);
  app.pot_test_timer = SWTimer_construct(100000); // every 0.05 seconds

  return app;
}

/** =================================================
 * Primary functions for each loop of the application
 */
void Application_loop(Application *app_p)
{
  static uint32_t lastOhms = pot.getOhm();

  // reply only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:
    String input = Serial.readString();

    // say what you got:
    Serial.print("I received: " + input);
  }
  
}

/**
 * Sets up pin states
 */
void InitializePins()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(INC_PIN, OUTPUT);
  pinMode(UD_PIN, OUTPUT);
}

// Blinks an LED once a second as a visual indicator of processor hang
void WatchdogLED(Application *app_p)
{
  static bool state = true;

  if (SWTimer_expired(&app_p->watchdog_timer))
  {
    digitalWrite(LED_PIN, state);
    state = !state;
    SWTimer_start(&app_p->watchdog_timer);
  }
}

void sprintln(String pre, uint32_t val, String suf){
  String output = pre + val + suf;
  Serial.println(output);
}

/**
 * Cycles potentiometer between 0 and 99% for testing
*/
void potSweep(Application *app_p){
  static uint8_t count = 0;

  // For now, cycles between 0% and 99% throttle
  if (SWTimer_expired(&app_p->pot_test_timer))
  {
    pot.incr();
    Serial.println(pot.getOhm());
    SWTimer_start(&app_p->pot_test_timer);
    count++;
  }

  if (count == 99)
  {
    count = 1;
    pot.setPosition(0, true);
  }
}