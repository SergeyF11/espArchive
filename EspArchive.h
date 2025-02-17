
#include <FS.h>
#pragma once

#include "hash32.h"
#include "File.h"

//#define ARFS_DEBUG
#ifdef ARFS_DEBUG
    #define debugPrintf(fmt, ...) { Serial.print(__PRETTY_FUNCTION__); Serial.println(__LINE__); Serial.printf((PGM_P)PSTR(fmt), ##__VA_ARGS__); }
    #define debugWrite(array,size) { Serial.write( array, size ); Serial.println(); }
#else
    #define debugPrintf
    #define debugWrite
#endif


#define magicStr "!<arch>\n"
#define constLength(array) (((sizeof(array))/(sizeof(array[0])))-1) 
class Archive {
    public:
    struct FileInfo {
        uint32_t nameHash;
        uint32_t offset;
        uint32_t length;
        bool valid() const { return nameHash != 0; };
        bool operator==(const char * name ) const { return Hash32::hash(name) == nameHash; };
        bool operator==(const uint32_t hash ) const { return hash == nameHash; };
        explicit inline operator bool () const { return nameHash != 0; };
    };
    typedef enum {
        Ok = 0,
        NoFs,
        NoData,
        DataError,
        NoIndex,
        IndexError,
        WrongArchive,
        Unknown,

    } Errors;
    const char * errorStr(){
        switch(error){
            case Ok: return PSTR("Ok");
            case NoFs: return PSTR("NoFs");
            case NoData: return PSTR("NoData");
            case DataError: return PSTR("DataError");
            case NoIndex: return PSTR("NoIndex");
            case IndexError: return PSTR("IndexError");
            case WrongArchive: return PSTR("WrongArchive");
        }
        return PSTR("Unknown");
    };
    
    private:
        /// @brief получить имя в fileName[] из заголовка fileHeader
        /// @param fileName 
        /// @param fileHeader 
        /// @param maxLenName 
        /// @return 
        static bool getName( char * fileName, const uint8_t * fileHeader, uint maxLenName=16 ) {
            if (fileHeader[0] == '/' || fileHeader[1] == '/') return false;
            bool gnuFormat = false;
            for( int i=0; i< maxLenName; i++){
                if( fileHeader[i] == '/' ) {
                    gnuFormat = true;
                    fileName[i]= '\0';
                    break;
                }
                fileName[i]= (char)fileHeader[i];
            }
            if ( ! gnuFormat ){
                // trim name
                debugPrintf("Gnu format. Trim name\n");
                int i=maxLenName-1;
                while( fileName[i] == 0x20 ){
                    i--;
                }
                fileName[i+1]= '\0';
            }
            return true;
        };
        static FileInfo preprocess(uint32_t length, uint32_t offset, const char * fileName);
        FS *fs;
        int count = 0;
        uint32_t offset = 0;
        const char * dataFile;
        const char * indexFile;
        const char * setIndexFile( const char * name){
            if( name != nullptr) {
                debugPrintf("%s\n", name);
                return name;    
            }
            auto _name = strdup(dataFile);
            auto i = strlen(_name);
            _name[--i] = 'x';
            _name[--i] = 'i';
            debugPrintf("%s\n", _name);
            return _name;
        };
        File data;
        File index;
        Errors error;

