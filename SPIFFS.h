#ifndef _SPIFFS_H_
#define _SPIFFS_H_

#include "FS.h"

namespace fs
{

class SPIFFSFS : public FS
{
public:
    SPIFFSFS();
    bool begin(bool formatOnFail=false, const char * basePath="/spiffs", uint8_t maxOpenFiles=10);
    bool format();
    size_t totalBytes();
    size_t usedBytes();
    void end();
};

}

extern fs::SPIFFSFS SPIFFS;


#endif
