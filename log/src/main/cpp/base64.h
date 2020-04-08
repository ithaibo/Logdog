//
// Created by haibo.wu on 2020/1/17.
//

#ifndef LOGDOG_BASE64_H
#define LOGDOG_BASE64_H

class Base64 {
public:
    static char* encode(const char *raw);
    static char* decode(const char *code);
};

#endif //LOGDOG_BASE64_H
