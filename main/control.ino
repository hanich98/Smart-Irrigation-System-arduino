SimpleKalmanFilter pressureKalmanFilter(1, 1, 0.00001);

Adafruit_BMP280 bmp; // I2C

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 0.5      /* Time ESP32 will go to sleep (in seconds) */
#define s_to_hour 3600         /* Convert from sec to hour */
// #define s_to_min 60 /*Convert from sec to min*/

unsigned long Total_time_sleep = TIME_TO_SLEEP * uS_TO_S_FACTOR * s_to_hour;
int Sleep_count, number_of_hours = 10;
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR bool morn = false;
int Battery, Battery_Filter, Bat_Perc;
bool Bat_Mail = false;


//--------------Mail----------------------------
#include "ESP32_MailClient.h"

#define emailSenderAccount "esp32.mcu@gmail.com"       // ESP32 email
#define emailSenderPassword "ESP32MCU@2020"            // ESP32 email password
#define emailRecipient "wajih_mechlawi@hotmail.com"    // receiver email
#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465
#define emailSubject "System Notification"

// The Email Sending data object contains config and data to send
SMTPData smtpData;
String Morning = "<div style=\"color:#2f4468;\"> <h1>Good Morning </h1> <p> Sent from ESP32 board</p> </div>";
String Good_Night = "<div style=\"color:#2f4468;\"> <h1>Good Night </h1> <p> Sent from ESP32 board</p> </div>";
String Pump_ON = "<div style=\"color:#2f4468;\"> <h1>Pump turned ON! </h1> <p> Sent from ESP32 board</p> </div>";
String Pump_OFF = "<div style=\"color:#2f4468;\"> <h1>Pump turned OFF! </h1> <p> Sent from ESP32 board</p> </div>";
String LOW_BATT = "<div style=\"color:#2f4468\"> <h1>Please have attention <br> Your battery level is 25% or less!! <br> Please Charge your system</h1> <p> Sent from ESP32 board</p> </div>";

// Callback function to get the Email sending status
void sendCallback(SendStatus info);


void Send_mail(String mail_msg)
{
    // Set the SMTP Server Email host, port, account and password
    smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

    // For library version 1.2.0 and later which STARTTLS protocol was supported,the STARTTLS will be
    // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
    //smtpData.setSTARTTLS(true);

    // Set the sender name and Email
    smtpData.setSender("ESP32", emailSenderAccount);

    // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
    smtpData.setPriority("High");

    // Set the subject
    smtpData.setSubject(emailSubject);

    // Set the message with HTML format
    if (mail_msg == "Morning")
        smtpData.setMessage(Morning, true);
    else if (mail_msg == "Good_Night")
        smtpData.setMessage(Good_Night, true);
    else if (mail_msg == "P_ON")
        smtpData.setMessage(Pump_ON, true);
    else if (mail_msg == "P_OFF")
        smtpData.setMessage(Pump_OFF, true);
    else if (mail_msg == "LOW_BATT")
        smtpData.setMessage(LOW_BATT, true);
    // Set the email message in text format (raw)
    //smtpData.setMessage("Hello World! - Sent from ESP32 board", false);

    // Add recipients, you can add more than one recipient
    smtpData.addRecipient(emailRecipient);

    smtpData.setSendCallback(sendCallback);

    //Start sending Email, can be set callback function to track the status
    if (!MailClient.sendMail(smtpData))
        Serial.println("Error sending Email, " + MailClient.smtpErrorReason());

    //Clear all data from Email object to free memory
    smtpData.empty();
}


void sendCallback(SendStatus msg)
{
    // Print the current status
    Serial.println(msg.info());

    // Do something when complete
    if (msg.success())
    {
        Serial.println("----------------");
    }
}
//-----------Pump Motor---------------