        /// @brief построить индексный файл с хэшем имён
        /// @param overwrite индекс
        /// @return код ошибки. Используйте errorStr() для описания ошибки
        Errors buildIndex(bool overwrite=false){
            if( fs->exists(indexFile) && overwrite ) {
                fs->remove(indexFile);
            }
            index = fs->open(indexFile, "w");
            if ( ! index ) return IndexError;

            // build here
            uint8_t magic[constLength(magicStr)];
            //uint8_t magic[8];
            if ( data.read(magic, sizeof(magic)) != sizeof(magic) ||
                memcmp(magic, magicStr, sizeof(magic)) ) {

                data.close();
                index.close();
                return WrongArchive;
            }
            offset += sizeof(magic);

            while (true) {
                delay(0);
                uint8_t fileHeader[60];
                // 0..15 = filename in ASCII
                // 48...57 = length in decimal ASCII
                
                int32_t length;
                if (data.read(fileHeader, sizeof(fileHeader)) != sizeof(fileHeader)) {
                    break;
                }
                offset += sizeof(fileHeader);
                fileHeader[58] = 0;
                if (1 != sscanf((char *)(fileHeader + 48), "%d", &length) || !length) {
                    break;
                }

                void *raw = malloc(length);
                if (!raw) {
                    break;
                }
                if (data.read((uint8_t *)raw, length) != length) {
                    free(raw);
                    break;
                }
        //        debugPrintf("Header: '%s'\n", (char *)fileHeader );
                char fileName[16];
                if ( getName( fileName, fileHeader )){
                    debugPrintf("preprocess ( offset= %u , %d bytes, file '%s')\n", offset, length, fileName );
                    auto info = preprocess(length, offset, fileName);
                    
                    if (index.write((uint8_t *)&info, sizeof(info)) != (ssize_t)sizeof(info)) {
                        free(raw);
                        break;
                    }
                    count++;
                }

                offset += length;
                free(raw);
                if (offset & 1) {
                uint8_t x;
                data.read(&x, 1);
                offset++;
                }
            }
            data.close();
            delay(1);
            index.close();
            debugPrintf("Buld index for %d files\n",count );
            return count > 0 ? Ok : IndexError;
        };

        /// @brief получить информацию для файла по хешу
        /// @param nameHash 
        /// @return FileInfo { hasheName, offset in ar, len }
        FileInfo _getFileInfo(const uint32_t nameHash ){
            FileInfo info;
            index = fs->open( indexFile, "r");
            if( index ){ 
                //for( int i = 0; i = count; i++){
                FileInfo buf;
                while( index.readBytes((char*)&buf,sizeof(buf)) ){
                    if( buf == nameHash ) {
                        info = buf;
                        break;
                    }
                }
                index.close();
            }
            debugPrintf("%s (nameHash=%lu, offset=%lu, len=%lu)\n", info.valid()? "Valid": "Invalid", info.nameHash, info.offset, info.length );
            return info;
        };

        /// @brief получить информацию для файла по имени 
        /// @param fileName 
        /// @return FileInfo { hasheName, offset in ar, len }
        FileInfo _getFileInfo(const char * fileName){
            return _getFileInfo( Hash32::hash(fileName) );
  
        };
        bool checkFs(){
            error = Ok;
            bool res = this->fs->begin();
            if ( ! res ) {
                error= NoFs;
            } else {
                debugPrintf("Fs %s ok\n", "LittleFS");
            }
            return res;
        };
    public:
    void close(){
        debugPrintf("Close data and index files\n");
        data.close();
        index.close();
    };

    ArchiveFile open(const FileInfo& fi){
        debugPrintf("File info is %s\n", fi.valid()? "valid": "INVALID");
        if( fi ){
            debugPrintf("Hash[%lu]=> %lu[%lu]\n", fi.nameHash, fi.offset, fi.length );
            data = fs->open( dataFile, "r");
            if ( data ) 
                return ArchiveFile( &data, fi.offset, fi.length );
            else    
                debugPrintf("Error open data file\n");
        }
        return ArchiveFile();
    };
    ArchiveFile open(const char * fileName){
        debugPrintf("%s\n", fileName );
        //auto nameHash = Hash32::hash(fileName);
        FileInfo fi = _getFileInfo(fileName);
        return open(fi);
        };
    ArchiveFile open(const uint32_t hashName){
        debugPrintf( "%lu", hashName );
        //return open(_getFileInfo( hashName ));
        FileInfo fi = _getFileInfo(hashName);
        return open(fi);
    }

    FileInfo getFileInfo(const char * fileName){
        return _getFileInfo(fileName);
    };

    /// @brief вернуть код ошибки
    /// @return 
    Errors getError(){ return error; };
    
    /// @brief вернуть число файлов в индексе
    /// @return 
    int getCountFiles(){ return count; };
    
    /// @brief прочитать архив и построить индекс
    /// @param recreateIndex 
    /// @return false если ошибка. исользуйте getError() для кода ошибки
    bool begin(bool recreateIndex=true){
        debugPrintf("\n");
        if ( !( data = this->fs->open( dataFile, "r" ) ) ){
            error = DataError;
            return false;
        }
        error = buildIndex(recreateIndex);
        data.close();
        return error == Errors::Ok;
    };


