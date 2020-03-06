//
// Created by haibo.wu on 2020/1/13.
//

#ifndef LOGDOG_FILEOPTION_H
#define LOGDOG_FILEOPTION_H

class FileOption {
public:
    /**
     * note: free
     * @param filePath
     * @return
     */
    const char* readFile(char const *filePath);
    size_t getFileSize(const char *filePath);
    size_t obtainFileSize(const char *path);
    void freeTempBuffer();

private:
    /**
     * temp buffer for read file
     */
    char *temp = nullptr;
};

#endif //LOGDOG_FILEOPTION_H
