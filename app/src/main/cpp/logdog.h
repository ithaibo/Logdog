//
// Created by haibo.wu on 2020/1/13.
//

#ifndef LOGDOG_LOGDOG_H
#define LOGDOG_LOGDOG_H




/**
 * save content to file with mmap
 * @param toSave something to save to file
 */
void MmapWrite(char const* filePath, char const* toSave);

const char* readFile(char const *filePath);

void writeFile(const char *filePath, const char *contentSave);

size_t getFileSize(const char *filePath);

void write(const char* logContent);

#endif //LOGDOG_LOGDOG_H
