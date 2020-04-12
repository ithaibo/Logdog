package com.andy.mmap;

import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

public class Mmap {
    static {
        System.loadLibrary("mmap-lib");
    }

    private long pointerBufferMap = -1;
    private String path;

    public Mmap(String path) {
        this.path = path;
        init(path);
    }

    public void save(String pattern, Object... params) {
        if (TextUtils.isEmpty(pattern)) return;
        String content = Utils.formatStr(pattern, params);
        if (TextUtils.isEmpty(content)) return;
        save(builderLogContent(content));
    }
    public String read() {
        if (pointerBufferMap == -1) {
            throw new IllegalStateException("pointerBufferMap not available, please create it");
        }
        return readFile(pointerBufferMap);
    }
    public void release() {
        if (-1 == pointerBufferMap) return;
        onExit(pointerBufferMap);
        pointerBufferMap = -1;
    }

    /**执行内存映射，并返回映射的相关指针（C++ Buffer）*/
    public native long createBuffer(@NonNull String path);
    /**写入内容（追加方式）*/
    public native void mmapWrite(long buffer, @NonNull String content);
    /**读取整个文件*/
    public native String readFile(long buffer);
    /**退出，释放已映射的内存缓冲区*/
    public native void onExit(long buffer);


    //TODO 增加随机访问某一区间的数据功能
    //TODO 增加指定起始位置，修改指定长度的内容


    /**
     * 创建内存映射
     * @param path 映射的文件路径
     */
    private void init(@NonNull String path) {
        if (!TextUtils.equals(path, this.path)) {
            this.path = path;
        } else {
            if (-1 != pointerBufferMap) {
                Log.w("Logdog", "pointerBufferMap not need init again!");
                return;
            }
        }
        pointerBufferMap = createBuffer(this.path);
    }

    /**
     * 格式化需要输出到文件的LOG日志内容。添加时间戳、进程、线程等信息
     * @param content
     * @return
     */
    @NonNull
    private String builderLogContent(String content) {
        return content + "\n";
    }

    private void save(String log) {
        if (TextUtils.isEmpty(log)) {
            return;
        }
        if (pointerBufferMap == -1) {
            throw new IllegalStateException("pointerBufferMap not available, please create it");
        }
        long start = System.nanoTime();
        mmapWrite(pointerBufferMap, log);
        long end = System.nanoTime();

        Log.i("Logdog", "write complete, time cost: " + (end - start));
    }
}
