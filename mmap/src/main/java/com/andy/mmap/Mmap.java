package com.andy.mmap;

import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.Keep;
import androidx.annotation.LongDef;
import androidx.annotation.NonNull;

import java.math.BigDecimal;

@Keep
public class Mmap {
    private long buffer = -1;
    static {
        System.loadLibrary("mmap-lib");
    }
    private static class H {
        private static final Mmap INSTANCE = new Mmap();
    }
    public static Mmap getInstance() {
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

    public boolean save(String pattern, Object... params) {
        if (TextUtils.isEmpty(pattern)) return false;
        String content = Utils.formatStr(pattern, params);
        if (TextUtils.isEmpty(content)) return false;
        return save(builderLogContent(content));
    }

    public boolean save(String log) {
        if (TextUtils.isEmpty(log)) {
            return false;
        }
        if (buffer == -1) {
            throw new IllegalStateException("buffer not available, please create it");
        }
//        long start = System.nanoTime();
        boolean success = mmapWrite(buffer, log);
//        long end = System.nanoTime();
//        Log.d("Logdog", "time cost invoke jni mmap.mmapWrite" + (end - start)/1000);

//        Log.i("Logdog", "write complete, time cost: " + (end - start));

        return success;
    }

    /**
     * 格式化需要输出到文件的LOG日志内容。添加时间戳、进程、线程等信息
     * @param content
     * @return
     */
    @NonNull
    private String builderLogContent(String content) {
        long timeStamp = System.currentTimeMillis();

        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder
                .append(Utils.formatDatetime(timeStamp))
                .append(content)
                .append("\n");
        return stringBuilder.toString();
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

    public native boolean mmapWrite(long buffer, @NonNull String content);
    public native long createBuffer(@NonNull String path);
    public native String readFile(long buffer);
    public native void onExit(long buffer);
}
