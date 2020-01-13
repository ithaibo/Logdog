package com.andy.logdog;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);

        File file = new File(getExternalFilesDir(null), "logdog");
        String path = file.getAbsolutePath();
        Log.i("Main", "path: " + path);

        Logdog.getInstance().w(path, "Main path: " + path);
    }
}
