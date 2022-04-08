import org.telegram.telegrambots.meta.TelegramBotsApi;
import org.telegram.telegrambots.meta.api.objects.Update;
import org.telegram.telegrambots.meta.exceptions.TelegramApiException;
import org.telegram.telegrambots.updatesreceivers.DefaultBotSession;

public class Main {
    static String chatId;


    public static void main(String[] args) {
        PagerBot bot = new PagerBot();
        Serial serial = new Serial("COM4", 9600);

        bot.setEventListener(new PagerBot.BotReceive() {
            @Override
            public void onNewEvent(Update update) {
                chatId = update.getMessage().getChatId().toString();
                if (update.hasMessage() && update.getMessage().hasText()) {
                    serial.sendMessage(update.getMessage().getText());
                }
            }
        });

        try {
            TelegramBotsApi botsApi = new TelegramBotsApi(DefaultBotSession.class);
            botsApi.registerBot(bot);
        } catch (TelegramApiException e) {
            e.printStackTrace();
        }


        serial.setPortListener(new Serial.SerialPortEvent() {
            @Override
            public void onReceive(String msg) {
                if (chatId != null && msg.contains("Message"))
                    bot.sendMessage(chatId, "Message sent...");
            }
        });

        if (serial.connect()) {
            System.out.println("Port opened");
        }
    }


}