#pragma once
//#include "../EspArchive.h"
#include "../hash/hash32.h"
#include <FS.h>
#include "../debug.h"

class ArchiveFile {
        private:
        File * data;
        const uint32_t start;
        const uint32_t len;
//        ArFs * arch;
        
        uint32_t seekPosition;
        public:
        ArchiveFile();
        ArchiveFile(File *_file, const uint32_t start, const uint32_t len);

        ~ArchiveFile();
        operator bool() const; // { return data != nullptr; }; 
        ArchiveFile& operator=(const ArchiveFile& src) noexcept;
        
            // Print methods:
        size_t write(uint8_t); // { return 0; };
        size_t write(const uint8_t *buf, size_t size);// { return 0;};
        int availableForWrite();// { return 0;};

            // Stream methods:
        int available(); //{ return len-seekPosition; };
        int read(); //{
//        int peek();
//        void flush();
        int read(uint8_t* buf, size_t size); // {
        size_t readBytes(char *buffer, size_t length);
        
        // bool seek(uint32_t pos, SeekMode mode);
        // bool seek(uint32_t pos) {
        //     return seek(pos, SeekSet);
        // }
        size_t position() const; // { return seekPosition; };
        size_t size() const; // { return len; };
        virtual ssize_t streamRemaining();// { return (ssize_t)size() - (ssize_t)position(); }
        void close();
        
//        const char* name() const;
        bool isFile() const;// { return true; };
        bool isDirectory() const;// { return false; };
    };