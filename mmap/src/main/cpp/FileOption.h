//
// Created by haibo.wu on 2020/1/13.
//

#ifndef LOGDOG_FILEOPTION_H
#define LOGDOG_FILEOPTION_H

class FileOption {
public:
    size_t getFileSize(const char *filePath);

    size_t obtainFileSize(const char *path);

    size_t getFileSizeByFd(int fd);

    bool fileExists(const char * filePath);

    int openFdForMMAP(const char* path);

    std::string readFileAll(const char *filePath);
};
#endif //LOGDOG_FILEOPTION_H
