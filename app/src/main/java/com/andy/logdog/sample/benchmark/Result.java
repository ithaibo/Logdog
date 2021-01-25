package com.andy.logdog.sample.benchmark;

public class Result {
    public int timesWrite;
    public String writeType;
    public long timeCost;
    public long contentLength;

    @Override
    public String toString() {
        return "Result{" +
                "timesWrite=" + timesWrite +
                ", writeType='" + writeType + '\'' +
                ", timeCost=" + timeCost +
                ", contentLength=" + contentLength +
                '}';
    }
}