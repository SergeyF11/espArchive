#pragma once
#include <FS.h>
#include <LittleFS.h>


namespace Time {    
    static time_t startTime;

    /// @brief internal buffer for print time
    static char buf[20];
   
    char * toStr(const time_t&);

    /// @brief check localtime is synced to NTP. Set vars for uptime
    /// @return true if synced
    bool isSynced(){
        auto now = time(nullptr);
        bool res = now > 3600*2*60;
        if ( startTime == 0 && res ){                
                startTime = time(nullptr)- millis()/1000;
        }
        return res;
    };

    /// @brief create char[] with time string.
    /// @attention 
    /// @param  time_t; default: now  
    /// @return  char * to time string
    char * toStr(const time_t& now = time(nullptr))  {
        static constexpr char tmpl[] PROGMEM = "%4d-%02d-%02d %02d:%02d:%02d";
        auto _tm = localtime( &now );
        sprintf( buf, tmpl, 
            _tm->tm_year+1900, _tm->tm_mon+1, _tm->tm_mday, 
            _tm->tm_hour, _tm->tm_min, _tm->tm_sec );
        return buf;
    }
};

namespace LsFs{
    static FS* fs = nullptr;
    static size_t printTo(Print& _p, const String& dirName, bool subDir = false ) {
        size_t size = 0;    
        if( ! subDir ) {
            size += _p.println(dirName);
        }  
        Dir next = fs->openDir(dirName);
        while (next.next()) {
            File file = next.openFile("r");
            size += _p.print(' ');
            if ( subDir ) size += _p.print(' '); 
            if ( file.isDirectory()){
              size += _p.print( next.fileName());
              size += _p.println('/');
              {
              String filePath = dirName;
              filePath += next.fileName();
              size += printTo(_p, filePath, true);    
              }
            } else 
              if (next.isFile()){   
                size += _p.printf_P( (PGM_P)PSTR("%s - %d bytes "), next.fileName().c_str(), file.size() );
                // size += _p.print(next.fileName());
                // size += _p.print(F(" - "));
                // size += _p.print( file.size());
                // size += _p.print(F(" bytes "));
            }
            
            time_t cr = file.getCreationTime();
            time_t lw = file.getLastWrite();
            file.close();
            size += _p.printf_P( (PGM_P)PSTR(" C:%s M:%s\n"), Time::toStr( cr ), Time::toStr( lw));
            // size += _p.print((F(" C:"))); 
            // size += _p.print( Time::toStr( cr ));
            // size += _p.print(F(" M:"));
            // size += _p.println( Time::toStr( lw));
          }
          return size;
    };    
};

struct FsLs : public Printable {
    private:
    const String dirname;
    public:
    
    FsLs(FS& fs, const String& dirname) :
        dirname(dirname)
        { LsFs::fs = &fs; };
    size_t printTo(Print& p) const {
        return LsFs::printTo(p, dirname);
    };
};


//Dir next = fs->ropenDir(dirname)
// namespace FsDir {
// size_t printTo(Print& p, FS& fs, const String& dirname, bool subDir = false ){
//     size_t size = 0;    
//     if( ! subDir ) {
//         size += p.println(dirname);
//     }  
//     Dir next = fs.openDir(dirname);
//     while (next.next()) {
//         File file = next.openFile("r");
//         size += p.print(' ');
//         if ( subDir ) size += p.print(' '); 
//         if ( file.isDirectory()){
//           size += p.print( next.fileName());
//           size += p.println('/');
//           {
//           String filePath = dirname;
//           filePath += next.fileName();
//           size += printTo(p, fs, filePath, true);    
//           }
//         } else 
//           if (next.isFile()){   
//             size += p.print(next.fileName());
//             size += p.print(F(" - "));
//             size += p.print( file.size());
//             size += p.print(F(" bytes "));
//         }
        
//         time_t cr = file.getCreationTime();
//         time_t lw = file.getLastWrite();
//         file.close();
//         size += p.print((F(" C:"))); 
//         size += p.print( Time::toStr( cr ));
//         size += p.print(F(" M:"));
//         size += p.println( Time::toStr( lw));
//       }
//       return size;
// };
// }