    //             uint8_t magic[constLength(magicStr)];
    //         //uint8_t magic[8];
    //         if (data.read(magic, sizeof(magic)) != sizeof(magic) ||
    //         memcmp(magic, magicStr, sizeof(magic)) ) {

    /// @brief загружает файлы из потока .ar на FS с декодированием на лету
    /// @param stream 
    /// @return 
    int downloadFiles(Stream& stream, const char * _path=nullptr){
        if ( _path && ! this->fs->exists(_path)){
            this->fs->mkdir(_path);
        }
        String path( _path ? "/" : _path);

        int fileCounter=0;
        size_t download = 0;
        error = Errors::WrongArchive;
        if( stream.available() > constLength(magicStr) ){
            // check magic
            uint8_t magic[constLength(magicStr)];
            if ( stream.readBytes(magic, sizeof(magic)) != sizeof(magic) || 
                 memcmp(magic, magicStr, sizeof(magic)) ) {
                error = WrongArchive;
                debugWrite(magic, sizeof(magic));

                return 0;
                }
            download += sizeof(magic);
            // magic OK
            debugPrintf("Magic found "); debugWrite(magic, sizeof(magic) );
            //read file header
            while ( stream.available() ) {
                if ( stream.available() <60 ) delay(10);
                else delay(0);
                uint8_t fileHeader[60];
                // 0..15 = filename in ASCII
                // 48...57 = length in decimal ASCII
                
                int32_t length;
                if (stream.readBytes(fileHeader, sizeof(fileHeader)) != sizeof(fileHeader)) {
                    error = WrongArchive;
                    break;
                }
                debugPrintf("Find header\n");
                download += sizeof(fileHeader);
                fileHeader[58] = 0;
                if (1 != sscanf((char *)(fileHeader + 48), "%d", &length) || !length) {
                    error = WrongArchive;
                    break;
                }
                char fileName[16];
                if ( ! getName(fileName, fileHeader )) {
                    debugPrintf("No name found\n");
                    error = WrongArchive;
                    break;
                }
                debugPrintf("File '%s', size=%d\n", fileName, length);
                File file;
                if ( ! ( file = this->fs->open( (path+fileName).c_str() , "w+") ) ){
                    debugPrintf("Error create file %s\n", (path+fileName).c_str() );
                    error = Errors::Unknown;
                    break;
                }
                size_t writed = 0;
                while( writed != length && stream.available() ){
                    writed += file.write( stream.read() );
                }
                file.close();
                if ( writed != length){
                    error = Errors::Unknown;
                    break;
                }
                fileCounter++;
                download += writed;
                // выравнивание
                if (download & 1) {
                    uint8_t x;
                    stream.read(&x, 1);
                    download++;
                }
            }
        } else debugPrintf("No stream. available(%d)\n", stream.available() );
        if ( fileCounter ) error = Errors::Ok;
        return fileCounter;
    };

    /// @brief download dataFile.ar file
    /// @return 
    size_t download(Stream& stream/* , bool owerwrite=false*/){
        size_t size=0;
        File file;        
    /*    if( ! owerwrite && this->fs->exists(dataFile) ){
            if ( ( file=this->fs->open(dataFile, "r"))){
                size = file.size();
                file.close();
                return size;
            } 
        } */
        if ( ( file=this->fs->open(dataFile, "w+") )){
            while ( stream.available() ){
                size += file.write( stream.read());
            }
            file.close();
        }   
        return size;
    };

    /// @brief Для работы с архивом как источником файлов
    /// @param fs 
    Archive(FS& fs ) :
    fs(&fs), dataFile(nullptr), indexFile(nullptr){
        checkFs();
    };
    
    /// @brief Для работы с архивом как ФС
    /// @param fs 
    /// @param dataFile 
    /// @param indexFile 
    Archive(FS& fs, const char * dataFile, const char * indexFile=nullptr ) :
    fs(&fs), dataFile(strdup(dataFile)), indexFile(strdup(setIndexFile(indexFile)))
    {
        if (! checkFs() ) return;
        if( ! this->fs->exists( dataFile )) {
            error = NoData;
            return;
        }

    };
    ~Archive(){
        close();
    };
    friend ArchiveFile;
};
