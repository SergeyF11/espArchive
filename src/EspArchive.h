#pragma once

//#define ARFS_DEBUG
//#define ARFILE_DEBUG

#include <FS.h>
#include "hash/hash32.h"
#include "file/File.h"
#include "debug.h"


#define arMagicStr "!<arch>\n"
#define constLength(array) (((sizeof(array))/(sizeof(array[0])))-1) 

class Archive {
    public:

    /// @brief инфорация о файле в архиве .ar
    /// { хеш имени, смещение, длинна }
    struct FileInfo {
        uint32_t nameHash;
        uint32_t offset;
        uint32_t length;
        
        bool valid() const;
        explicit operator bool () const;
        bool operator==(const char * name ) const;
        bool operator==(const uint32_t hash ) const;
    };

    ///  typedef enum Errors;
    /// @brief типы ошибок
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

    const char * errorStr();
    
    private:

    static bool getName( char * fileName, const uint8_t * fileHeader, uint maxLenName=16 );
    static FileInfo preprocess(const char * fileName, uint32_t offset, uint32_t length);

        FS *fs;
        int count = 0;
        uint32_t offset = 0;
        const char * dataFile;
        const char * indexFile;

        File data;
        File index;
        Errors error;

        const char * setIndexFile( const char * name);
        Errors buildIndex(bool overwrite=false);
        FileInfo _getFileInfo(const uint32_t nameHash );
        FileInfo _getFileInfo(const char * fileName);
        bool checkFs();

    public:
    bool begin(bool recreateIndex=true);
    ArchiveFile open(const FileInfo& fi);
    ArchiveFile open(const uint32_t hashName);
    ArchiveFile open(const char * fileName);
    void close();
    FileInfo getFileInfo(const char * fileName);
    Errors getError() const;
    int getCountFiles() const;

    int downloadFiles(Stream& stream, const char * _path=nullptr);
    size_t download(Stream& stream);

    Archive(FS& fs );
    Archive(FS& fs, const char * dataFile, const char * indexFile=nullptr );
    ~Archive(); 
    const char * getArchiveName() const; 
    friend ArchiveFile;
};
