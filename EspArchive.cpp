#include "EspArchive.h"
#include "hash32.h"


Archive::FileInfo Archive::preprocess(uint32_t length, uint32_t offset, const char * fileName){
    Archive::FileInfo fi;
    // Clear the CertInfo
    memset(&fi, 0, sizeof(fi));
    fi.nameHash = Hash32::Hash(fileName);
    fi.length = length;
    fi.offset = offset;
    return fi;
}
