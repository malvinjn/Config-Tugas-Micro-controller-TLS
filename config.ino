#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// Pin konek
#define SERVO_PIN 15
#define LED_GREEN 5  
#define LED_RED 22
#define BUZZER_PIN 13
#define I2C_SDA 2
#define I2C_SCL 4

// set OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {12, 14, 27, 26}; 
byte colPins[COLS] = {25, 33, 32, 35}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

Servo doorServo;

// setting akese
String secretPIN = "1234"; 
String inputPIN = "";
int attemptCount = 0;
bool isLockedOut = false;

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  doorServo.attach(SERVO_PIN);
  doorServo.write(0); 

  Wire.begin(I2C_SDA, I2C_SCL);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); // Berhenti jika OLED gagal terdeteksi
  }
  display.setTextColor(WHITE);
  
  showStandbyScreen();
}

void loop() {
  if (isLockedOut) return; 

  char key = keypad.getKey();
  if (key) {
    if (key == '#') { 
      checkPIN();
    } else if (key == '*') { 
      inputPIN = "";
      showStandbyScreen();
    } else {
      inputPIN += key;
      
      // Update tampilan saat mengetik
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Masukkan PIN:");
      display.setCursor(0, 16);
      display.setTextSize(2);
      display.print(inputPIN);
      display.display();
      display.setTextSize(1);
    }
  }
}

void checkPIN() {
  if (inputPIN == secretPIN) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Akses Diterima");
    display.display();
    digitalWrite(LED_GREEN, HIGH);
    
    doorServo.write(90); 
    delay(3000);         
    doorServo.write(0);  
    
    digitalWrite(LED_GREEN, LOW);
    attemptCount = 0;    
    inputPIN = "";
    showStandbyScreen();
    
  } else {
    attemptCount++;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("PIN Salah!");
    display.display();
    digitalWrite(LED_RED, HIGH);
    
    tone(BUZZER_PIN, 500); 
    delay(1000);
    noTone(BUZZER_PIN);
    digitalWrite(LED_RED, LOW);
    
    if (attemptCount >= 3) {
      triggerAlarm();
    } else {
      inputPIN = "";
      showStandbyScreen();
    }
  }
}

void triggerAlarm() {
  isLockedOut = true;
  unsigned long alarmStartTime = millis();
  unsigned long lastBuzzerToggle = 0;
  bool buzzerState = false;
  char lastKey = 0;
  
  keypad.setHoldTime(3000); 
  
  while (millis() - alarmStartTime < 10000) { 
    int timeLeft = 10 - ((millis() - alarmStartTime) / 1000);
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("ALARM AKTIF!");
    display.setCursor(0, 16);
    display.print("Sisa: ");
    display.print(timeLeft);
    display.print("s");
    display.setCursor(0, 32);
    display.print("(Tahan * untuk Stop)");
    display.display();
    
    if (millis() - lastBuzzerToggle >= 200) {
      lastBuzzerToggle = millis();
      buzzerState = !buzzerState;
      if (buzzerState) {
        tone(BUZZER_PIN, 1000);
        digitalWrite(LED_RED, HIGH);
      } else {
        noTone(BUZZER_PIN);
        digitalWrite(LED_RED, LOW);
      }
    }
    
    char key = keypad.getKey(); 
    if (key) {
      lastKey = key; 
    }
    
    if (keypad.getState() == HOLD && lastKey == '*') {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Alarm Dihentikan");
      display.display();
      delay(1500);
      break; 
    }
  }
  
  noTone(BUZZER_PIN);
  digitalWrite(LED_RED, LOW);
  keypad.setHoldTime(500); 
  
  attemptCount = 0; 
  inputPIN = "";
  isLockedOut = false;
  showStandbyScreen();
}

void showStandbyScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Masukkan PIN:");
  display.display();
}
