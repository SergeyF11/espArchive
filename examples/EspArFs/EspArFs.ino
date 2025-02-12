#include <LittleFS.h>
#include <EspArFs.h>
#include "timeStr.h"

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

   if ( ! LittleFS.begin() ) {
    Serial.println("Error: Littlefs begin");
    return;
   }
    Serial.println("LittleFS ok");
    auto start = millis();
    Serial.printf("Start ArFs define in %lu\n", start );

    ArFs arFs( LittleFS, "/certs.ar");
    if ( arFs.getError() != ArFs::Errors::Ok ){
        Serial.print("Error ArFs="); Serial.println( arFs.errorStr() );
        Serial.println( arFs.getError() );
        return;
    }
    
    if( ! arFs.begin() ){
      Serial.print("Error ArFs="); Serial.println( arFs.errorStr() );
        return;
    }
    
    Serial.printf("ArFs define finish. Takes %lums\n", millis()-start );
    
    Serial.printf("ArFs ok. Has %d files in index\n", arFs.getCountFiles() );
    Serial.println( listDirToString(LittleFS, "/", true ));


    //preprocess ( offset= 172328 , 664 bytes, file 'ca_154.der')
    start =millis();
    ArFs::FileInfo info = arFs.getFileInfo("ca_154.der");
    

    if( info ){
        Serial.printf("offset=%d, length=%d\nTake in %lums\n", info.offset, info.length, millis()-start);
    }
    auto f = arFs.open("ca_154.der");
    
    
    uint8_t buf[20];
    uint8 c;
    // while( f.read(&buf[0],20) ){
    while( f.read(&c,1) ){
      if ( c == '+' ){
        //uint8 buf[20];
        buf[0] = c;
        f.read(&buf[1], 17 );
        buf[19] = '\0';
        break;
      }
    }
    
      Serial.print( (char*)buf );
    

}

void loop(){}