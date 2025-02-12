#pragma once

namespace Time {
    
    static time_t startTime;

    /// @brief internal buffer for print time
    static char buf[20];
    // static char * buf = nullptr;
     void _free_buf(){
        // if ( buf != nullptr ) { 
        //     delete[] buf;
        //     buf = nullptr;
        // }
    };
   
    char * toStr(const time_t&);

    /// @brief check localtime is synced to NTP. Set vars for uptime
    /// @return true if synced
    bool isSynced(){
        auto now = time(nullptr);
        bool res = now > 3600*2*60;
        if ( startTime == 0 && res ){
                
                startTime = time(nullptr)- millis()/1000;
                // debugPrintf("%lu sec before time synced\n", secondsBeforeTimeSync);
                // debugPrintf("Synced time is %s\n", toStr(startTime));
                //_free_buf();
        }
        return res;
        //return time(nullptr) > 3600*2*60;
    };

    /// @brief create char[] with time string.
    /// @attention 
    /// @param  time_t; default: now  
    /// @return  char * to time string
    char * toStr(const time_t& now = time(nullptr))  {
        //_free_buf();
        // if ( buf == nullptr )
        //     buf =  new char[20]; //(char *)malloc(20);
        static constexpr char tmpl[] PROGMEM = "%4d-%02d-%02d %02d:%02d:%02d";
        //auto now = time(nullptr);
        auto _tm = localtime( &now );
        sprintf( buf, tmpl, 
            _tm->tm_year+1900, _tm->tm_mon+1, _tm->tm_mday, 
            _tm->tm_hour, _tm->tm_min, _tm->tm_sec );
        return buf;
    }
    // size_t printTo(Print& p){
    //     auto size =  p.print(toStr());
    //     //_free_buf();
    //     return size;
    // };

 
namespace ValueSize {
    /// @brief return size in kBytes as string
    /// @param _size size in bytes
    /// @param preccision true if need floating value
    /// @return String in format "%d(.%d)kB"
    String inKb(const unsigned long long _size, bool preccision=false){ 
        int rounder = ( preccision ? 10 : 100 ) * 5;
        //Serial.printf("Rounder=%u\n", rounder);

        unsigned long long size=_size+rounder;
        String _out(size/1024);
        if ( preccision ){
            String out;
            auto dec=((size%1024)*1000/1024)/100;
            if ( dec > 0 ) { 
                out += '.'; 
                //if ( dec < 10 ) out += '0';
                out += dec; 
            }
            if( !out.isEmpty() ) _out += out;
        }
        return _out + F("kB");
    };
}
}