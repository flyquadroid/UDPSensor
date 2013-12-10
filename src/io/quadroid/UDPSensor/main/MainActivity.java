package io.quadroid.UDPSensor.main;

import android.app.Activity;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;
import io.quadroid.UDPSensor.R;
import io.quadroid.UDPSensor.helper.IPAddressValidator;
import io.quadroid.UDPSensor.ndk.QuadroidLib;
import io.quadroid.UDPSensor.net.UDPClient;

import java.io.IOException;
import java.net.SocketException;
import java.net.UnknownHostException;

/**
 * UDPSensor
 * User: Matthias Nagel
 * Date: 26.11.13
 * http://www.quadroid.io
 */
public class MainActivity extends Activity {

    private static TextView mAccelerometerX;
    private static TextView mAccelerometerY;
    private static TextView mAccelerometerZ;
    private static TextView mGyroscopeX;
    private static TextView mGyroscopeY;
    private static TextView mGyroscopeZ;
    private static TextView mMagnetometerX;
    private static TextView mMagnetometerY;
    private static TextView mMagnetometerZ;
    private static TextView mBarometer;
    private static TextView mTimestamp;
    private static TextView mSentPacketsTextView;
    private static MainActivity mMainActivity;
    private static long mTime = 0;
    private static float mA0 = 0.0f;
    private static float mA1 = 0.0f;
    private static float mA2 = 0.0f;
    private static float mG0 = 0.0f;
    private static float mG1 = 0.0f;
    private static float mG2 = 0.0f;
    private static float mM0 = 0.0f;
    private static float mM1 = 0.0f;
    private static float mM2 = 0.0f;
    private static float mB = 0.0f;
    private static long sentPackets = 0;
    private static boolean sendActive = false;
    private EditText mIPAddressText;
    private EditText mPortText;
    private Thread ndk;

    public static void onValuesEventChanged(long timestamp, float a0, float a1, float a2, float g0, float g1, float g2, float m0, float m1, float m2, float b) {

        mTime = timestamp;
        mA0 = a0;
        mA1 = a1;
        mA2 = a2;
        mG0 = g0;
        mG1 = g1;
        mG2 = g2;
        mM0 = m0;
        mM1 = m1;
        mM2 = m2;
        mB = b;

        if (sendActive) {
            try {
                UDPClient.getInstance().sendPacket(timestamp, a0, a1, a2, g0, g1, g2, m0, m1, m2, b);
                sentPackets++;
            } catch (IOException e) {
                e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
            }
        }

        mMainActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAccelerometerX.setText(String.format("%.3f", mA0));
                mAccelerometerY.setText(String.format("%.3f", mA1));
                mAccelerometerZ.setText(String.format("%.3f", mA2));
                mGyroscopeX.setText(String.format("%.3f", mG0));
                mGyroscopeY.setText(String.format("%.3f", mG1));
                mGyroscopeZ.setText(String.format("%.3f", mG2));
                mMagnetometerX.setText(String.format("%.3f", mM0));
                mMagnetometerY.setText(String.format("%.3f", mM1));
                mMagnetometerZ.setText(String.format("%.3f", mM2));
                mBarometer.setText(String.format("%.3f", mB));
                mTimestamp.setText(String.valueOf(mTime));
                mSentPacketsTextView.setText(String.valueOf(sentPackets));
            }
        });
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mMainActivity = this;

        mAccelerometerX = (TextView) findViewById(R.id.accelerometerX);
        mAccelerometerY = (TextView) findViewById(R.id.accelerometerY);
        mAccelerometerZ = (TextView) findViewById(R.id.accelerometerZ);

        mGyroscopeX = (TextView) findViewById(R.id.gyroscopeX);
        mGyroscopeY = (TextView) findViewById(R.id.gyroscopeY);
        mGyroscopeZ = (TextView) findViewById(R.id.gyroscopeZ);

        mMagnetometerX = (TextView) findViewById(R.id.magnetometerX);
        mMagnetometerY = (TextView) findViewById(R.id.magnetometerY);
        mMagnetometerZ = (TextView) findViewById(R.id.magnetometerZ);

        mBarometer = (TextView) findViewById(R.id.barometer);

        mTimestamp = (TextView) findViewById(R.id.timestamp);

        mSentPacketsTextView = (TextView) findViewById(R.id.sentPacketsText);

        mIPAddressText = (EditText) findViewById(R.id.ipAddressText);
        mPortText = (EditText) findViewById(R.id.portText);

        setUDPValues();

        ndk = new Thread(new Runnable() {
            public void run() {
                QuadroidLib.init();
            }
        });
        ndk.start();
    }

    public void onToggle(View view) {

        ConnectivityManager connManager = (ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE);
        NetworkInfo mWifi = connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);

        if (mWifi.isConnected()) {

            if (setUDPValues()) {
                sendActive = ((ToggleButton) view).isChecked();
            }

            if (sendActive) {
                sentPackets = 0;
                mIPAddressText.setEnabled(false);
                mPortText.setEnabled(false);
                ((ToggleButton) view).setChecked(true);
            } else {
                mIPAddressText.setEnabled(true);
                mPortText.setEnabled(true);
                ((ToggleButton) view).setChecked(false);
            }

        } else {
            Toast.makeText(this, "Please connect to Wi-Fi", 3000).show();
            ((ToggleButton) view).setChecked(false);
        }
    }

    private boolean setUDPValues() {

        try {
            IPAddressValidator ipAddressValidator = new IPAddressValidator();
            String url = mIPAddressText.getText().toString();

            if (ipAddressValidator.validate(url)) {
                UDPClient.getInstance().setInetAddress(url);

                Integer port = Integer.valueOf(mPortText.getText().toString());

                if (port >= 0 && port <= 65535) {
                    UDPClient.getInstance().setPort(port);
                    return true;
                } else {

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(mMainActivity, "No valid Port", 3000).show();
                        }
                    });
                }
            } else {

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(mMainActivity, "No valid IP address", 3000).show();
                    }
                });
            }

        } catch (UnknownHostException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        } catch (SocketException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        }

        return false;
    }

    public void onQuadroidButton(View view) {
        Uri uriUrl = Uri.parse("http://www.quadroid.io");
        Intent launchBrowser = new Intent(Intent.ACTION_VIEW, uriUrl);
        startActivity(launchBrowser);
    }
}
