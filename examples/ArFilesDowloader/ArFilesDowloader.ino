#include <LittleFS.h>
#include <WiFiClient.h>
#include <EspArchive.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "fsUtils.h"

//#define MY_CREDENTIAL
#ifdef MY_CREDENTIAL
#include <my.h>
#else
#define WIFI_SSID "YOUR_WIFI"
#define WIFI_PSK  "YOUR_PSK"

#endif


void setup(){
    Serial.begin(115200);
    do{
        delay(300);
    } while( !Serial );
    Serial.println();

#ifdef MY_CREDENTIAL    
    if( ! myWiFiConnect() ){
        Serial.println("ERROR: wifi connection");
        return;
    } else {
        Serial.println("Connected WiFi");
    }
#else
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    auto startMs = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if ( (millis() -startMs) >= periodMs ) {
            Serial.print("Can't to connect to ");
            Serial.println( WIFI_SSID);
            return;
        }
        Serial.print('.');
        delay(500); 
    }
    Serial.print("Connected to ");
    Serial.println( WIFI_SSID);
#endif

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

   if ( ! LittleFS.begin() ) {
    Serial.println("Error: Littlefs begin");
    return;
   }
    Serial.println("LittleFS ok");
   ///clean files
    auto i = removeFiles(LittleFS, "test.txt::test.ar::test.ix::test.json", "::");
    if ( i > 0) Serial.printf("%d files deleted\n",i);

    auto start = millis();
    Serial.printf("Start ArFs define in %lums\n", start );

    Archive arFs( LittleFS, "/test.ar");
    HTTPClient https;
    if ( https.begin( *client, "https://smilart.ru/download/iso/test.ar")){
        int httpCode = https.GET();
        if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

          int writed = arFs.downloadFiles(https.getStream() ); 
           if ( writed ){
            Serial.print( writed );
            Serial.println(" files downloaded"); 
            Serial.printf( "in %dms\n", millis()-start );

            FsLs ls( LittleFS,"/");
            Serial.println( ls );

           } else {
            Serial.print("Error:"); Serial.println( arFs.errorStr() );
           }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
    Serial.println("=================== The end =========================");

}

void loop(){

}