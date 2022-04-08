package ru.example.notificationstopager

import android.content.Context
import android.content.Intent
import android.service.notification.NotificationListenerService
import android.service.notification.StatusBarNotification
import androidx.localbroadcastmanager.content.LocalBroadcastManager


class NotificationService : NotificationListenerService() {
    var context: Context? = null
    var title: String = ""
    var text: String = ""

    override fun onCreate() {
        super.onCreate()
        context = applicationContext
    }

    override fun onListenerConnected() {
        super.onListenerConnected()
        for (noti in getActiveNotifications()){
            onNotificationPosted(noti)
        }
    }

    override fun onNotificationPosted(sbn: StatusBarNotification) {
        title = ""
        text = ""
        val pack = sbn.packageName
        var ticker = ""
        if (sbn.notification.tickerText != null) {
            ticker = sbn.notification.tickerText.toString()
        }

        title = sbn.notification.extras?.getString("android.title") ?: ""
        text = sbn.notification.extras?.getString("android.text") ?: ""

        if(title.isEmpty() && text.isEmpty())
            return

        val msgrcv = Intent("Msg")
        with(msgrcv) {
            putExtra("package", pack)
            putExtra("ticker", ticker)
            putExtra("title", title)
            putExtra("text", text)
        }


        LocalBroadcastManager.getInstance(context!!).sendBroadcast(msgrcv)
    }

    override fun onNotificationRemoved(sbn: StatusBarNotification) {

    }

}