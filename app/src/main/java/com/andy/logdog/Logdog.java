package com.andy.logdog;

import android.os.Process;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

/**
 *
 todo 多种类型数据如何序列化、反序列化？
 todo 写入其他数据类型
 todo 读取其他类型的数据
 */
public class Logdog {
    public native void mmapWrite(long buffer, @NonNull String content);
    private long buffer = -1;
    static {
        System.loadLibrary("native-lib");
    }
    private static class H {
        private static final Logdog INSTANCE = new Logdog();
    }
    public static Logdog getInstance() {
        return H.INSTANCE;
    }



    private String path;
    public void init(@NonNull String path) {
        if (!TextUtils.equals(path, this.path)) {
            this.path = path;
        } else {
            if (-1 != buffer) {
                Log.w("Logdog", "buffer not need init again!");
                return;
            }
        }
        buffer = createBuffer(this.path);
    }
    public void w(
            String content) {
        if (TextUtils.isEmpty(content)) return;

        if (buffer == -1) {
            throw new IllegalStateException("buffer not available, please create it");
        }
        //进程ID
        int processId = Process.myPid();
        //线程ID
        int threadTid = Process.myTid();
        long threadIdJava = Thread.currentThread().getId();
        long timeStamp = System.currentTimeMillis();
        String contentToSave = "" +
                Utils.foramtDatetime(timeStamp) + " [" +
                processId +":"+
                threadTid+":" +
                threadIdJava + "] " +
                content+"\n";
        long start = System.nanoTime();
        mmapWrite(buffer, contentToSave);
        long end = System.nanoTime();

        Log.i("Logdog", "write complete, time cost: " + (end - start));
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


    public native long createBuffer(@NonNull String path);
    public native String readFile(long buffer);
    public native void onExit(long buffer);
}
