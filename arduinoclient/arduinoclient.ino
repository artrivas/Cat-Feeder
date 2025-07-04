#include <DHT.h>
#include <Servo.h>
#include <ArduinoJson.h>

#define DHTPIN 7
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

Servo myservo;

int pos = 0;
//Pins
const int servoPin = 4;
const int trigger1 = 2; // Dispenser
const int echo1 = 3; // Dispenser

const int trigger2 = 5; // Bowl
const int echo2 = 6; // Bowl
const int pirPin = 13;

//Time control
unsigned long dhtTime = 0;  
unsigned long servoTime = 0; 
unsigned long THRESHOLD = 1000;

bool encendio = false;

StaticJsonDocument<128> rxDoc;
StaticJsonDocument<64>  txDoc;

void setup()
{
  Serial.begin(9600);
  //dht.begin();
  myservo.attach(servoPin);
  Serial2.begin(115200);
  //pinMode(trigger1, OUTPUT);
  //pinMode(echo1, INPUT);
  //digitalWrite(trigger1, LOW);
  //pinMode(trigger2, OUTPUT);
  //pinMode(echo2, INPUT);
  //digitalWrite(trigger2, LOW);
  //pinMode(pirPin,INPUT);
  myservo.write(0);
}

void loop()
{
  if(Serial2.available()){
    String message = Serial2.readStringUntil('\n');
    DeserializationError error = deserializeJson(rxDoc, message);

    // Test if parsing succeeds.
    if (error) {
      Serial2.print(F("deserializeJson() failed: "));
      Serial2.println(error.f_str());
      return;
    }
    if(rxDoc["servo"] == true && !encendio){
      Serial.println("Feeding cats...");
      myservo.write(90);
      encendio = true;  
      servoTime = millis(); 
    }
  }
  if(encendio && millis()-servoTime > THRESHOLD){
    myservo.write(0);
    encendio = false;
    //Aca deberia enviar el mensaje al backend
    servoTime = millis();
  }
  long t1,d1,t2,d2;
  float h,t;
  if(millis() - dhtTime > 2000){
    //Lectura de temperatura analogica
    //h = dht.readHumidity();
    //t = dht.readTemperature();
    h = 124.32;
    t = 43.2;
    if(isnan(h) || isnan(t)){
      Serial2.println("Error obteniendo los datos del sensor DHT11");
      return;
    }
    dhtTime = millis();
  }
  /*int value = digitalRead(pirPin);
  
  digitalWrite(trigger1, HIGH);
  digitalWrite(trigger2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger1, LOW);
  digitalWrite(trigger2, LOW);
  
  t1 = pulseIn(echo1,HIGH);
  t2 = pulseIn(echo2,HIGH);
  */
  /*
  Testing*/
  int value = 2;
  t1 = 1000; 
  t2 = 2000;
  
  d1 = t1/59;
  d2 = t2/59;
  

  txDoc["humidity"] = h;
  txDoc["temperature"] = t;
  txDoc["pir"] = value;
  txDoc["d1"] = d1;
  txDoc["d2"] = d2;

  serializeJson(txDoc, Serial2);
  Serial2.println();

  Serial.print("Distance: \t");
  Serial.print(d1);
  Serial.println("cm");
  Serial.print(d2);
  Serial.println("cm");

  Serial.print("Temperature = ");
  Serial.print(t);
  Serial.print(" Degree Celsius\n");
  Serial.print("Humidity: \t");
  Serial.println(h);

  Serial.print("Pir: \t");
  Serial.println(value);

  
}
