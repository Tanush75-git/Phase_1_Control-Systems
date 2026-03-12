#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Pin Definitions ---
#define RELAY_PIN 8      // Relay module controlling the pump (Red LED)
#define TRIG_PIN 10      // Ultrasonic sensor TRIG pin
#define ECHO_PIN 11      // Ultrasonic sensor ECHO pin
#define SWITCH_PIN 12    // Slide switch for Auto/Manual override
#define GREEN_LED 13     // System status indicator

// --- Tank Dimensions (in cm) ---
// Adjust these values based on your actual tank size
const int FULL_LEVEL = 10;   // Distance when tank is considered FULL (Pump turns OFF)
const int EMPTY_LEVEL = 90;  // Distance when tank is considered EMPTY (Pump turns ON)

// Initialize the LCD (I2C address is usually 0x27 or 0x3F for 16x2 screens)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Global variables to track state
bool pumpState = false;

void setup() {
  Serial.begin(9600);
  
  // Configure pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  // Use internal pull-up resistor since the switch connects to ground
  pinMode(SWITCH_PIN, INPUT_PULLUP); 

  // Make sure pump is initially off
  digitalWrite(RELAY_PIN, LOW); 

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Water Level");
  lcd.setCursor(0, 1);
  lcd.print("Controller");
  delay(2000);
  lcd.clear();
}

void loop() {
  // 1. Read the switch state (LOW = Auto Mode, HIGH = System Off/Manual)
  bool isAutoMode = (digitalRead(SWITCH_PIN) == LOW);
  
  // 2. Measure distance from ultrasonic sensor
  long distance = getDistance();
  
  // Calculate water level percentage (Optional, for display)
  // Constrain ensures the percentage stays between 0 and 100
  int levelPercentage = map(distance, EMPTY_LEVEL, FULL_LEVEL, 0, 100);
  levelPercentage = constrain(levelPercentage, 0, 100);

  // 3. Control Logic
  if (isAutoMode) {
    digitalWrite(GREEN_LED, HIGH); // Indicator that Auto Mode is ON
    
    // If water is low, turn pump ON
    if (distance >= EMPTY_LEVEL && !pumpState) {
      pumpState = true;
      digitalWrite(RELAY_PIN, HIGH); // Turn Relay ON
    }
    // If water is high, turn pump OFF
    else if (distance <= FULL_LEVEL && pumpState) {
      pumpState = false;
      digitalWrite(RELAY_PIN, LOW); // Turn Relay OFF
    }
  } else {
    // System turned off via switch
    digitalWrite(GREEN_LED, LOW);
    pumpState = false;
    digitalWrite(RELAY_PIN, LOW); 
  }

  // 4. Update LCD Display
  updateDisplay(distance, levelPercentage, isAutoMode);
  
  // Short delay to stabilize readings
  delay(500); 
}

// Function to calculate distance using HC-SR04
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  // Calculate distance in cm (Speed of sound is ~343m/s)
  long cm = duration * 0.034 / 2; 
  return cm;
}

// Function to handle LCD formatting
void updateDisplay(long dist, int percent, bool autoMode) {
  lcd.setCursor(0, 0);
  lcd.print("Lvl:");
  lcd.print(percent);
  lcd.print("%  D:");
  lcd.print(dist);
  lcd.print("cm   "); // Extra spaces to clear trailing characters

  lcd.setCursor(0, 1);
  if (autoMode) {
    lcd.print("Pump: ");
    lcd.print(pumpState ? "ON " : "OFF");
    lcd.print(" [AUTO]");
  } else {
    lcd.print("System OFF     ");
  }
}
