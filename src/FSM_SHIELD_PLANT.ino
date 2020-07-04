#include <MQ7.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <U8x8lib.h>
#include<SPI.h>


#define DHTPIN            7  
#define DHTTYPE           DHT22     // DHT 22 (AM2302)

//MQ-7 Sensor Variables
float ppm;
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

//FSM Variables

int rst = 0;
int err = 0;
int errorCounter = 0;
bool hardError = 0;
enum State {INIT,START,MEASURE,ACTUATE,COMUNICATE,ERRORE,SAFE,END};
enum State Pstate,Nstate;

//SPI with ESP8266: MASTER
#define SS 53 //arancio
#define SCK 52 //verde
#define MOSI = 51 //giallo
#define MISO = 50 //rosso

uint8_t a;

void setup() {

  Pstate=START;
  
}

void loop() {

  
  if(rst == 1){
    Pstate = START;
  }else{
    switch(Pstate){
    case START : Serial.println("START"); Pstate=INIT; break;
    case INIT : Serial.println("INIT"); Init(); break;
    case MEASURE : Serial.println("MEASURE"); Measure(); break;
    case ACTUATE : Serial.println("ACTUATE"); Actuate(); break;
    case COMUNICATE : Serial.println("COMUNICATE"); Comunicate(); break;
    case ERRORE : Serial.println("ERRORE"); Error(); break;
    case SAFE : Serial.println("SAFE"); Safe(); break;
    case END : Serial.println("END"); End(); break;
   }

  }
}

void Init(){
  
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

  //SPI 
  pinMode(SS,OUTPUT);
  digitalWrite(SS,HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  Serial.println("SPI Initialized");
  
  
  delayMS = sensor.min_delay / 1000;

  //Monitoraggio potenza
  //se ok -> Pstate==MEASURE;
  // else -> Pstate==ERRORE;
  //Settaggi della comunicazione
  Pstate=MEASURE;
}

void Measure(){
  
  //DHT22 loop
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("ERROE reading temperature!");
    errorCounter+=1;
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
    errorCounter+=1;
  }
  else {
    Serial.print("Humidity: ");
    humid=event.relative_humidity;
    Serial.print(event.relative_humidity);
    Serial.println("%");
  }

  //MQ-7 loop
  ppm = mq7.getPPM();
  Serial.print  ("CO2: "); Serial.println(ppm);  // prints the value read

  //OLED DISPLAY LOOP
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0,0,"New Measurement");
  u8x8.drawString(0,1,"Temp. :"); u8x8.setCursor(8, 1); u8x8.print(temp);
  u8x8.drawString(0,2,"Humid. :"); u8x8.setCursor(8, 2); u8x8.print(humid);
  u8x8.drawString(0,3,"CO2 :"); u8x8.setCursor(8, 3); u8x8.print(ppm);

  //CHECK DI POTENZA CON MISURE;
  // se errori -> err=1;
  
  if(err==1 || errorCounter != 0){
    Pstate = ERRORE;
  }else{
    Pstate = ACTUATE;
  }
  delay(delayMS);
}

void Error(){
//CONTROLLO VARIABILI DI SISTEMA CON VALORI CRITICI e AGGIORNAMENTO ATTUATORI
  if(err==1){
    Pstate=SAFE;
  }else{
    if(errorCounter != 0){
      Serial.println("ERROR ON SENSORS");
    }
    Pstate=MEASURE;
    }
}

void Actuate(){
pinMode(LED_BUILTIN,OUTPUT);
digitalWrite(LED_BUILTIN,HIGH);
//COMUNICA CON INA via I2C
delay(100);
digitalWrite(LED_BUILTIN,LOW);
Pstate=COMUNICATE;
}

void Comunicate(){
  u8x8.drawString(0,4,"NEW DATA");

  //SPI MASTER mode, ESP SLAVE mode
  digitalWrite(SS,LOW);
  SPI.transfer('x');
  SPI.transfer('\n');
  digitalWrite(SS,HIGH);
  delay(1000);
  
  Pstate=MEASURE;
  //Questo stato porta a MEASURE oppure ERRORE  
}

void Safe(){
}

void End(){
}
