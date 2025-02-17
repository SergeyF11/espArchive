#include <LittleFS.h>

#include <EspArchive.h>
#include "timeStr.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "fsUtils.h"

#define MY_CREDENTIAL
#include <my.h>



void setup(){
    Serial.begin(115200);
    do{
        delay(300);
    } while( !Serial );
    Serial.println();

    if( ! myWiFiConnect() ){
        Serial.println("ERROR: wifi connection");
        return;
    } else {
        Serial.println("Connected WiFi");
    }
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
    Serial.printf("Start ArFs define in %lu\n", start );

    Archive arFs( LittleFS, "/test.ar");
    HTTPClient https;
    if ( https.begin( *client, "https://smilart.ru/download/iso/test.ar")){
        int httpCode = https.GET();
        if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
       
          // size_t writed = arFs.download( https.getStream() );
          int writed = arFs.downloadFiles(https.getStream() ); 
           if ( writed ){
            Serial.print( writed );
            Serial.println(" files"); 
            FsLs ls(&LittleFS,"/");
            Serial.println( ls );
            //DirFs::printTo(Serial, LittleFS,"/");
            // String ls = listDirToString(LittleFS, "/");
            // Serial.println( ls );

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