package ru.example.notificationstopager

import android.Manifest
import android.Manifest.permission.BLUETOOTH_CONNECT
import android.app.NotificationChannel
import android.app.NotificationManager
import android.bluetooth.BluetoothDevice
import android.content.*
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import me.aflak.bluetooth.Bluetooth
import me.aflak.bluetooth.interfaces.DeviceCallback
import ru.example.notificationstopager.databinding.ActivityMainBinding
import java.net.URLEncoder
import java.nio.charset.Charset
import java.nio.charset.StandardCharsets
import java.util.*

class MainActivity : AppCompatActivity() {
    lateinit var binding: ActivityMainBinding;

    var capV: Long = 0
    var freqV: Long = 0
    var invV: Boolean = false
    lateinit var prefs: SharedPreferences

    //val MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB") //для HC-05, на HC-06/04 возможно другой UUID
    val bluetooth = Bluetooth(this);


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        prefs = getSharedPreferences("prefs", MODE_PRIVATE)

        capV = prefs.getLong("cap", 0)
        freqV = prefs.getLong("freq", 0)
        invV = prefs.getBoolean("inv", false)
        with(binding) {

            cap.setText(if (capV > 0) capV.toString() else "")
            freq.setText(if (freqV > 0) freqV.toString() else "")
            inv.isChecked = invV
        }

        binding.save.setOnClickListener {
            with(prefs.edit()) {
                putLong("cap", binding.cap.text.toString().toLong()).apply()
                putLong("freq", binding.freq.text.toString().toLong()).apply()
                putBoolean("inv", binding.inv.isChecked).apply()
            }
        }

        binding.chPerm.setOnClickListener {
            reqPermissions()
        }

        binding.testNotif.setOnClickListener {
            createNotif()
        }

        LocalBroadcastManager.getInstance(this).registerReceiver(onNotice, IntentFilter("Msg"));

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (checkSelfPermission(Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED) {

            }
        }
        btInit()
//        startService(Intent(this,NotificationService::class.java))
    }

    fun btInit() {
        bluetooth.onStart();
        bluetooth.setDeviceCallback(object : DeviceCallback {
            override fun onDeviceConnected(device: BluetoothDevice?) {

            }

            override fun onDeviceDisconnected(device: BluetoothDevice?, message: String?) {

            }

            override fun onMessage(message: ByteArray?) {

            }

            override fun onError(errorCode: Int) {

            }

            override fun onConnectError(device: BluetoothDevice?, message: String?) {

            }
        })

        bluetooth.connectToName("BT Super");

    }

    fun sendToBt(msg: String) {

        if (bluetooth.isConnected)
            bluetooth.send(msg, StandardCharsets.UTF_8)
    }

    fun createNotif() {
        var builder = NotificationCompat.Builder(this, "sdfsdf")
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setContentTitle("Важное событие")
            .setContentText("Тестируем пейджер...")
            .setPriority(NotificationCompat.PRIORITY_HIGH)

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "Notifi"
            val descriptionText = "Some text"
            val importance = NotificationManager.IMPORTANCE_DEFAULT
            val channel = NotificationChannel("sdfsdf", name, importance).apply {
                description = descriptionText
            }
            // Register the channel with the system
            val notificationManager: NotificationManager =
                getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(channel)
        }

        with(NotificationManagerCompat.from(this)) {
            // notificationId is a unique int for each notification that you must define
            notify(123, builder.build())
        }
    }


    private fun reqPermissions(): Boolean {
        startActivity(Intent("android.settings.ACTION_NOTIFICATION_LISTENER_SETTINGS"))

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            ActivityCompat.requestPermissions(
                this, listOf(
                    Manifest.permission.BLUETOOTH_SCAN,
                    BLUETOOTH_CONNECT
                ).toTypedArray(), 100
            )
        }


        return false
    }

    private val onNotice: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {

            //      val title = Translit.cyr2lat(intent.getStringExtra("title"))
            //      val text = Translit.cyr2lat(intent.getStringExtra("text"))
            val title = intent.getStringExtra("title")
            val text = intent.getStringExtra("text")
            Log.e("messg", title.plus(", ").plus(text))
            sendToBt("send f $freqV c $capV e 1 i ${if (invV) 1 else 0} m $title: $text\r")


        }
    }
}