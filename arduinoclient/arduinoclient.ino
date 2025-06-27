#include <DHT.h>
#include <Servo.h>
#include <ArduinoJson.h>

#define DHTPIN 7
#define DHTTYPE DHT11

#define TXD1 19
#define RXD1 21

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
  Serial2.begin(9600);
  dht.begin();
  myservo.attach(servoPin);
  Serial.begin(115200);
  pinMode(trigger1, OUTPUT);
  pinMode(echo1, INPUT);
  digitalWrite(trigger1, LOW);
  pinMode(trigger2, OUTPUT);
  pinMode(echo2, INPUT);
  digitalWrite(trigger2, LOW);
  pinMode(pirPin,INPUT);
}

void loop()
{
  if(Serial.available()){
    String message = Serial.readStringUntil('\n');
    DeserializationError error = deserializeJson(rxDoc, message);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    if(rxDoc["servo"] == true && !encendio){
      Serial2.println("Feeding cats...");
      myservo.write(90);
      encendio = true;  
      servoTime = millis(); 
    }
  }
  if(encendio && millis()-servoTime > THRESHOLD){
    myservo.write(0);
    encendio = false;
    //Aca deberia enviar el mensaje al backend
  }
  long t1,d1,t2,d2;
  float h,t;
  if(millis() - dhtTime > 2000){
    //Lectura de temperatura analogica
    h = dht.readHumidity();
    t = dht.readTemperature();
    if(isnan(h) || isnan(t)){
      Serial.println("Error obteniendo los datos del sensor DHT11");
      return;
    }
    dhtTime = millis();
  }
  int value = digitalRead(pirPin);
  digitalWrite(trigger1, HIGH);
  digitalWrite(trigger2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger1, LOW);
  digitalWrite(trigger2, LOW);

  t1 = pulseIn(echo1,HIGH);
  t2 = pulseIn(echo2,HIGH);
  d1 = t1/59;
  d2 = t2/59;

  txDoc["humidity"] = h;
  txDoc["temperature"] = t;
  txDoc["pir"] = value;
  txDoc["d1"] = d1;
  txDoc["d2"] = d2;

  serializeJson(txDoc, Serial);
  Serial.println();

  Serial2.print("Distance: \t");
  Serial2.print(d1);
  Serial2.println("cm");
  Serial2.print(d2);
  Serial2.println("cm");

  Serial2.print("Temperature = ");
  Serial2.print(t);
  Serial2.print(" Degree Celsius\n");
  Serial2.print("Humidity: \t");
  Serial2.println(h);

  Serial2.print("Pir: \t");
  Serial2.println(value);

  
}
