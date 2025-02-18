#include <EspArchive.h>

/// @brief упаковка информации о файле
/// @param fileName 
/// @param offset 
/// @param length 
/// @return FileInfo struct
Archive::FileInfo Archive::preprocess(const char * fileName, uint32_t offset, uint32_t length)
{
    Archive::FileInfo fi;
    memset(&fi, 0, sizeof(fi));
    fi.nameHash = Hash32::Hash(fileName);
    fi.length = length;
    fi.offset = offset;
    return fi;
};


/// @brief получить информацию о файле по хешу имени
/// @param nameHash 
/// @return FileInfo { hasheName, offset in ar, len }
Archive::FileInfo Archive::_getFileInfo(const uint32_t nameHash ){
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

/// @brief получить информацию о файле по имени 
/// @param fileName 
/// @return FileInfo { hasheName, offset in ar, len }
Archive::FileInfo Archive::_getFileInfo(const char * fileName){
    return _getFileInfo( Hash32::hash(fileName) );
};


/// @brief получить FileInfo по имени файла из индекса
/// @param fileName 
/// @return 
Archive::FileInfo Archive::getFileInfo(const char * fileName){
    return _getFileInfo(fileName);
};

/// @brief проверяет валидность информации
/// @return true если есть хеш имени
bool Archive::FileInfo::valid() const { return nameHash != 0; };

/// @brief преобразователь типа в bool
/// @return  true если есть FileInfo валидно
Archive::FileInfo::operator bool() const { return nameHash != 0; };

/// @brief оператор сравнения имени по строке 
/// @param name 
/// @return 
bool Archive::FileInfo::operator==(const char * name ) const { return Hash32::hash(name) == nameHash; };

/// @brief оператор сравнения имени по хешу
/// @param hash 
/// @return 
bool Archive::FileInfo::operator==(const uint32_t hash ) const { return hash == nameHash; };