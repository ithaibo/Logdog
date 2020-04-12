package com.andy.logdog;

import android.os.Process;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import com.andy.mmap.Mmap;
import com.andy.mmap.Utils;

import org.jetbrains.annotations.NotNull;

import java.util.Objects;

/**
 * todo 多种类型数据如何序列化、反序列化？
 * todo 写入其他数据类型
 * todo 读取其他类型的数据
 */
public class Logdog {

    private static class H {
        private static final Logdog INSTANCE = new Logdog();
    }

    public static Logdog getInstance() {
        return H.INSTANCE;
    }

    private Mmap mmap;
    private String path;

    /**
     * TODO 重构：这里需要设计创建一个新的Logdog；取消单利模式
     * @param path
     */
    public void init(@NonNull String path) {
        if (!TextUtils.equals(path, this.path)) {
            this.path = path;
        } else {
            if (null != mmap) {
                Log.w("Logdog", "buffer not need init again!");
                return;
            }
        }
        mmap = new Mmap(path);
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
        Objects.requireNonNull(mmap, "Mmap == null");
        if (TextUtils.isEmpty(log)) {
            Log.w("Logdog", "content of log is empty!");
            return;
        }
        long start = System.nanoTime();
        mmap.save(log);
        long end = System.nanoTime();

        Log.i("Logdog", "write complete, time cost: " + (end - start));
    }

    /**
     * 格式化需要输出到文件的LOG日志内容。添加时间戳、进程、线程等信息
     *
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
        Objects.requireNonNull(mmap, "Mmap == null");
        return mmap.read();
    }

    public void release() {
        if (null == mmap) return;

    }
}
