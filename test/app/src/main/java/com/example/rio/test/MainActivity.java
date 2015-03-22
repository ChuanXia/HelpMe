package com.example.rio.test;


import android.app.Activity;
import android.app.IntentService;
import android.os.Bundle;
import android.telephony.SmsManager;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.content.Context;
import android.os.Handler;
import java.util.UUID;
import android.content.Intent;


import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;


import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;


public class MainActivity extends Activity{

    private EditText phoneNumber;
    private EditText smsBody;
    private Button smsManagerBtn;

    private final static UUID PEBBLE_APP_UUID = UUID.fromString("58bb321a-549f-43aa-887d-1345cf6b65a2");

    private PebbleKit.PebbleDataReceiver dataReceiver;
    private Handler mHandler;

    //added
    GPSTracker gps;
    double latitude;
    double longitude;
    //added

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);



        phoneNumber = (EditText) findViewById(R.id.phoneNumber);
        String str = readFromFile();
        if(str.length() > 1){
            phoneNumber.setText(str);
        }
        //phoneNumber.setText("9194508503");
        //smsBody = (EditText) findViewById(R.id.smsBody);
        smsManagerBtn = (Button) findViewById(R.id.smsManager);
        //smsSendToBtn = (Button) findViewById(R.id.smsSIntent);
        // smsViewBtn = (Button) findViewById(R.id.smsVIntent);
        gps = new GPSTracker(MainActivity.this);
        smsManagerBtn.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {

                onClickSave();
            }
        });

        Intent msgIntent = new Intent(this, MyIntentService.class);
        startService(msgIntent);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (dataReceiver != null) {
            unregisterReceiver(dataReceiver);
            dataReceiver = null;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        final Handler handler = new Handler();
        //phoneNumber.setText("I am receiving 0");
        dataReceiver = new PebbleKit.PebbleDataReceiver(PEBBLE_APP_UUID) {
            @Override
            public void receiveData(final Context context, final int transactionId, final PebbleDictionary data) {

                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        if(gps.canGetLocation()) {
                            gps.getLocation();
                            latitude = gps.getLatitude();
                            longitude = gps.getLongitude();
                            Log.v("MEISHI","haha");


                        } else {

                            //gps.showSettingsAlert();
                        }
                        sendByPebble();
                        PebbleKit.sendAckToPebble(context, transactionId);
                    }
                });
            }
        };
        PebbleKit.registerReceivedDataHandler(this, dataReceiver);
    }

    public void sendByPebble() {
        try {
            SmsManager smsManager = SmsManager.getDefault();
            smsManager.sendTextMessage(phoneNumber.getText().toString(),
                    null,
                    "Help me! My current location: \n" + "latitude: " + latitude +"\n longitude: " + longitude,
                    null,
                    null);
            Toast.makeText(getApplicationContext(), "Help me please! My location is: \n" + "latitude: " + latitude +"\n longitude: " + longitude,
                    Toast.LENGTH_LONG).show();
        } catch (Exception ex) {
            Toast.makeText(getApplicationContext(),"Your sms has failed...",
                    Toast.LENGTH_LONG).show();
            ex.printStackTrace();
        }
    }

    private void writeToFile(String data) {
        try {
            OutputStreamWriter outputStreamWriter = new OutputStreamWriter(openFileOutput("num.txt", Context.MODE_PRIVATE));
            outputStreamWriter.write(data);
            outputStreamWriter.close();
        }
        catch (IOException e) {
            Log.e("Exception", "File write failed: " + e.toString());
        }
    }


    private String readFromFile() {

        String ret = "";

        try {
            InputStream inputStream = openFileInput("num.txt");

            if ( inputStream != null ) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
                String receiveString = "";
                StringBuilder stringBuilder = new StringBuilder();

                while ( (receiveString = bufferedReader.readLine()) != null ) {
                    stringBuilder.append(receiveString);
                }

                inputStream.close();
                ret = stringBuilder.toString();
            }
        }
        catch (FileNotFoundException e) {
            Log.e("login activity", "File not found: " + e.toString());
        } catch (IOException e) {
            Log.e("login activity", "Can not read file: " + e.toString());
        }

        return ret;
    }

    public void onClickSave() {
        String str = phoneNumber.getText().toString();
        //write(str);
        writeToFile(str);
        Log.v("MEISHI","click it");
    }


}