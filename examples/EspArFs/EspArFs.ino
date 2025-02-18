#pragma message("Run ArStream.ino example first")
#pragma message("Сначала выполните пример ArStream.ino")


#include <LittleFS.h>
#include <EspArchive.h>
#include  "fsUtils.h" 

void setup(){
    Serial.begin(115200);
    do{
        delay(300);
    } while( !Serial );
    Serial.println();

   if ( ! LittleFS.begin() ) {
    Serial.println("Error: Littlefs begin");
    return;
   }
    Serial.println("LittleFS ok");
    auto start = millis();
    Serial.printf("Start ArFs define in %lu\n", start );

    Archive arFs( LittleFS, "/test.ar");
    if ( arFs.getError() != Archive::Errors::Ok ){
        Serial.print("Error ArFs="); Serial.println( arFs.errorStr() );
        Serial.println( arFs.getError() );
        return;
    }
    
    if( ! arFs.begin() ){
      Serial.print("Error ArFs="); Serial.println( arFs.errorStr() );
        return;
    }
    
    Serial.printf("ArFs define finish. Takes %lums\n", millis()-start );
    
    Serial.printf("ArFs '%s' ok. Has %d files in index\n", arFs.getArchiveName(),  arFs.getCountFiles() );
    FsLs ls(LittleFS);

    Serial.println( ls );


    //preprocess ( offset= 172328 , 664 bytes, file 'ca_154.der')
    start =millis();
    Archive::FileInfo info = arFs.getFileInfo("ca_154.der");
    

    if( info ){
        Serial.printf("offset=%d, length=%d\nTake in %lums\n", info.offset, info.length, millis()-start);
    }

    // доступ по имени
    auto f = arFs.open("test.json");

    char buf[100] ={0};
    f.readBytes( buf, f.size());    
    f.close();
    Serial.println( "+++++++++++++++ test.json +++++++++++++++" );
    Serial.println( buf );

    // доступ по хешу имени
    f = arFs.open("test.txt"_h);

    auto readed = f.readBytes( buf, f.size());  
    f.close();
    buf[readed] = '\0';

    Serial.println( "+++++++++++++++ test.txt +++++++++++++++" );
    Serial.println( buf );
    Serial.println( "++++++++++++ the end +++++++++++++++");
}

void loop(){}