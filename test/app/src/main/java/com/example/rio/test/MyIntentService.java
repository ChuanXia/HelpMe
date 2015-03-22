package com.example.rio.test;

import android.app.IntentService;
import android.content.Intent;
import android.content.Context;
import android.os.Handler;
import android.telephony.SmsManager;
import android.util.Log;
import android.widget.Toast;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

import java.util.UUID;


/**
 * An {@link IntentService} subclass for handling asynchronous task requests in
 * a service on a separate handler thread.
 * <p/>
 * TODO: Customize class - update intent actions, extra parameters and static
 * helper methods.
 */
public class MyIntentService extends IntentService {
    // TODO: Rename actions, choose action names that describe tasks that this
    // IntentService can perform, e.g. ACTION_FETCH_NEW_ITEMS
    private static final String ACTION_FOO = "com.example.rio.test.action.FOO";
    private static final String ACTION_BAZ = "com.example.rio.test.action.BAZ";

    // TODO: Rename parameters
    private static final String EXTRA_PARAM1 = "com.example.rio.test.extra.PARAM1";
    private static final String EXTRA_PARAM2 = "com.example.rio.test.extra.PARAM2";

    private PebbleKit.PebbleDataReceiver dataReceiver;
    private final static UUID PEBBLE_APP_UUID = UUID.fromString("58bb321a-549f-43aa-887d-1345cf6b65a2");

    //added
    GPSTracker gps;
    double latitude;
    double longitude;


    /**
     * Starts this service to perform action Foo with the given parameters. If
     * the service is already performing a task this action will be queued.
     *
     * @see IntentService
     */
    // TODO: Customize helper method
    public static void startActionFoo(Context context, String param1, String param2) {
        Intent intent = new Intent(context, MyIntentService.class);
        intent.setAction(ACTION_FOO);
        intent.putExtra(EXTRA_PARAM1, param1);
        intent.putExtra(EXTRA_PARAM2, param2);
        context.startService(intent);
    }

    /**
     * Starts this service to perform action Baz with the given parameters. If
     * the service is already performing a task this action will be queued.
     *
     * @see IntentService
     */
    // TODO: Customize helper method
    public static void startActionBaz(Context context, String param1, String param2) {
        Intent intent = new Intent(context, MyIntentService.class);
        intent.setAction(ACTION_BAZ);
        intent.putExtra(EXTRA_PARAM1, param1);
        intent.putExtra(EXTRA_PARAM2, param2);
        context.startService(intent);
    }

    public MyIntentService() {
        super("MyIntentService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        final Handler handler = new Handler();
        Log.v("BG","In onhandle");
        //phoneNumber.setText("I am receiving 0");
        dataReceiver = new PebbleKit.PebbleDataReceiver(PEBBLE_APP_UUID) {
            @Override
            public void receiveData(final Context context, final int transactionId, final PebbleDictionary data) {

                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        sendByPebble();
                        PebbleKit.sendAckToPebble(context, transactionId);
                    }
                });
            }
        };
        PebbleKit.registerReceivedDataHandler(this, dataReceiver);
    }

    /**
     * Handle action Foo in the provided background thread with the provided
     * parameters.
     */
    private void handleActionFoo(String param1, String param2) {
        // TODO: Handle action Foo
        throw new UnsupportedOperationException("Not yet implemented");
    }

    /**
     * Handle action Baz in the provided background thread with the provided
     * parameters.
     */
    private void handleActionBaz(String param1, String param2) {
        // TODO: Handle action Baz
        throw new UnsupportedOperationException("Not yet implemented");
    }

    public void sendByPebble() {
        try {
            Log.v("BG","GOOD NEWS!");
            SmsManager smsManager = SmsManager.getDefault();
            smsManager.sendTextMessage("9194508503",
                    null,
                    "Help me (bg)! My current location: \n" + "latitude: " + latitude +"\n longitude: " + longitude,
                    null,
                    null);
            Toast.makeText(getApplicationContext(), "Help me please! My location is: \n" + "latitude: " + latitude + "\n longitude: " + longitude,
                    Toast.LENGTH_LONG).show();
        } catch (Exception ex) {
            Toast.makeText(getApplicationContext(),"Your sms has failed...",
                    Toast.LENGTH_LONG).show();
            ex.printStackTrace();
        }
    }
}
