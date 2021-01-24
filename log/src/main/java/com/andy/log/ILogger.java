package com.andy.log;

import androidx.annotation.NonNull;

public interface ILogger {
    boolean init(@NonNull String path);
    boolean d(String pattern, Object... params);
    boolean i(String pattern, Object... params);
    boolean w(String pattern, Object... params);
    boolean e(String pattern, Object... params);
    void exit();
}
