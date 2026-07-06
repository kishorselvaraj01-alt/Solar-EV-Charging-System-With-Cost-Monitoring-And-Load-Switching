#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "BluetoothSerial.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);
BluetoothSerial SerialBT;
// Pins
#define VOLT_PIN 34
#define RELAY_SHOP 26 // IN1 (Shop Load)
#define RELAY_CHARGE 27 // IN2 (Charging)
// Calibration
float voltageFactor = 11.0;
// Battery limits
float minBatteryVolt = 3.0;
float maxBatteryVolt = 4.2;
float detectThreshold = 2.5;
// Cost settings
float pricePerPercent = 0.50;
bool vehiclePresent = false;
bool prevVehiclePresent = false;
bool sessionActive = false;
bool batteryFull = false;
float initialPercent = 0;
float batteryPercent = 0;
38
float chargingPercent = 0;
float finalCost = 0;
// New Variables
float totalCost = 0;
int totalSessions = 0;
String systemStatus = "GRID";
int anim = 0;
void setup()
{
 Serial.begin(115200);
 pinMode(RELAY_CHARGE, OUTPUT);
 pinMode(RELAY_SHOP, OUTPUT);
 lcd.init();
 lcd.backlight();
 analogReadResolution(12);
 // Bluetooth Start
 SerialBT.begin("EV_CHARGER");
 // Default → Shop load ON
 digitalWrite(RELAY_CHARGE, LOW);
 digitalWrite(RELAY_SHOP, HIGH);
}
void loop()
{
 // ===== READ VOLTAGE =====
 int adc = analogRead(VOLT_PIN);
39
 float sensedVolt = (adc / 4095.0) * 3.3;
 float batteryVolt = sensedVolt * voltageFactor;
 // ===== BATTERY % =====
 batteryPercent =
 ((batteryVolt - minBatteryVolt) /
 (maxBatteryVolt - minBatteryVolt)) * 100.0;
 batteryPercent = constrain(batteryPercent, 0, 100);
 // ===== VEHICLE DETECT =====
 if (batteryVolt > detectThreshold)
 vehiclePresent = true;
 else
 vehiclePresent = false;
 // ============================================
 // BATTERY CONNECTED
 // ============================================
 if (vehiclePresent && !prevVehiclePresent)
 {
 initialPercent = batteryPercent;
 sessionActive = true;
 batteryFull = false;
 systemStatus = "EV CHARGING";
 digitalWrite(RELAY_CHARGE, HIGH);
 digitalWrite(RELAY_SHOP, LOW);
 }
 // ============================================
 // BATTERY FULL
 // ============================================
 if (batteryPercent >= 100 && sessionActive)
40
 {
 batteryFull = true;
 chargingPercent = batteryPercent - initialPercent;
 finalCost = chargingPercent * pricePerPercent;
 totalCost += finalCost;
 totalSessions++;
 systemStatus = "GRID";
 digitalWrite(RELAY_CHARGE, LOW);
 digitalWrite(RELAY_SHOP, HIGH);
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("Battery Full");
 lcd.setCursor(0, 1);
 lcd.print("Cost Rs:");
 lcd.print(finalCost, 2);
 delay(4000);
 sessionActive = false;
 }
 // ============================================
 // BATTERY REMOVED
 // ============================================
 if (!vehiclePresent && prevVehiclePresent && sessionActive)
 {
 chargingPercent = batteryPercent - initialPercent;
 if (chargingPercent < 0) chargingPercent = 0;
41
 finalCost = chargingPercent * pricePerPercent;
 totalCost += finalCost;
 totalSessions++;
 systemStatus = "GRID";
 digitalWrite(RELAY_CHARGE, LOW);
 digitalWrite(RELAY_SHOP, HIGH);
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("Charged:");
 lcd.print((int)chargingPercent);
 lcd.print("%");
 lcd.setCursor(0, 1);
 lcd.print("Cost Rs:");
 lcd.print(finalCost, 2);
 delay(5000);
 sessionActive = false;
 }
 // ============================================
 // NORMAL DISPLAY
 // ============================================
 lcd.clear();
 // WAITING
 if (!vehiclePresent && !sessionActive)
 {
42
 systemStatus = "GRID";
 digitalWrite(RELAY_CHARGE, LOW);
 digitalWrite(RELAY_SHOP, HIGH);
 lcd.setCursor(0, 0);
 lcd.print("EV CHARGING");
 lcd.setCursor(0, 1);
 lcd.print("Waiting...");
 }
 // CHARGING
 else if (vehiclePresent && !batteryFull)
 {
 chargingPercent = batteryPercent - initialPercent;
 if (chargingPercent < 0) chargingPercent = 0;
 lcd.setCursor(0, 0);
 lcd.print("Battery:");
 lcd.print((int)batteryPercent);
 lcd.print("%");
 lcd.setCursor(0, 1);
 lcd.print("Charging");
 if (anim == 0) lcd.print(".");
 if (anim == 1) lcd.print("..");
 if (anim == 2) lcd.print("...");
 if (anim == 3) lcd.print("....");
 anim++;
 if (anim > 3) anim = 0;
 }
43
 prevVehiclePresent = vehiclePresent;
 // ============================================
 // SEND DATA TO PHONE
 // ============================================
 SerialBT.println("------EV CHARGER------");
 SerialBT.print("Total EV Charged: ");
 SerialBT.println(totalSessions);
 SerialBT.print("Total Cost: Rs ");
 SerialBT.println(totalCost, 2);
 SerialBT.print("Status: ");
 SerialBT.println(systemStatus);
 SerialBT.print("Battery: ");
 SerialBT.print((int)batteryPercent);
 SerialBT.println("%");
 SerialBT.print("Charging: ");
 SerialBT.print((int)chargingPercent);
 SerialBT.println("%");
 SerialBT.print("Cost: Rs ");
 SerialBT.println(finalCost, 2);
 SerialBT.println("----------------------");
 delay(1000);
}