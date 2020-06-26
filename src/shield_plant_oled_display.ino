//Include CO2  (MQ-7) and Temperature e Humidity (DHT22)

#include <MQ7.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <U8x8lib.h>

#define DHTPIN            7  
#define DHTTYPE           DHT22     // DHT 22 (AM2302)

//MQ-7 Sensor Variables
float sensorValue;
int Aread;
MQ7 mq7(A4, 5.0);

//DHT22 Variables
DHT_Unified dht(DHTPIN, DHTTYPE);
float temp;
float humid;
uint32_t delayMS;

//OLED 1.3'' I2C DISPLAY Variables
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
float x;
String mystr;

void setup()
{
  Serial.begin(9600);      // sets the serial port to 9600
   
  // DHT22.
  dht.begin();
  Serial.println("DHTxx Unified Sensor Example");
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
  // Set delay between sensor readings based on sensor details.

  //OLED DISPLAY
  u8x8.begin();
  u8x8.setPowerSave(0);
  
  delayMS = sensor.min_delay / 1000;
}

void loop()
{
  //MQ-7 loop
  float ppm = mq7.getPPM();
    // Delay between measurements.
  delay(delayMS);

  //DHT22 loop
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    temp=event.temperature;
    Serial.print(event.temperature);
    Serial.println(" *C");
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    humid=event.relative_humidity;
    Serial.print(event.relative_humidity);
    Serial.println("%");
  }

  Serial.print  ("CO2: "); Serial.println(ppm);  // prints the value read

  //OLED DISPLAY LOOP
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0,0,"Air Shield");
  u8x8.drawString(0,1,"Temp. :"); u8x8.setCursor(8, 1); u8x8.print(temp);
  u8x8.drawString(0,2,"Humid. :"); u8x8.setCursor(8, 2); u8x8.print(humid);
  u8x8.drawString(0,3,"CO2 :"); u8x8.setCursor(8, 3); u8x8.print(ppm);  
}
