#include <OneWire.h>                
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

OneWire ourWire(2);                //Se establece el pin 2  como bus OneWire
DallasTemperature sensors(&ourWire); //Se declara una variable u objeto para nuestro sensor

LiquidCrystal_I2C lcd(0x27, 16, 2);
float phval = 0.0;
unsigned long int avgval;
int buffer_arr[10], temp;

//proceso de calibracon de ph
//Correction = Actual pH - Current reading
float Correction = 3.86 - (22.2);
float calibration_value = 21.34 + (Correction);

void setup() {
delay(1000);
Serial.begin(9600);
sensors.begin();   //Se inicia el sensor

 lcd.init();
 lcd.begin(16, 2);
 lcd.backlight();
 lcd.setCursor(2, 0);
 lcd.print("  Micelio  ");
 lcd.setCursor(2, 1);
 lcd.print("  Botanico  ");
 delay(3000);
 lcd.clear();

}
 
void loop() {
sensors.requestTemperatures();   //Se envía el comando para leer la temperatura
float temp1= sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC


for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(A0);
    delay(30);
  }

  // Ordenamiento de burbuja para filtrar ruido
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  avgval = 0.0;
  for (int i = 2; i < 8; i++) {
    avgval += buffer_arr[i];
  }

  float volt = ((float)avgval * 5.0 / 1024) / 6;
  float temp_compensation = temp1; // Adjust this based on your sensor specs
  float ph_act = -5.70 * volt + calibration_value + temp_compensation;



  ////---
Serial.print("Temperatura= ");
Serial.print(temp1);
Serial.println(" C");
Serial.println("----------");
Serial.print("Ph Sensor nigga= ");
Serial.print(ph_act);
Serial.println("----------");
//----

  lcd.setCursor(4, 0);
  lcd.print("pH: ");
  lcd.setCursor(7, 0);
  lcd.print(ph_act);
  lcd.setCursor(5
, 1);
  lcd.print("T: ");
  lcd.setCursor(7, 1);
  lcd.print(temp1);
  lcd.setCursor(11, 1);
  lcd.print("C");
  delay(1000);

}
