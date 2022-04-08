import org.telegram.telegrambots.bots.TelegramLongPollingBot;
import org.telegram.telegrambots.meta.api.methods.send.SendMessage;
import org.telegram.telegrambots.meta.api.objects.Update;
import org.telegram.telegrambots.meta.exceptions.TelegramApiException;

public class PagerBot extends TelegramLongPollingBot {

    public interface BotReceive {
        void onNewEvent(Update update);
    }

    BotReceive eventListener;

    public void setEventListener(BotReceive eventListener) {
        this.eventListener = eventListener;
    }

    @Override
    public void onUpdateReceived(Update update) {
        if (eventListener != null)
            eventListener.onNewEvent(update);
        // We check if the update has a message and the message has text
        if (update.hasMessage() && update.getMessage().hasText()) {
            SendMessage message = new SendMessage(); // Create a SendMessage object with mandatory fields
            message.setChatId(update.getMessage().getChatId().toString());
            message.setText(update.getMessage().getText());

            System.out.println(update.getMessage().getText());
        }


    }

    public void sendMessage(String chatId, String msg){
        try {
            SendMessage message = new SendMessage(); // Create a SendMessage object with mandatory fields
            message.setChatId(chatId);
            message.setText(msg);
            execute(message);
        } catch (TelegramApiException e) {
            e.printStackTrace();
        }
    }

    @Override
    public String getBotUsername() {
        return "Your Bot Name";
    }

    @Override
    public String getBotToken() {
        return "Paste your token";
    }
}