#include <LittleFS.h>
#include <EspArchive.h>
#include "timeStr.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <hash32.h>

#define MY_CREDENTIAL
#include <my.h>


String listDirToString( FS& fs, const String& dirname, bool subDir=false){
//  D_PRINT("List dir: ", dirname);
  
  String out;  
  if( ! subDir ) {
    out = String(dirname);
    out.concat( F("\r\n"));
  }  
  Dir root = fs.openDir(dirname);
  
  while (root.next()) {
    File file = root.openFile("r");
    out.concat(F(" "));  
    if ( subDir ) out.concat(F(" ")); 
    if ( file.isDirectory()){
      out += root.fileName();
      out.concat( F("/\r\n"));
      String filePath = dirname;
      filePath += root.fileName();
      out += listDirToString(fs, filePath, true);    
      
    } else 
      if (root.isFile()){   
        out += (root.fileName());
        out.concat(F(" - "));
        out += file.size();
        out.concat(F(" bytes "));
    }
    
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    out.concat(F(" C:")); 
    out += Time::toStr( cr);
    out.concat(F(" M:"));
    out += Time::toStr( lw);
    out.concat(F("\r\n"));
    Time::_free_buf();
  }
  if ( ! subDir ){
    out.concat(F("FS uses "));
    FSInfo info;
    LittleFS.info(info);
    out += info.usedBytes;
    out += F(" bytes of ");
    out += info.totalBytes;
    out.concat(F("\r\n"));        
    //nextLine(out);
  }    
  
//  D_PRINT("Listdir result: ", out);  
  return out;
};


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

           size_t writed = arFs.download( https.getStream() );
           if ( writed ){
            Serial.print("Archive size ="); Serial.println( writed );
            //Serial.println( listDirToString(LittleFS, "/"));
            
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