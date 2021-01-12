package com.andy.logdog.sample.benchmark;

import androidx.annotation.NonNull;

public interface ILogger {
    void init(@NonNull String path);
    void d(String pattern, Object... params);
    void i(String pattern, Object... params);
    void w(String pattern, Object... params);
    void e(String pattern, Object... params);
    void exit();
}
