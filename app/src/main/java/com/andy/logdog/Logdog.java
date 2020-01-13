package com.andy.logdog;

import android.content.Context;

import androidx.annotation.NonNull;

import java.io.File;
import java.io.FileDescriptor;

public class Logdog {
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
        String fileName = String.valueOf(System.currentTimeMillis());
        String path = context.getApplicationContext()
                .getExternalFilesDir(String.format("logdog/%s.log", fileName))
                .getAbsolutePath();
        native_init(path);
    }


    public void w(String path, String content) {
        native_write(path, content);
    }

    public native void native_init(@NonNull String pathLog);
    public native void native_write(@NonNull String path, @NonNull String content);
}
