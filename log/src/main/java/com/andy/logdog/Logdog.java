package com.andy.logdog;

import android.os.Process;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import org.jetbrains.annotations.NotNull;

/**
 *
 todo 多种类型数据如何序列化、反序列化？
 todo 写入其他数据类型
 todo 读取其他类型的数据
 */
public class Logdog {

    private long buffer = -1;
    static {
        System.loadLibrary("logdog");
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
    public void d(String pattern, Object... params) {
        writeLog(LogLevel.DEBUG, pattern, params);
    }
    public void i(String pattern, Object... params) {
        writeLog(LogLevel.INFO, pattern, params);
    }
    public void w(String pattern, Object... params) {
        writeLog(LogLevel.WARN, pattern, params);
    }
    public void e(String pattern, Object... params) {
        writeLog(LogLevel.ERROR, pattern, params);
    }
    private void writeLog(LogLevel logLevel, String pattern, Object... params) {
        if (null == logLevel) return;
        String content = Utils.formatStr(pattern, params);
        if (TextUtils.isEmpty(content)) return;
        writeLog(builderLogContent(logLevel.getName(), content));
    }
    private void writeLog(String log) {
        if (TextUtils.isEmpty(log)) {
            return;
        }
        if (buffer == -1) {
            throw new IllegalStateException("buffer not available, please create it");
        }
        long start = System.nanoTime();
        mmapWrite(buffer, log);
        long end = System.nanoTime();

        Log.i("Logdog", "write complete, time cost: " + (end - start));
    }

    /**
     * 格式化需要输出到文件的LOG日志内容。添加时间戳、进程、线程等信息
     * @param logLevel 日志级别 see as {@link LogLevel}
     * @param content
     * @return
     */
    @NotNull
    private String builderLogContent(String logLevel, String content) {
        int processId = Process.myPid();
        //线程ID
        int threadTid = Process.myTid();
        long threadIdJava = Thread.currentThread().getId();
        long timeStamp = System.currentTimeMillis();

        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder
                .append(Utils.formatStr("[%s]", logLevel))
                .append(Utils.formatDatetime(timeStamp)).append(" [")
                .append(processId).append(":")
                .append(threadTid).append(":")
                .append(threadIdJava).append("] ")
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

    public native void mmapWrite(long buffer, @NonNull String content);
    public native long createBuffer(@NonNull String path);
    public native String readFile(long buffer);
    public native void onExit(long buffer);
}
