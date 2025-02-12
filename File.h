#pragma once
#include "EspArchive.h"
#include "hash32.h"

#define ARFILE_DEBUG
#ifdef ARFILE_DEBUG
    #define debugPrintf(fmt, ...) { Serial.print(__PRETTY_FUNCTION__); Serial.println(__LINE__); Serial.printf((PGM_P)PSTR(fmt), ##__VA_ARGS__); }
#else
    #define debugPrintf
#endif

class ArchiveFile {
        private:
        File * data;
        const uint32_t start;
        const uint32_t len;
//        ArFs * arch;
        
        uint32_t seekPosition;
        public:
        ArchiveFile() :
            data(nullptr), start(0),len(0)
        { debugPrintf("Created null file\n"); };
        ArchiveFile(File *_file, const uint32_t start, const uint32_t len) :
            data(_file),start(start), len(len)
        { 
            
            seekPosition = 0;
            if ( data->seek(start) ){
                debugPrintf("Set start position %lu as start, file len=%lu\n", start, len);
            } else {
                debugPrintf("Error: can't to set %lu position\n", start);
                debugPrintf("Error: size=%lu, setted to %lu position\n", data->size(), data->position());
                
            }

        };
        // ArchiveFile(File *_file, ArFs::FileInfo& fi) :
        //     ArchiveFile(_file, fi.offset, fi.length)
        // {};
        ~ArchiveFile(){
            debugPrintf("Destroy object");
            //close();
        };
        operator bool() const { return data != nullptr; }; 
        ArchiveFile& operator=(const ArchiveFile& src) noexcept {
            if (this != &src) {
                //close();
                seekPosition = 0;
                new ( this ) ArchiveFile(src.data,src.start,src.len);
                debugPrintf("Copy '%s', %lu[%lu]\n", data->fullName(), start, len );
            }
            return *this;
        };
            // Print methods:
        size_t write(uint8_t) { return 0; };
        size_t write(const uint8_t *buf, size_t size) { return 0;};
        int availableForWrite() { return 0;};

            // Stream methods:
        int available(){ return len-seekPosition; };
        int read(){
            uint8_t result;
            if ( data->read(&result, 1) != 1) {
                return -1;
            }
            return result;
        };
//        int peek();
//        void flush();
        int read(uint8_t* buf, size_t size) {
            auto canRead = ( size <= available() ) ? size : available();
            auto readed = data->read(buf, canRead);
            seekPosition += readed;
            debugPrintf("Need %d bytes, readed %d bytes\n", size, readed );
            return readed;
        };
        size_t readBytes(char *buffer, size_t length) {
            return read((uint8_t*)buffer, length);
        };
        // bool seek(uint32_t pos, SeekMode mode);
        // bool seek(uint32_t pos) {
        //     return seek(pos, SeekSet);
        // }
        size_t position() const { return seekPosition; };
        size_t size() const { return len; };
        virtual ssize_t streamRemaining() { return (ssize_t)size() - (ssize_t)position(); }
        void close(){
            //data->close();
            seekPosition=0;
            debugPrintf("ArFs file closed\n");
        };

//        const char* name() const;
        bool isFile() const { return true; };
        bool isDirectory() const { return false; };
    };