#include <LittleFS.h>
#include <EspArchive.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <hash/hash32.h>
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
    auto start = millis();
    Serial.printf("Start ArFs define in %lu\n", start );
    FsLs ls(LittleFS, "/");

    Archive arFs( LittleFS, "/test.ar");
    HTTPClient https;
    if ( https.begin( *client, "https://smilart.ru/download/iso/test.ar")){
        int httpCode = https.GET();
        if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

           size_t writed = arFs.download( https.getStream() );
           if ( writed ){
            Serial.print("Archive size ="); Serial.println( writed );
            //Serial.println( listDirToString(LittleFS, "/"));
            Serial.print(ls);
            
            // build index
            if ( arFs.begin(false) ){
                // доступ по имени
            //    {
                Serial.println("File name access");
                const char fileName[] = "test.txt";
                auto f = arFs.open(fileName);
                if (f) {
                    Serial.printf("file '%s' size=%lu\n", fileName, f.size());
                    uint8_t buf[ f.size() +1] = {0};
                    f.read(buf, f.size());
                    buf[ f.size()] = '\0';
                    Serial.println("+++++++++++ Content ++++++++++++++++");
                    Serial.println((char *)buf);
                    Serial.println("+++++++++++ Content ++++++++++++++++");
                    f.close();
                }
             //   }
                // доступ по хешу
                //{

                Serial.println("Hash access");
                //auto f = arFs.open( Hash32::_hash_const( "test.json") );
                //auto 
                f = arFs.open( "test.json"_h );
                if( f ){
                    Serial.printf("file size=%lu\n", f.size());
                    uint8_t buf[ f.size() +1] = {0};
                    f.read(buf, f.size());
                    buf[ f.size()] = '\0';
                    Serial.println("+++++++++++ Content ++++++++++++++++");
                    Serial.println((char *)buf);
                    Serial.println("+++++++++++ Content ++++++++++++++++");
                    f.close();
                } else {
                    Serial.print("Error: "); Serial.println( "open file" );
                }
               // }
            }
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