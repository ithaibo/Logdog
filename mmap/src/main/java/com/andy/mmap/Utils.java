package com.andy.mmap;

import android.text.TextUtils;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class Utils {
    private static final String PATTERN_yyyyMMddHHmmssSSS = "yyyy-MM-dd HH:mm:ss：SSS";

    public static final String PATTERN_yyyyMMdd = "yyyyMMdd";

    public static String formatDatetime(long timestamp) {
        Date date = new Date(timestamp);
        SimpleDateFormat dateFormat = new SimpleDateFormat(PATTERN_yyyyMMddHHmmssSSS, Locale.CHINA);
        return dateFormat.format(date);
    }


    public static String formatDateNow() {
        return formatDateNow(PATTERN_yyyyMMdd);
    }
    public static String formatDateNow(String pattern) {
        Date date = new Date(System.currentTimeMillis());
        SimpleDateFormat dateFormat = new SimpleDateFormat(pattern, Locale.CHINA);
        return dateFormat.format(date);
    }

    public static String formatStr(String pattern, Object... params) {
        if (TextUtils.isEmpty(pattern)) return null;
        try {
            return String.format(pattern, params);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}
