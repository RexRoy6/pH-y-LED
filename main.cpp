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

// YOUR CALCULATED CALIBRATION VALUES
float calibration_offset = 25.88423;  // Your calculated offset
float slope = -5.70;  // Keep the same slope for now

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
}

void loop() {
  sensors.requestTemperatures();
  float temp1 = sensors.getTempCByIndex(0);

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

  float volt = ((float)avgval * 5.0 / 1024) / 6;
  
  // Calculate pH with your calibration
  float ph_act = (slope * volt) + calibration_offset;
  
  // Temperature compensation
  float pH_compensated = ph_act + ((25 - temp1) * 0.03);

  // Display results
  Serial.print("Temperatura= ");
  Serial.print(temp1);
  Serial.println(" C");
  Serial.print("Voltage: ");
  Serial.println(volt, 4);
  Serial.print("pH Raw: ");
  Serial.println(ph_act, 2);
  Serial.print("pH Compensated: ");
  Serial.println(pH_compensated, 2);
  Serial.println("----------");

  // LCD Display - show compensated pH value
  lcd.setCursor(0, 0);
  lcd.print("pH:");
  lcd.print(pH_compensated, 2);  // Show  decimal places
  lcd.print("  ");
  
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temp1, 1);
  lcd.print("C ");
  
  delay(1000);
}