#include <OneWire.h>                
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

OneWire ourWire(2);
DallasTemperature sensors(&ourWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

float phval = 0.0;
unsigned long int avgval;
int buffer_arr[10], temp;

// Calibration variables
float calibration_offset = 25.88423;  // Your current offset
float slope = -5.70;                  // Your current slope

// Two-point calibration variables
float V1, V2;  // Voltage readings for pH 4.0 and pH 7.0
bool calibrated = false;
int calibration_step = 0;

void setup() {
  delay(1000);
  Serial.begin(9600);
  sensors.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(2, 0);
  lcd.print("  Micelio  ");
  lcd.setCursor(2, 1);
  lcd.print("  Botanico  ");
  delay(3000);
  lcd.clear();
  
  Serial.println("=== pH Sensor Two-Point Calibration ===");
  Serial.println("Commands:");
  Serial.println("1 - Start calibration with pH 4.0 solution");
  Serial.println("2 - Save pH 4.0 reading and move to pH 7.0");
  Serial.println("3 - Save pH 7.0 reading and calculate calibration");
  Serial.println("4 - Use current calibration (skip)");
  Serial.println("======================================");
}

float readVoltage() {
  // Read analog values
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(A0);
    delay(30);
  }

  // Bubble sort for filtering
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  avgval = 0;
  for (int i = 2; i < 8; i++) {
    avgval += buffer_arr[i];
  }

  return ((float)avgval * 5.0 / 1024) / 6;
}

void performCalibration() {
  float volt = readVoltage();
  
  switch(calibration_step) {
    case 1: // Waiting for pH 4.0 solution
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Put in pH 4.0");
      lcd.setCursor(0, 1);
      lcd.print("Volt:");
      lcd.print(volt, 3);
      break;
      
    case 2: // Save pH 4.0 and wait for pH 7.0
      V1 = volt;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("pH4.0 Saved!");
      lcd.setCursor(0, 1);
      lcd.print("Put in pH 7.0");
      delay(2000);
      break;
      
    case 3: // Save pH 7.0 and calculate
      V2 = volt;
      
      // Calculate slope and offset
      slope = (7.0 - 4.0) / (V2 - V1);
      calibration_offset = 4.0 - (slope * V1);
      calibrated = true;
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Calibration OK!");
      lcd.setCursor(0, 1);
      lcd.print("S:");
      lcd.print(slope, 2);
      lcd.print(" O:");
      lcd.print(calibration_offset, 2);
      
      Serial.println("=== CALIBRATION COMPLETE ===");
      Serial.print("pH 4.0 Voltage: "); Serial.println(V1, 4);
      Serial.print("pH 7.0 Voltage: "); Serial.println(V2, 4);
      Serial.print("Slope: "); Serial.println(slope, 4);
      Serial.print("Offset: "); Serial.println(calibration_offset, 4);
      Serial.println("===========================");
      delay(3000);
      break;
  }
}

void normalOperation() {
  sensors.requestTemperatures();
  float temp1 = sensors.getTempCByIndex(0);
  float volt = readVoltage();
  
  // Calculate pH with calibration
  float ph_act = (slope * volt) + calibration_offset;
  
  // Temperature compensation
  float pH_compensated = ph_act + ((25 - temp1) * 0.03);

  // Display results
  Serial.print("Temperature: ");
  Serial.print(temp1);
  Serial.print("C | Voltage: ");
  Serial.print(volt, 4);
  Serial.print("V | pH: ");
  Serial.print(ph_act, 2);
  Serial.print(" | pH Comp: ");
  Serial.println(pH_compensated, 2);

  // LCD Display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH:");
  lcd.print(pH_compensated, 2);
  
  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.print(temp1, 1);
  lcd.print("C");
  
  if (!calibrated) {
    lcd.setCursor(10, 1);
    lcd.print("UNCAL");
  }
}

void loop() {
  // Check for calibration commands
  if (Serial.available()) {
    char command = Serial.read();
    switch(command) {
      case '1':
        calibration_step = 1;
        Serial.println("Starting calibration... Put sensor in pH 4.0 solution");
        break;
      case '2':
        if (calibration_step == 1) {
          calibration_step = 2;
          Serial.println("pH 4.0 reading saved. Now put sensor in pH 7.0 solution");
        }
        break;
      case '3':
        if (calibration_step == 2) {
          calibration_step = 3;
          Serial.println("pH 7.0 reading saved. Calculating calibration...");
        }
        break;
      case '4':
        calibrated = true;
        calibration_step = 0;
        Serial.println("Using current calibration values");
        break;
    }
  }

  if (calibration_step > 0) {
    performCalibration();
  } else {
    normalOperation();
  }
  
  delay(1000);
}