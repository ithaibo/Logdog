//
// Created by haibo.wu on 2020/1/13.
//

#ifndef LOGDOG_FILEOPTION_H
#define LOGDOG_FILEOPTION_H

#include <vector>

class FileOption {
public:
    size_t getFileSize(const char *filePath);

    size_t obtainFileSize(const char *path);

    bool fileExists(const char * filePath);

    int openFdForMMAP(const char* path);

    std::string readFileAll(const char *filePath);

    static void writeFile(const char * path, uint8_t *data, const size_t length);

//    std::vector<std::string> listAllLog
};
#endif //LOGDOG_FILEOPTION_H
