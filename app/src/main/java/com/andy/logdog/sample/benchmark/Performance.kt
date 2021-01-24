package com.andy.logdog.sample.benchmark

import com.andy.logdog.sample.logdog.Logdog


class Performance {

    fun logdog():Boolean {
        return Logdog.getInstance().d(LocalMockLogRepository.apiResponse)
    }

    fun xlog() :Boolean {
        return XLogger.getInstance().d(LocalMockLogRepository.apiResponse)
    }
}