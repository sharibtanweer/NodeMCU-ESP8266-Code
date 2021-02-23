#include <ESP8266WiFi.h>  
#include <ESP8266HTTPClient.h>  
#include <WiFiClientSecureBearSSL.h>  
#include <ArduinoJson.h>
#include <EEPROM.h>

void writeString(char add,String data);
String read_String(char add);

int LED = 2;
String ssid = "admin";
String password = "12345678";
int EE_ADDR = 0;
String ssidpass = "";

void setup() {  
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(LED, OUTPUT);
  pinMode(D3, OUTPUT);
  if(!ssidpass.isEmpty()){
    ssidpass = read_String(16);
    Serial.println(ssidpass);
    Serial.println("RE: "+ssidpass);
    String savedssid = getValue(ssidpass, ',', 0);
    String savedpass = getValue(ssidpass, ',', 1);
    WiFi.begin(savedssid, savedpass);
  }else{
    Serial.println("RE: "+ssid+"::"+password);
    WiFi.begin(ssid, password);
  }
  while (WiFi.status() != WL_CONNECTED) { 
    delay(100);  
  } 
}  

void TestHttpsAPI(){  
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);  
  client->setInsecure();  
  HTTPClient https; 
  if (https.begin(*client, "<<YOUR API>>?key=ACCONT0003")) {
    int httpCode = https.GET(); 
    if (httpCode > 0) { 
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {  
        Serial.println(String("[HTTPS] Received: ") + https.getString());  
        StaticJsonBuffer<512> buff;
        JsonObject& root = buff.parseObject(https.getString());
        ssid = root["NETWORK"].as<String>();
        password = root["PASSWORD"].as<String>();
        String STATUS = root["STATUS"].as<String>();
        Serial.println(ssid);
        Serial.println(password);
        Serial.println(STATUS);
          if (STATUS.equals("On")) {
            digitalWrite(LED, LOW);
            digitalWrite(D3, LOW);
          }else{
            digitalWrite(LED, HIGH);
            digitalWrite(D3, HIGH);
          }
          WiFi.begin(ssid, password);
          ssidpass = ssid+","+password;
          writeString(16, ssidpass);
          String recivedData = read_String(16);
          Serial.println(recivedData);
          delay(10);
          while (WiFi.status() != WL_CONNECTED) {  
            delay(100);  
          }
        }  
     } else {  
        Serial.printf("[HTTPS] GET... failed, error: %s\n\r", https.errorToString(httpCode).c_str());  
     }  
     https.end();  
   } else {  
      Serial.printf("[HTTPS] Unable to connect\n\r");  
   }  
}

String getValue(String data, char separator, int index){
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void writeString(char add,String data){
  int _size = data.length();
  int i;
  for(i=0;i<_size;i++){
    EEPROM.put(add+i,data[i]);
  }
  EEPROM.put(add+_size,'\0');
  EEPROM.commit();
}

String read_String(char add){
  int i;
  char data[100];
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  while(k != '\0' && len<500){    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';
  return String(data);
}
      
void loop(){
  if(WiFi.status()== WL_CONNECTED){
    TestHttpsAPI();
  }else{
    WiFi.begin(ssid, password);  
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED, LOW);
      digitalWrite(D3, LOW);
      Serial.println("Not Connected2");
      delay(100);  
    }
  } 
  delay(2000);  
}
