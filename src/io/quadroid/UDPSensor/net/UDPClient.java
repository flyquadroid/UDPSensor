package io.quadroid.UDPSensor.net;

import java.io.IOException;
import java.net.*;

/**
 * UDPSensor
 * User: Matthias Nagel
 * Date: 26.11.13
 * http://www.quadroid.io
 */
public class UDPClient {

    private static UDPClient mInstance;
    private InetAddress mInetAddress;
    private int mPort;
    private DatagramSocket mDatagramSocket;
    private DatagramPacket mDatagramPacket;
    private String bufferString;

    public static UDPClient getInstance() throws SocketException {

        if (mInstance == null) {
            mInstance = new UDPClient();
        }

        return mInstance;
    }

    private UDPClient() throws SocketException {
        this.mDatagramSocket = new DatagramSocket();
        this.bufferString = "";
    }

    public void sendPacket(long timestamp, float a0, float a1, float a2, float g0, float g1, float g2, float m0, float m1, float m2, float b) {

        bufferString = String.valueOf(timestamp);
        bufferString += "; ";
        bufferString += String.format("%.4f", a0);
        bufferString += "; ";
        bufferString += String.format("%.4f", a1);
        bufferString += "; ";
        bufferString += String.format("%.4f", a2);
        bufferString += "; ";
        bufferString += String.format("%.4f", g0);
        bufferString += "; ";
        bufferString += String.format("%.4f", g1);
        bufferString += "; ";
        bufferString += String.format("%.4f", g2);
        bufferString += "; ";
        bufferString += String.format("%.2f", m0);
        bufferString += "; ";
        bufferString += String.format("%.2f", m1);
        bufferString += "; ";
        bufferString += String.format("%.2f", m2);
        bufferString += "; ";
        bufferString += String.format("%.4f", b);

        bufferString = bufferString.replace(",", ".");

        this.mDatagramPacket = new DatagramPacket(this.bufferString.getBytes(), this.bufferString.getBytes().length, this.mInetAddress, this.mPort);
        try {
            this.mDatagramSocket.send(this.mDatagramPacket);
        } catch (IOException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        }
    }

    public void setInetAddress(String inetAddress) throws UnknownHostException {
        this.mInetAddress = InetAddress.getByName(inetAddress);
    }

    public void setPort(int port) {
        this.mPort = port;
    }
}