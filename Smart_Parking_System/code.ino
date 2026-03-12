
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Initialize the I2C LCD (Standard Tinkercad address is 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gateServo;

// Pin Definitions based exactly on image_8ebe8b.png
const int trigPins[3]  = {13, 11, 9};
const int echoPins[3]  = {12, 10, 8};
const int redPins[3]   = {6, 4, 2};
const int greenPins[3] = {5, 3, 1}; // Pin 1 used here!
const int servoPin     = 7; 

// Logic Variables
const int distanceThreshold = 50; // cm to detect a car
bool isOccupied[3] = {false, false, false};
unsigned long entryTime[3] = {0, 0, 0}; 
int freeSlots = 3;

// Variables for LCD Screen Toggling
unsigned long lastScreenUpdate = 0;
int currentScreen = 0;

void setup() {
  // NOTE: Serial.begin() is OMITTED so Pin 1 (Green LED 3) functions properly!
  
  lcd.init();
  lcd.backlight();
  
  gateServo.attach(servoPin);
  gateServo.write(90); // Start with gate OPEN

  // Initialize Sensor & LED pins
  for(int i = 0; i < 3; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
    pinMode(redPins[i], OUTPUT);
    pinMode(greenPins[i], OUTPUT);
  }
}

// Function to calculate distance (with a 30ms timeout to prevent lag)
long getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  long duration = pulseIn(echo, HIGH, 30000); 
  if (duration == 0) return 999; // If no echo, assume it's far away
  return duration * 0.034 / 2;
}

void loop() {
  freeSlots = 0; // Reset count every loop
  unsigned long currentMillis = millis();

  // 1. Check all 3 slots
  for(int i = 0; i < 3; i++) {
    long dist = getDistance(trigPins[i], echoPins[i]);
    
    // If distance is less than threshold = OCCUPIED
    if (dist > 0 && dist < distanceThreshold) {
      digitalWrite(redPins[i], HIGH);
      digitalWrite(greenPins[i], LOW);
      
      if (!isOccupied[i]) {
        // Car JUST arrived
        isOccupied[i] = true;
        entryTime[i] = currentMillis; // Start the clock
      }
    } 
    // If distance is large = FREE
    else {
      digitalWrite(redPins[i], LOW);
      digitalWrite(greenPins[i], HIGH);
      freeSlots++;
      
      if (isOccupied[i]) {
         // Car JUST left
         isOccupied[i] = false;
         entryTime[i] = 0; // Reset the clock
      }
    }
  }

  // 2. Control Gate Servo
  if (freeSlots > 0) {
    gateServo.write(90); // Open the gate
  } else {
    gateServo.write(0);  // Close the gate
  }

  // 3. LCD Screen Toggling Logic (Switches every 2.5 seconds)
  // Because the LCD is small, we cycle through 3 different screens
  if (currentMillis - lastScreenUpdate >= 2500) {
    lastScreenUpdate = currentMillis;
    currentScreen = (currentScreen + 1) % 3; 
    lcd.clear();
  }

  // Draw the current screen
  if (currentScreen == 0) {
    // SCREEN 0: General Status
    lcd.setCursor(0, 0);
    lcd.print("Free Slots: ");
    lcd.print(freeSlots);
    lcd.setCursor(0, 1);
    lcd.print("Gate: ");
    lcd.print(freeSlots > 0 ? "OPEN  " : "CLOSED");
  } 
  else if (currentScreen == 1) {
    // SCREEN 1: Slot 1 & 2 Fare Tracking
    printSlotData(0, 0); // Print Slot 1 on top line
    printSlotData(1, 1); // Print Slot 2 on bottom line
  }
  else if (currentScreen == 2) {
    // SCREEN 2: Slot 3 Fare Tracking & Gate Status
    printSlotData(2, 0); // Print Slot 3 on top line
    lcd.setCursor(0, 1);
    lcd.print("Gate: ");
    lcd.print(freeSlots > 0 ? "OPEN  " : "CLOSED");
  }
  
  delay(50); // Small delay for stability
}

// Helper function to calculate and print time/fare
void printSlotData(int slotIndex, int lineNumber) {
  lcd.setCursor(0, lineNumber);
  lcd.print("S");
  lcd.print(slotIndex + 1);
  lcd.print(":");
  
  if (isOccupied[slotIndex]) {
    // Calculate minutes passed. (60000 ms = 1 minute)
    unsigned long elapsedMillis = millis() - entryTime[slotIndex];
    unsigned long minutesParked = elapsedMillis / 60000;
    unsigned long fare = minutesParked * 10; // 10 rupees per minute
    
    lcd.print(minutesParked);
    lcd.print("m Rs");
    lcd.print(fare);
  } else {
    lcd.print("FREE");
  }
}
