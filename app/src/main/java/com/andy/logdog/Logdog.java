package com.andy.logdog;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.File;
import java.io.FileDescriptor;

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
        File file = new File(context.getApplicationContext()
                .getExternalFilesDir(null), "logdog");
        path = file.getAbsolutePath();
        native_init(path);
    }


    public void w(String path, String content) {
//        native_write(path, content);
        write_file(path, content);
    }

    public native void native_init(@NonNull String pathLog);
    public native void native_write(@NonNull String path, @NonNull String content);
    public native void write_file(@NonNull String path, @NonNull String content);
    public native String read_file(@NonNull String path);
}
