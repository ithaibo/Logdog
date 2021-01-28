package com.andy.logdog.sample.logdog;

import android.os.Process;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import com.andy.log.ILogger;
import com.andy.log.LogLevel;
import com.andy.mmap.Mmap;
import com.andy.mmap.Utils;


import java.util.Objects;

/**
 * todo 多种类型数据如何序列化、反序列化？
 * todo 写入其他数据类型
 * todo 读取其他类型的数据
 */
public class Logdog implements ILogger {

    private static class H {
        private static final Logdog INSTANCE = new Logdog();
    }

    public static Logdog getInstance() {
        return H.INSTANCE;
    }

    private Mmap mmap;
    private String path;

    public synchronized boolean init(@NonNull String path) {
        if (!TextUtils.equals(path, this.path)) {
            this.path = path;
        } else {
            if (null != mmap) {
                Log.w("Logdog", "buffer not need init again!");
                return false;
            }
        }
        mmap = Mmap.getInstance();
        mmap.init(path);
        return true;
    }

    public boolean d(String pattern, Object... params) {
        return writeLog(LogLevel.DEBUG, pattern, params);
    }

    public boolean i(String pattern, Object... params) {
        writeLog(LogLevel.INFO, pattern, params);
        return true;
    }

    public boolean w(String pattern, Object... params) {
        writeLog(LogLevel.WARN, pattern, params);
        return true;
    }

    public boolean e(String pattern, Object... params) {
        writeLog(LogLevel.ERROR, pattern, params);
        return true;
    }

    @Override
    public void exit() {
        release();
    }

    private boolean writeLog(LogLevel logLevel, String pattern, Object... params) {
//        if (null == logLevel) return;
//        String content = Utils.formatStr(pattern, params);
//        if (TextUtils.isEmpty(content)) return;
        return writeLog(pattern);
//     todo   writeLog(builderLogContent(logLevel.getName(), content));
    }

    private boolean writeLog(String log) {
        if (null == mmap) return false;
        if (TextUtils.isEmpty(log)) {
            Log.w("Logdog", "content of log is empty!");
            return false;
        }
        boolean saved = mmap.save(log);
        return saved;
//        Log.i("Logdog", "write complete, time cost: " + (end - start));
    }

    /**
     * 格式化需要输出到文件的LOG日志内容。添加时间戳、进程、线程等信息
     *
     * @param logLevel 日志级别 see as {@link LogLevel}
     * @param content
     * @return
     */
    @SuppressWarnings("StringBufferReplaceableByString")
    @NonNull
    private String builderLogContent(String logLevel, String content) {
        int processId = Process.myPid();
        //线程ID
        int threadTid = Process.myTid();
        long threadIdJava = Thread.currentThread().getId();
        long timeStamp = System.currentTimeMillis();

        return new StringBuilder()
                .append(Utils.formatStr("[%s]", logLevel))
                .append(Utils.formatDatetime(timeStamp)).append(" [")
                .append(processId).append(":")
                .append(threadTid).append(":")
                .append(threadIdJava).append("] ")
                .append(content)
                .append("\n")
                .toString();
    }

    private synchronized void release() {
        if (null == mmap) return;
        mmap = null;
    }
}
