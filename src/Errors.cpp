#include <EspArchive.h>


/// @brief сообщение об ошибке
/// @return указатель на текстовое сообщение ошибки
const char * Archive::errorStr(){
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

/// @brief вернуть код ошибки
/// @return 
Archive::Errors Archive::getError() const { return error; };