bool Pump_status = false;
int Pump_On_H = 20, Pump_On_M = 8, Pump_On_S = 0;
int Pump_OFF_H = Pump_On_H, Pump_OFF_M = Pump_On_M + 2, Pump_OFF_S = 0;
int Remain_Hour = 0, Remain_Min = 0, Remain_sec = 0;
String Remain_Pump_ON = "", Remain_Pump_OFF = "";
void Pump(int hour, int minutes, int seconds)
{
    if ((hour == Pump_On_H && minutes == Pump_On_M && seconds == Pump_On_S))
    {
        if (Rain_Prob < 2) // condition if the rain probability is less than 20% for the next 12 hours
        {
            Pump_status = true;
            digitalWrite(pump_pin, HIGH);
            Send_mail("P_ON");
        }
    }
    else if ((hour == Pump_OFF_H && minutes == Pump_OFF_M && seconds == Pump_OFF_S))
    {
        Pump_status = false;
        digitalWrite(pump_pin, LOW);
        Send_mail("P_OFF");
    }
    if (!Pump_status)
    {
        Remain_Hour = (Pump_On_H - 1) - hour;
        Remain_Hour < 0 ? Remain_Hour += 24 : Remain_Hour = Remain_Hour;
        Remain_Min = (Pump_On_M - 1) - minutes;
        if (Pump_On_H == hour && Remain_Min < 0)
        {
            Remain_Hour = 23;
        }
        Remain_Min < 0 ? Remain_Min += 59 : Remain_Min = Remain_Min;
        Remain_sec = 60 - seconds;
        // Remain_sec < 0 ? Remain_sec += 59 : Remain_sec = Remain_sec;
        Remain_Pump_OFF = String(Remain_Hour) + ":" + String(Remain_Min) + ":" + String(Remain_sec);
        // Serial.println(Remain_Pump_OFF);
        // delay(100);
    }
    else
    {
        Remain_Min = (Pump_OFF_M - 1) - minutes;
        Remain_sec = 60 - seconds;
        Remain_Pump_ON = String(Remain_Min) + ":" + String(Remain_sec);
        // Serial.println("Remain_Pump_ON");
    }
    // Serial.println(Remain_Pump_OFF);
}

//-----------Sleep------------------------

void SLEEP()
{
    esp_sleep_enable_timer_wakeup(Total_time_sleep);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Hours");
    Serial.println(bootCount);
    //esp_deep_sleep_p_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    //Serial.println("Configured all RTC Peripherals to be powered down in sleep");
    Sleep_count = number_of_hours * 12;
    if (bootCount++ == Sleep_count)
    {
        bootCount = 0;
        morn = true;
        Serial.println(morn);

    } // to sleep for 10 hours
    else
    {
        Serial.println("Going to sleep now");
        Serial.flush();
        esp_deep_sleep_start();
    }
}
void Night(int hour, int minutes, int seconds)
{
    if (morn == true)
    {
        Send_mail("Morning");
        morn = false;
        // Serial.println("Morning: " + String(morn));
    }
    // Serial.println(String(hour) + ":" + String(minutes) + ":" + String(seconds));
    if ((hour == 23 && minutes == 35 && seconds == 0) || bootCount > 0)
    {
        if (bootCount == 0) // Send mail when ESP goes to sleep for the first time
            Send_mail("Good_Night");
        SLEEP();
    }
}
//-------------Time Update-------------
void Time_Update(int hour, int minutes, int seconds)
{

    Night(hour, minutes, seconds);
    Pump(hour, minutes, seconds);
}

//-----------Battery-------------------

void batt_update()
{
    Battery = analogRead(36);
    Battery_Filter = pressureKalmanFilter.updateEstimate(Battery);
    Bat_Perc = map(Battery_Filter, 2400, 3300, 0, 100);
    Bat_Perc<0 ? Bat_Perc = 0 : Bat_Perc> 100 ? Bat_Perc = 100 : Bat_Perc = Bat_Perc;
    // Serial.println(Battery_Filter);
    // Serial.println("------");
}
String BattLevel()
{
    for (int i = 0; i < 100; i++)
    {
        batt_update();
    }
    // Serial.println(Bat_Perc);
    // Serial.println("------");
    String level = "";
    if (Bat_Perc > 75)
    {
        level = "<i class=\"fas fa-battery-full\"></i>";
        Bat_Mail = false;
    }
    else if (Bat_Perc > 50)
        level = "<i class=\"fas fa-battery-three-quarters\"></i>";
    else if (Bat_Perc > 25)
        level = "<i class=\"fas fa-battery-half\"></i>";
    else
    {
        level = "<i class=\"fas fa-battery-quarter\"></i>";
        if (Bat_Mail == false)
        {
            Serial.println("LOW_BAT");
            Send_mail("LOW_BATT");
            Bat_Mail = true;
        }
    }
    return level;
}

//--------------BMP initialize-------------------
void BMP_Setup()
{
    Serial.println(F("BMP280 test"));

    if (!bmp.begin())
    {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
        while (1)
            ;
    }

    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    // Setup Battery Reading
    for (int i = 0; i < 100; i++)
    {
        batt_update();
    }
}
