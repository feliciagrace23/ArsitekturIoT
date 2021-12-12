#ifdef ESP32
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
  #include <SPIFFS.h>
#else
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <FS.h>
#endif

const char* ssid = "CALVIN-Student";
const char* password = "CITStudentsOnly";

int UpperThreshold = 518;
int LowerThreshold = 490;
int reading = 0;
float BPM = 0.0;
int pinBuzzer = 2;
bool IgnoreReading = false;
bool FirstPulseDetected = false;
unsigned long FirstPulseTime = 0;
unsigned long SecondPulseTime = 0;
unsigned long PulseInterval = 0;

AsyncWebServer server(80);

String read_AD8232() {
  float temp = analogRead(34);
  if (temp < 100){
    digitalWrite(pinBuzzer,HIGH);
    delay(1500);
  }
  digitalWrite(pinBuzzer,LOW);
  
  if (isnan(temp)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
  //  Serial.println(temp);
    return String(temp);
  }
}


String read_BPM() {
   reading = analogRead(34); 
      // Heart beat leading edge detected.
      if(reading > UpperThreshold && IgnoreReading == false){
        if(FirstPulseDetected == false){
          FirstPulseTime = millis();
          FirstPulseDetected = true;
        }
        else{
          SecondPulseTime = millis();
          PulseInterval = SecondPulseTime - FirstPulseTime;
          FirstPulseTime = SecondPulseTime;
        }
        IgnoreReading = true;
      }

      // Heart beat trailing edge detected.
      if(reading < LowerThreshold){
        IgnoreReading = false;
      }  
      BPM = (1.0/PulseInterval) * 60.0 * 1000;
      delay(15);
      Serial.println(BPM);
      Serial.flush();
      return String(BPM);
      
}


void setup(){
  Serial.begin(115200);
  pinMode(pinBuzzer,OUTPUT);
  pinMode(18, INPUT); // Setup for leads off detection LO +
  pinMode(19, INPUT); // Setup for leads off detection LO -

  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/ecgvalue", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", read_AD8232().c_str());
  });
  server.on("/BPM", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", read_BPM().c_str());
  });
  server.begin();
}

void loop(){
  
}
