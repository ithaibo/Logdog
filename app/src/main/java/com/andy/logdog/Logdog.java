package com.andy.logdog;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;

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
        mmap_write(path, content);
        Log.i("Logdog", "write complete");
    }

    public native void native_init(@NonNull String pathLog);
    public native void mmap_write(@NonNull String path, @NonNull String content);
    public native String readFile(@NonNull String path);
    public native void printBase64(@NonNull String content);


    public void mmapW(@NonNull String path, @NonNull String content) {
        try {
            RandomAccessFile randomAccessFile = new RandomAccessFile(path, "r");
            FileChannel fileChannel = randomAccessFile.getChannel();
            long size = fileChannel.size();
            MappedByteBuffer buffer = fileChannel.map(FileChannel.MapMode.READ_WRITE, size, content.getBytes().length);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
