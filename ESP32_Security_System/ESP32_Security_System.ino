/*
  ESP32 Smart Security System

  Features:
  - PIR motion detection
  - Keypad password unlock
  - LCD status display
  - Buzzer alarm
  - Blynk integration

  Made for learning IoT and embedded systems.
*/

#define BLYNK_TEMPLATE_ID "TMPL3EkDNmTuG"
#define BLYNK_TEMPLATE_NAME "ESP32 Security System"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// WiFi credentials
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definitions
#define PIR_PIN 13
#define BUZZER_PIN 12
#define LED_PIN 14
#define BUTTON_PIN 27

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {19, 18, 5, 17};
byte colPins[COLS] = {16, 4, 2, 15};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Password
String password = "1234";
String enteredPassword = "";

// System status
bool systemArmed = false;

BLYNK_WRITE(V0)
{
  int value = param.asInt();

  if(value == 1)
  {
    systemArmed = true;

    lcd.clear();
    lcd.print("System Armed");
  }
  else
  {
    systemArmed = false;

    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, LOW);

    lcd.clear();
    lcd.print("System Off");
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  lcd.print("Connecting...");

  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  lcd.clear();
  lcd.print("Ready");
}

void loop()
{
  Blynk.run();

  checkButton();
  checkMotion();
  checkKeypad();
}

void checkButton()
{
  static bool lastState = HIGH;

  bool currentState = digitalRead(BUTTON_PIN);

  if(lastState == HIGH && currentState == LOW)
  {
    systemArmed = !systemArmed;

    lcd.clear();

    if(systemArmed)
    {
      lcd.print("System Armed");
    }
    else
    {
      lcd.print("System Off");

      noTone(BUZZER_PIN);
      digitalWrite(LED_PIN, LOW);
    }

    delay(300);
  }

  lastState = currentState;
}

void checkMotion()
{
  if(systemArmed)
  {
    int motion = digitalRead(PIR_PIN);

    if(motion == HIGH)
    {
      lcd.clear();
      lcd.print("Motion Detected");

      digitalWrite(LED_PIN, HIGH);

      tone(BUZZER_PIN, 1000);

      Blynk.logEvent("motion_alert", "Motion detected");
    }
  }
}

void checkKeypad()
{
  char key = keypad.getKey();

  if(key)
  {
    if(key == '#')
    {
      if(enteredPassword == password)
      {
        systemArmed = false;

        noTone(BUZZER_PIN);
        digitalWrite(LED_PIN, LOW);

        lcd.clear();
        lcd.print("Access Granted");

        delay(1500);

        lcd.clear();
        lcd.print("System Off");
      }
      else
      {
        lcd.clear();
        lcd.print("Wrong Password");

        delay(1500);
      }

      enteredPassword = "";
    }
    else if(key == '*')
    {
      enteredPassword = "";

      lcd.clear();
    }
    else
    {
      enteredPassword += key;

      lcd.setCursor(0, 1);
      lcd.print("*");
    }
  }
}
