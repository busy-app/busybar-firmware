#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

void setup(void)  {
  Serial.begin(921600);
  
  if(!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while(1) { delay(10); }
  }

  Serial.println("ina begin");
}

void loop(void) {
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
//  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
//  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
//  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
//  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
//  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
//  Serial.println("");

  Serial.print("t="); Serial.print(millis());
  Serial.print(" bv="); Serial.print((int)(busvoltage * 1000000.0f));
  Serial.print(" lv="); Serial.print((int)(loadvoltage * 1000000.0f));
  Serial.print(" i="); Serial.print((int)(current_mA * 1000.0f));
  Serial.println();
}
