package com.andy.note;

import android.content.Context;
import android.util.Log;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import com.andy.logdog.sample.benchmark.Performance;
import com.andy.logdog.sample.benchmark.XLogger;
import com.andy.logdog.sample.logdog.Logdog;

import org.junit.Test;
import org.junit.runner.RunWith;

import java.io.File;

@RunWith(AndroidJUnit4.class)
public class BenchmarkTest {
    final int times = 10000;

    @Test
    public void testLogdog() {
        Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        assert("com.andy.note".equals(context.getOpPackageName()));
        String path = new File(context.getExternalFilesDir(null), "logdog").getAbsolutePath();
        Logdog.getInstance().init(path);

        Performance performance = new Performance();
        long startLogdog = System.currentTimeMillis();
        int countSuccess = 0;
        for (int i =0 ; i < times; i++) {
            boolean success = performance.logdog();
            if (!success) {
                assert false;
                break;
            }
            countSuccess++;
        }
        long endLogdog = System.currentTimeMillis();
        Log.d("Logdog", String.format("%d times write with Logdog, cost(ms):%d, times success:%d", times, (endLogdog - startLogdog), countSuccess));
//        assert true;
    }

    @Test
    public void testXLog() {
        Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        assert("com.andy.note".equals(context.getOpPackageName()));
        XLogger.getInstance().init(context.getExternalFilesDir(null).getAbsolutePath());

        Performance performance = new Performance();
        long startXLog = System.currentTimeMillis();
        for (int i =0 ; i < times; i++) {
            boolean success = performance.xlog();
            assert success;
        }
        long endXLog = System.currentTimeMillis();
        Log.d("Xlog", String.format("%d times write with XLog, cost(ms):%d", times, (endXLog - startXLog)));
        assert true;
    }
}
