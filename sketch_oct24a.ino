#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <DHT.h>
#include <time.h>

// Config Firebase
#define FIREBASE_HOST "sensorsdatabase.firebaseio.com"
#define FIREBASE_AUTH "7EWhWQ2OJGYD5o6Qzb8ei6XMIaIajEFoFUoVbFDl"

// Config connect WiFi
#define WIFI_SSID "Fuck OFF"
#define WIFI_PASSWORD "AAAAAAAA"

// Config DHT
#define DHTPIN D1
#define DHTTYPE DHT22

// LED Debug
#define DEBUG_WIFICONNECT 14
#define DEBUG_PUTHDATA 5

// Config time
int timezone = 7;
char ntp_server1[20] = "ntp.ku.ac.th";
char ntp_server2[20] = "fw.eng.ku.ac.th";
char ntp_server3[20] = "time.uni.net.th";

// Variables
int dst = 0;
const int pingPin = D2;
int inPin = D3;
String now = "";

// Variables Device data
String deviceName = "Device1";
String Location = "132,321";

DHT dht(DHTPIN, DHTTYPE);


void setup() {
  pinMode(DEBUG_WIFICONNECT, OUTPUT);
  pinMode(DEBUG_PUTHDATA, OUTPUT);

  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(DEBUG_WIFICONNECT, !digitalRead(DEBUG_WIFICONNECT));
    delay(500);
  }
  digitalWrite(DEBUG_WIFICONNECT, HIGH);
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
  Serial.println("Waiting for time");
  while (!time(nullptr)) {
    Serial.println();
    delay(500);
  }
  Serial.println();
  Serial.println("Now: " + NowString());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Serial.begin(9600);
}

void loop() {
  long duration, cm;
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int stat;
     
      pinMode(pingPin, OUTPUT);
      digitalWrite(pingPin, LOW);
      delayMicroseconds(2);
      digitalWrite(pingPin, HIGH);
      delayMicroseconds(5);
      digitalWrite(pingPin, LOW);
      pinMode(inPin, INPUT);
      duration = pulseIn(inPin, HIGH);
      
  //Change Distance to Status
      cm = microsecondsToCentimeters(duration);
      if(cm < 10){
        stat = 1;
      }else{
        stat = 0;
      }

  //Check Variable to Serial Monitor
      Serial.println("--------------------------------");
      Serial.println("Get Data");
      Serial.println("--------------------------------");
      Serial.print("Distance : ");
      Serial.print(cm);
      Serial.println(" cm");
      Serial.print("Status : ");
      Serial.println(stat);
      Serial.print("Temperature : ");
      Serial.println(t);
      Serial.print("Humidity : ");
      Serial.println(h);
      Serial.print("Time : ");
      Serial.println(NowString());
      Serial.println("--------------------------------");     
      delay(1000);

  //JSON Things
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& logs = jsonBuffer.createObject();
      JsonObject& logsdht = jsonBuffer.createObject();

      logs["Status"] = stat;
      logs["UP-Time"] = NowString();

      logsdht["Temperature"] = t;
      logsdht["Humidity"] = h;
      logsdht["UP-Time"] = NowString();

  digitalWrite(DEBUG_PUTHDATA, HIGH);

  //Get data from Firebase
      FirebaseObject data = Firebase.get(deviceName);
      float StatusDB = data.getFloat("/Status");
      Serial.print("Old Status : ");
      Serial.println(StatusDB);
      Serial.print("New Status : ");
      Serial.println(stat);
  
  // append a new value to /...  
      if(StatusDB != stat){
        Serial.println("WARNING! - Status has changed AED not Avialable");
        Firebase.push(deviceName+"/LOG", logs);
        Serial.println("pushed: /"+deviceName+"/LOG/Status");
        Firebase.set(deviceName+"/Temperature", t);
        Firebase.set(deviceName+"/Humidity", h);
        Firebase.set(deviceName+"/Status", stat);
      }
      else if(t > 50 || h > 100){
        Serial.println("WARNING! - Temperature or Humidity was not in the aspect. Please check the Cabinet");
        Firebase.push(deviceName+"/LOG", logsdht);
        Serial.println("pushed: /"+deviceName+"/LOG/Temp-Humi");
        Firebase.set(deviceName+"/Temperature", t);
        Firebase.set(deviceName+"/Humidity", h);
        Firebase.set(deviceName+"/Status", stat);
      }
      else{
        Firebase.set(deviceName+"/Temperature", t);
        Firebase.set(deviceName+"/Humidity", h);
        Firebase.set(deviceName+"/Status", stat);
      }
      
  // handle error
      if (Firebase.failed()) {
          Serial.print("pushing /"+deviceName+" failed:");
          Serial.println(Firebase.error());  
          return;
      }

  //Serial Monitor LOG
      Serial.println("--------------------------------");
      Serial.println("Set Data");
      Serial.println("--------------------------------");
      Serial.println("Set: /"+deviceName+"/Distance");
      Serial.println("Set: /"+deviceName+"/Status");
      Serial.println("Set: /"+deviceName+"/Temperature");
      Serial.println("Set: /"+deviceName+"/Humidity");
      Serial.println();
      Serial.println("Successful!!!");
      Serial.println("--------------------------------");
      Serial.println();
      Serial.println();
      delay(2000);
      digitalWrite(DEBUG_PUTHDATA, LOW);
      delay(5000);
 }

String NowString() {
  time_t now = time(nullptr);
  struct tm* newtime = localtime(&now);

  String tmpNow = "";
  tmpNow += String(newtime->tm_hour);
  tmpNow += ":";
  tmpNow += String(newtime->tm_min);
  tmpNow += ":";
  tmpNow += String(newtime->tm_sec);
  return tmpNow;
}

long microsecondsToCentimeters(long microseconds)
    {
      // The speed of sound is 340 m/s or 29 microseconds per centimeter.
      // The ping travels out and back, so to find the distance of the
      // object we take half of the distance travelled.
      return microseconds / 29 / 2;
    }
