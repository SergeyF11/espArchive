#pragma once

#ifdef ARFS_DEBUG
    #define debugPrintf(fmt, ...) { Serial.print(__PRETTY_FUNCTION__); Serial.println(__LINE__); Serial.printf((PGM_P)PSTR(fmt), ##__VA_ARGS__); }
    #define debugWrite(array,size) { Serial.write( array, size ); Serial.println(); }
#else
    #define debugPrintf
    #define debugWrite
#endif

#ifdef ARFILE_DEBUG
#ifndef debugAfPrintf
    #define debugAfPrintf(fmt, ...) { Serial.print(__PRETTY_FUNCTION__); Serial.println(__LINE__); Serial.printf((PGM_P)PSTR(fmt), ##__VA_ARGS__); }
#endif
#else
    #define debugAfPrintf
#endif