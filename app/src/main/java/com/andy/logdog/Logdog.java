package com.andy.logdog;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

public class Logdog {
    static String path;
    long buffer = -1;


    private static class H {
        private static final Logdog INSTANCE = new Logdog();
        // Used to load the 'native-lib' library on application startup.
        static {
            System.loadLibrary("native-lib");
        }
    }
    public static Logdog getInstance() {
        return H.INSTANCE;
    }

    public void init(@NonNull Context context) {
        buffer = createBuffer(path);
    }


    public void w(String content) {
        if (buffer == -1) {
            throw new IllegalStateException("buffer not available, please create it");
        }
        mmapWrite(buffer, content);
        Log.i("Logdog", "write complete");
    }

    public String read() {
        if (buffer == -1) {

            throw new IllegalStateException("buffer not available, please create it");
        }
        return readFile(buffer);
    }

    public void release() {
        if (-1 == buffer) return;
        onExit(buffer);
        buffer = -1;
    }

    //todo 多种类型数据如何序列化、反序列化？

    public native long createBuffer(@NonNull String path);
    public native void mmapWrite(long buffer, @NonNull String content);
    //todo 写入其他数据类型

    public native String readFile(long buffer);
    //todo 读取其他类型的数据
//    public native void printBase64(@NonNull String content);


    public native void onExit(long buffer);
}
