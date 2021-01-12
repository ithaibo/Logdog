package com.andy.logdog.sample.benchmark;

import androidx.annotation.NonNull;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class SyncLogger implements ILogger {
    private static ILogger instance;


    public static ILogger getInstance() {
        if (null == instance) {
            instance = new SyncLogger();
        }
        return instance;
    }

    private FileOutputStream fos;
    private String path;

    @Override
    public void init(@NonNull String path) {
        this.path = path;

    }

    @Override
    public void d(String pattern, Object... params) {
        writeLog(pattern);
    }

    @Override
    public void i(String pattern, Object... params) {
        writeLog(pattern);
    }

    @Override
    public void w(String pattern, Object... params) {
        writeLog(pattern);
    }

    private void writeLog(String pattern) {
        File file = new File(path);
        try {
            fos = new FileOutputStream(file, true);
            fos.write(pattern.getBytes());
            fos.flush();

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                fos.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            fos = null;
        }
    }

    @Override
    public void e(String pattern, Object... params) {
        writeLog(pattern);
    }

    @Override
    public void exit() {
        if (null == fos) return;
    }
}
