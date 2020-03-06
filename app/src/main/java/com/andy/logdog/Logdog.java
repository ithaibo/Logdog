package com.andy.logdog;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

public class Logdog {
    static String path;


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

        nativeInit(path);
    }


    public void w(String path, String content) {
        mmapWrite(path, content);
        Log.i("Logdog", "write complete");
    }

    public native void nativeInit(@NonNull String pathLog);
    public native void mmapWrite(@NonNull String path, @NonNull String content);
    public native String readFile(@NonNull String path);
    public native void printBase64(@NonNull String content);
    public native void onExit();
}
