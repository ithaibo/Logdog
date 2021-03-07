//
// Created by Andy on 3/6/21.
//

#ifndef LOGDOG_DEFAULTSERIALIZER_H
#define LOGDOG_DEFAULTSERIALIZER_H

#include "LogInterceptor.h"
#include "../log_protocol.h"

class DefaultSerializer : public LogInterceptor<HbLog, uint8_t> {
public:
    uint8_t *visit(HbLog *input) override;
};

#endif //LOGDOG_DEFAULTSERIALIZER_H
