#include <EspArchive.h>

/// @brief открыть файл на чтение в архиве по имени
/// @param fileName 
/// @return 
ArchiveFile Archive::open(const char * fileName){
    debugPrintf("%s\n", fileName );
    //auto nameHash = Hash32::hash(fileName);
    FileInfo fi = _getFileInfo(fileName);
    return open(fi);
    };

/// @brief открыть файл на чтение в архиве по хешу имени
/// @param hashName 
/// @return 
ArchiveFile Archive::open(const uint32_t hashName){
    debugPrintf( "%lu", hashName );
    //return open(_getFileInfo( hashName ));
    FileInfo fi = _getFileInfo(hashName);
    return open(fi);
};


/// @brief открыть файл на чтение в архиве по FileInfo
/// @param fi 
/// @return ArchiveFile
ArchiveFile Archive::open(const FileInfo& fi){
    debugPrintf("File info is %s\n", fi.valid()? "valid": "INVALID");
    if( fi ){
        if ( ! data ) { 
            debugPrintf("Hash[%lu]=> %lu[%lu]\n", fi.nameHash, fi.offset, fi.length );
            data = fs->open( dataFile, "r");
        }
        if ( data ) 
            return ArchiveFile( &data, fi.offset, fi.length );
        else    
            debugPrintf("Error open data file\n");
    }
    return ArchiveFile();
};