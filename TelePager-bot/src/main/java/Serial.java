import com.fazecast.jSerialComm.SerialPort;
import com.fazecast.jSerialComm.SerialPortDataListener;

import java.io.InputStream;
import java.nio.charset.StandardCharsets;

import static com.fazecast.jSerialComm.SerialPort.TIMEOUT_READ_BLOCKING;

public class Serial {
    public interface SerialPortEvent {
        void onReceive(String msg);
    }

    private String port;
    private int rate;
    private SerialPort serialPort;
    private SerialPortEvent portListener;

    public Serial(String port, int rate) {
        this.port = port;
        this.rate = rate;
    }

    public void setPortListener(SerialPortEvent portListener) {
        this.portListener = portListener;
    }

    public boolean connect() {
        if (port.toLowerCase().split("com").length < 2) {
            System.out.println(port + " is really correct?");
            return false;
        }
        SerialPort[] avPorts = SerialPort.getCommPorts();
        for (SerialPort avPort : avPorts) {
            if (avPort.getSystemPortPath().toLowerCase().contains(port.toLowerCase())) {
                serialPort = avPort;
                serialPort.setComPortTimeouts(TIMEOUT_READ_BLOCKING, 15000, 5000);
                break;
            }
        }
        if (serialPort == null) {
            System.out.println("Port NULL");
            return false;
        }
        if (serialPort.isOpen()) {
            System.out.println("Port is busy");
            return false;
        }
        serialPort.setBaudRate(rate);
        serialPort.openPort();
        serialPort.addDataListener(new SerialPortDataListener() {
            @Override
            public int getListeningEvents() {
                return 3;
            }

            @Override
            public void serialEvent(com.fazecast.jSerialComm.SerialPortEvent event) {
                switch (event.getEventType()) {
                    case SerialPort.LISTENING_EVENT_DATA_AVAILABLE: {
                        StringBuilder msg = new StringBuilder();
                        InputStream in = serialPort.getInputStream();
                        try {
                            for (int j = 0; j < in.available(); ++j)
                                msg.append((char) in.read());
                            System.out.print(msg);
                            if (portListener != null)
                                portListener.onReceive(msg.toString());
                            in.close();
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                        break;
                    }

                    case SerialPort.LISTENING_EVENT_DATA_WRITTEN: {
                        break;
                    }
                }
            }
        });


        return serialPort.isOpen();
    }

    public boolean sendMessage(String msg) {
        if (!serialPort.isOpen()) {
            System.out.println("Port is closed");
            return false;
        }
        if (msg == null || msg.isEmpty()) {
            System.out.println("Empty message");
            return false;
        }
        msg = msg + "\r";
        byte[] buf = msg.getBytes(StandardCharsets.UTF_8);
        serialPort.writeBytes(msg.getBytes(StandardCharsets.UTF_8), buf.length);
        return true;
    }

}
