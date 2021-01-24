package com.andy.logdog.sample.benchmark;

import androidx.annotation.NonNull;

import com.andy.log.ILogger;
import com.andy.note.BuildConfig;
import com.tencent.mars.xlog.Log;
import com.tencent.mars.xlog.Xlog;

public class XLogger implements ILogger {
    private static class H {
        private static final XLogger instance = new XLogger();
    }
    public static XLogger getInstance() {
        return H.instance;
    }
    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("marsxlog");
    }


    private XLogger() {
    }

    @Override
    public boolean init(@NonNull String path) {
        final String logPath = path + "/marssample/log";

        // this is necessary, or may cash for SIGBUS
        final String cachePath = path + "/xlog";

        //init xlog
        Xlog xlog = new Xlog();
        Log.setLogImp(xlog);

        if (BuildConfig.DEBUG) {
            Log.setConsoleLogOpen(false);
            Log.appenderOpen(Xlog.LEVEL_DEBUG, Xlog.AppednerModeSync, cachePath, logPath, "XLOGSAMPLE", 0);
        } else {
            Log.setConsoleLogOpen(false);
            Log.appenderOpen(Xlog.LEVEL_INFO, Xlog.AppednerModeAsync, cachePath, logPath, "XLOGSAMPLE", 0);
        }
        return true;
    }

    @Override
    public boolean d(String pattern, Object... params) {
        Log.d("", pattern);
        return true;
    }

    @Override
    public boolean i(String pattern, Object... params) {
        Log.i("", pattern);
        return true;
    }

    @Override
    public boolean w(String pattern, Object... params) {
        Log.w("", pattern);
        return true;
    }

    @Override
    public boolean e(String pattern, Object... params) {
        Log.e("", pattern);
        return true;
    }

    @Override
    public void exit() {

    }
}
