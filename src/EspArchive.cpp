#include "EspArchive.h"
#include "hash/hash32.h"

const char * Archive::getArchiveName() const{
    return dataFile;
};

/// @brief Деструктор
Archive::
~Archive(){
    close();
};

/// @brief Конструктор для работы с архивом как источником файлов
/// @param fs 
Archive::
Archive(::FS& fs ) :
    fs(&fs), dataFile(nullptr), indexFile(nullptr)
{
    checkFs();
};
/// @brief Конструктор для работы с архивом как ФС
/// @param fs 
/// @param dataFile 
/// @param indexFile 
Archive::
Archive(::FS& fs, const char * dataFile, const char * indexFile ) :
    fs(&fs), dataFile(strdup(dataFile)), indexFile(strdup(setIndexFile(indexFile)))
{
    if (! checkFs() ) return;
    if( ! this->fs->exists( dataFile )) {
        error = NoData;
        return;
    }

};

/// @brief загружает файлы из потока .ar на FS с декодированием на лету
/// @param stream 
/// @return число декодированных и записанных файлов
/// @note перезаписывает существующие файлы
int Archive::downloadFiles(Stream& stream, const char * _path){
    if ( _path && ! this->fs->exists(_path)){
        this->fs->mkdir(_path);
    }
    String path( _path ? "/" : _path);

    int fileCounter=0;
    size_t download = 0;
    error = Errors::WrongArchive;
    if( stream.available() > constLength(arMagicStr) ){
        // check magic
        uint8_t magic[constLength(arMagicStr)];
        if ( stream.readBytes(magic, sizeof(magic)) != sizeof(magic) || 
                memcmp(magic, arMagicStr, sizeof(magic)) ) {
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

/// @brief создаёт файл dataFile.ar из потока
/// @note перезапишет файл, если он существует
/// @return 
size_t Archive::download(Stream& stream){
    size_t size=0;
    File file;        
    if ( ( file=this->fs->open(dataFile, "w+") )){
        while ( stream.available() ){
            size += file.write( stream.read());
        }
        file.close();
    }   
    return size;
};

/// @brief получить имя в fileName[] из заголовка fileHeader
/// @param fileName 
/// @param fileHeader 
/// @param maxLenName 
/// @return 
bool Archive::getName( char * fileName, const uint8_t * fileHeader, uint maxLenName ) {
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

/// @brief имя файла индекса
/// @param name если nullptr, имя индекса совпадает с именем архива с суффиксом '.ix'
/// @return 
const char * Archive::setIndexFile( const char * name){
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

/// @brief построить индексный файл с хэшем имён 
/// @note файл даты должен быть уже открыт на чтение 'r'
/// @param overwrite индекс
/// @return код ошибки. Используйте errorStr() для описания ошибки
Archive::Errors Archive::buildIndex(bool overwrite){
    if( fs->exists(indexFile) && overwrite ) {
        fs->remove(indexFile);
    }
    index = fs->open(indexFile, "w");
    if ( ! index ) return IndexError;

    // build here
    uint8_t magic[constLength(arMagicStr)];
    if ( data.read(magic, sizeof(magic)) != sizeof(magic) ||
        memcmp(magic, arMagicStr, sizeof(magic)) ) {

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
        //берем длинну файла с позиции 48 заголовка
        if (1 != sscanf((char *)(fileHeader + 48), "%d", &length) || !length) {
            break;
        }

        // смещаем указатель чтения в файле data, убеждаемся что данные есть в архиве
        if( ! data.seek(length, SeekCur) ){
            break;
        }

        char fileName[16];
        if ( getName( fileName, fileHeader )){
            debugPrintf("preprocess ( offset= %u , %d bytes, file '%s')\n", offset, length, fileName );
            auto info = preprocess(fileName, offset, length);
            
            if (index.write((uint8_t *)&info, sizeof(info)) != (ssize_t)sizeof(info)) {
                //free(raw);
                break;
            }
            count++;
        }

        offset += length;
        //free(raw);
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


/// @brief закрывает файлы data и index
void Archive::close(){
    debugPrintf("Close data and index files\n");
    data.close();
    index.close();
};


/// @brief вернуть число файлов в индексе
/// @return 
int Archive::getCountFiles() const { return count; };

/// @brief прочитать архив и построить индекс
/// @param recreateIndex 
/// @return false если ошибка. используйте getError() для кода ошибки
bool Archive::begin(bool recreateIndex){
    debugPrintf("\n");
    if ( !( data = this->fs->open( dataFile, "r" ) ) ){
        error = DataError;
        return false;
    }
    error = buildIndex(recreateIndex);
    data.close();
    return error == Errors::Ok;
};


bool Archive::checkFs(){
    error = Ok;
    bool res = this->fs->begin();
    if ( ! res ) {
        error= NoFs;
    } else {
        debugPrintf("Fs ok\n");
    }
    return res;
};








// /// @brief проверяет валидность информации
// /// @return true если есть хеш имени
// bool Archive::FileInfo::valid() const { return nameHash != 0; };

// /// @brief преобразователь типа в bool
// /// @return  true если есть FileInfo валидно
// Archive::FileInfo::operator bool() const { return nameHash != 0; };

// /// @brief оператор сравнения имени по строке 
// /// @param name 
// /// @return 
// bool Archive::FileInfo::operator==(const char * name ) const { return Hash32::hash(name) == nameHash; };

// /// @brief оператор сравнения имени по хешу
// /// @param hash 
// /// @return 
// bool Archive::FileInfo::operator==(const uint32_t hash ) const { return hash == nameHash; };
