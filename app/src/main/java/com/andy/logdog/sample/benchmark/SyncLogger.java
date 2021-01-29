package com.andy.logdog.sample.benchmark;

import androidx.annotation.NonNull;

import com.andy.log.ILogger;

import java.io.File;
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
    public boolean init(@NonNull String path) {
        this.path = path;
        return true;
    }

    @Override
    public boolean d(String pattern, Object... params) {
        writeLog(pattern);
        return true;
    }

    @Override
    public boolean i(String pattern, Object... params) {
        writeLog(pattern);
        return true;
    }

    @Override
    public boolean w(String pattern, Object... params) {
        writeLog(pattern);
        return true;
    }

    @Override
    public boolean e(String pattern, Object... params) {
        writeLog(pattern);
        return true;
    }

    @Override
    public void exit() {
        if (null == fos) return;
    }

    private void writeLog(String pattern) {
        byte[] data = ZlibUtils.gzip(pattern.getBytes());
        if (null == data) return;

        File file = new File(path);
        try {
            fos = new FileOutputStream(file, true);
            fos.write(data);
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
}
