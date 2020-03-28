package com.andy.logdog;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class Utils {
    private static final String PATTERN_yyyyMMddHHmmssSSS = "yyyy-MM-dd HH:mm:ss：SSS";

    public static String foramtDatetime(long timestamp) {
        Date date = new Date(timestamp);
        SimpleDateFormat dateFormat = new SimpleDateFormat(PATTERN_yyyyMMddHHmmssSSS, Locale.CHINA);
        return dateFormat.format(date);
    }
}