int removeFiles(FS& fs, char * namesSeparated, const char * separator="," ){
    int deleted = 0;
    char * rest = namesSeparated;
    char *name;
    while(( name = strtok_r(rest, separator , &rest ) )){
        if( fs.remove(name) ) {
            Serial.printf("File '%s' deleted\n",name);
            deleted ++;
        }
        else Serial.printf("Error: delete file '%s'\n", name);
    }
    return deleted;
};
int removeFiles(FS& fs, const char * namesSeparated, const char * separator="," ){
    char * names = strdup(namesSeparated);
    int i = removeFiles(fs, names, separator);
    free(names);
    return i;
};

// String listDirToString( FS& fs, const String& dirname, bool subDir=false){
//     //  D_PRINT("List dir: ", dirname);  
        
//       String out;  
//       if( ! subDir ) {
//         out = String(dirname);
//         out.concat( F("\r\n"));
//       }  
//       Dir next = fs.openDir(dirname);
      
//       while (next.next()) {
//         File file = next.openFile("r");
//         out.concat(F(" "));  
        

//         if ( subDir ) out.concat(F(" ")); 
//         if ( file.isDirectory()){
//           out += next.fileName();
//           out.concat( F("/\r\n"));
//           String filePath = dirname;
//           filePath += next.fileName();
//           out += listDirToString(fs, filePath, true);    
          
//         } else 
//           if (next.isFile()){   
            

//             out += (next.fileName());
//             out.concat(F(" - "));
//             out += file.size();
//             out.concat(F(" bytes "));
//         }
        
//         time_t cr = file.getCreationTime();
//         time_t lw = file.getLastWrite();
//         file.close();
//         out.concat(F(" C:")); 
//         out += Time::toStr( cr);
//         out.concat(F(" M:"));
//         out += Time::toStr( lw);
//         out.concat(F("\r\n"));
//         Time::_free_buf();
//       }
// String listDirToString( FS& fs, const String& dirname, bool subDir=false){
//     //  D_PRINT("List dir: ", dirname);  
        
//       String out;  
//       if( ! subDir ) {
//         out = String(dirname);
//         out.concat( F("\r\n"));
//       }  
//       Dir next = fs.openDir(dirname);
      
//       while (next.next()) {
//         File file = next.openFile("r");
//         out.concat(F(" "));  
        

//         if ( subDir ) out.concat(F(" ")); 
//         if ( file.isDirectory()){
//           out += next.fileName();
//           out.concat( F("/\r\n"));
//           String filePath = dirname;
//           filePath += next.fileName();
//           out += listDirToString(fs, filePath, true);    
          
//         } else 
//           if (next.isFile()){   
            

//             out += (next.fileName());
//             out.concat(F(" - "));
//             out += file.size();
//             out.concat(F(" bytes "));
//         }
        
//         time_t cr = file.getCreationTime();
//         time_t lw = file.getLastWrite();
//         file.close();
//         out.concat(F(" C:")); 
//         out += Time::toStr( cr);
//         out.concat(F(" M:"));
//         out += Time::toStr( lw);
//         out.concat(F("\r\n"));
//         Time::_free_buf();
//       }
//       if ( ! subDir ){
//         out.concat(F("FS uses "));
//         FSInfo info;
//         LittleFS.info(info);
//         out += info.usedBytes;
//         out += F(" bytes of ");
//         out += info.totalBytes;
//         out.concat(F("\r\n"));        
//         //nextLine(out);
//       }    
      
//     //  D_PRINT("Listdir result: ", out);  
//       return out;
//     };
//       if ( ! subDir ){
//         out.concat(F("FS uses "));
//         FSInfo info;
//         LittleFS.info(info);
//         out += info.usedBytes;
//         out += F(" bytes of ");
//         out += info.totalBytes;
//         out.concat(F("\r\n"));        
//         //nextLine(out);
//       }    
      
//     //  D_PRINT("Listdir result: ", out);  
//       return out;
//     };