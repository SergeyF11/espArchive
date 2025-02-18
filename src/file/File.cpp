#include "File.h"

/// @brief пустой конструктор файла в архиве
ArchiveFile::ArchiveFile() :
    data(nullptr), start(0),len(0)
{ 
    debugAfPrintf("Created null file\n"); 
};

/// @brief конструктор файла в архиве
/// @param _file файл архива
/// @param start начало файла = смещение в архиве
/// @param len  размер файла
ArchiveFile::ArchiveFile(File *_file, const uint32_t start, const uint32_t len) :
    data(_file),start(start), len(len)
{    
    seekPosition = 0;
    if ( data->seek(start, SeekSet) ){
        debugAfPrintf("Set start position %lu as start, file len=%lu\n", start, len);
    } else {
        debugAfPrintf("Error: can't to set %lu position\n", start);
        debugAfPrintf("Error: size=%lu, setted to %lu position\n", data->size(), data->position());
        
    }

};

/// @brief деструктор файла
ArchiveFile::~ArchiveFile(){
    debugAfPrintf("Destroy object");
    //close();
};
/// @brief приведение типа файла для проверки
ArchiveFile::operator bool() const { return data != nullptr; }; 

/// @brief присваивание 
/// @param src 
/// @return 
ArchiveFile& ArchiveFile::operator=(const ArchiveFile& src) noexcept {
    if (this != &src) {
        //close();
        seekPosition = 0;
        new ( this ) ArchiveFile(src.data,src.start,src.len);
        debugAfPrintf("Copy '%s', %lu[%lu]\n", data->fullName(), start, len );
    }
    return *this;
};
    // Print methods:
size_t ArchiveFile::write(uint8_t) { return 0; };
size_t ArchiveFile::write(const uint8_t *buf, size_t size) { return 0;};
int ArchiveFile::availableForWrite() { return 0;};

    // Stream methods:
int ArchiveFile::available(){ return len-seekPosition; };
int ArchiveFile::read(){
    uint8_t result;
    if ( data->read(&result, 1) != 1) {
        return -1;
    }
    return result;
};
//        int peek();
//        void flush();
int ArchiveFile::read(uint8_t* buf, size_t size) {
    auto canRead = ( size <= available() ) ? size : available();
    auto readed = data->read(buf, canRead);
    seekPosition += readed;
    debugAfPrintf("Need %d bytes, readed %d bytes\n", size, readed );
    return readed;
};
size_t ArchiveFile::readBytes(char *buffer, size_t length) {
    return read((uint8_t*)buffer, length);
};
// bool seek(uint32_t pos, SeekMode mode);
// bool seek(uint32_t pos) {
//     return seek(pos, SeekSet);
// }
size_t ArchiveFile::position() const { return seekPosition; };
size_t ArchiveFile::size() const { return len; };
ssize_t ArchiveFile::streamRemaining() { return (ssize_t)size() - (ssize_t)position(); }
void ArchiveFile::close(){
    //data->close();
    seekPosition=0;
    debugAfPrintf("ArFs file closed\n");
};

//        const char* name() const;
bool ArchiveFile::isFile() const { return true; };
bool ArchiveFile::isDirectory() const { return false; };
