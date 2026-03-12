#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Configuration ---
const String password = "1122";
String input_password = "";

// --- Servo and LCD Setup ---
Servo myServo;
// 0x27 is the standard address for most I2C LCD modules
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// --- Keypad Mapping ---
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// These pins match your green wires in the diagram
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  myServo.attach(10);        // Servo connected to Pin 10
  myServo.write(0);          // Start in locked position (0 degrees)
  
  lcd.init();
  lcd.backlight();
  lcd.print("Enter Password:");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key == '#') {        // Use '#' to clear or reset if you make a mistake
      input_password = "";
      lcd.clear();
      lcd.print("Enter Password:");
    } 
    else {
      input_password += key;
      lcd.setCursor(input_password.length() - 1, 1);
      lcd.print('*');        // Print stars for privacy
    }

    // Check if password length matches the requirement
    if (input_password.length() == password.length()) {
      lcd.clear();
      
      if (input_password == password) {
        lcd.print("ACCESS GRANTED");
        myServo.write(90);   // Rotate servo to unlock (90 degrees)
        delay(3000);         // Wait 3 seconds
        myServo.write(0);    // Rotate back to lock
      } else {
        lcd.print("WRONG PASSWORD");
        delay(2000);
      }
      
      // Reset for next attempt
      input_password = "";
      lcd.clear();
      lcd.print("Enter Password:");
    }
  }
}
