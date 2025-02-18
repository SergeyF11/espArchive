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
    
    FsLs(FS& fs, const String& dirname="/") :
        dirname(dirname)
        { LsFs::fs = &fs; };
    size_t printTo(Print& p) const {
        return LsFs::printTo(p, dirname);
    };
};


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
    int deleted = removeFiles(fs, names, separator);
    free(names);
    return deleted;
};

