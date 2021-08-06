
const unsigned long UpdateInterval = 600000; // Delay between updates, in milliseconds, WU allows 500 requests per-day maximum and no more than 2.8 per-minute!
unsigned long StartTime = 0;
#define MaxReadings 15 // 96 Maximum, but practical limit is 12

//################ PROGRAM VARIABLES and OBJECTS ################
// Use your own API key by signing up for a free developer account at https://openweathermap.org/
String apikey = "c6d05c9173864d2613184393e59ee68f"; // See: https://openweathermap.org/
const char wxserver[] = "api.openweathermap.org";
//Set your location according to OWM locations
String City = "Beirut";
String Country = "LB";
String Language = "EN";
String Units = "M";
String Mode = "B";
int ForecastPeriods = 11;
const char *Timezone = "GMT0BST,M3.5.0/01,M10.5.0/02";
String webpage, Wx_Description, CompassPointer;
bool RxWeather = false, RxForecast = false;

WiFiClient wxclient;
WebServer server1(80);

struct Forecast_record_type
{
    float lat;
    float lon;
    int Dt;
    String Period;
    float Temperature;
    float Humidity;
    float High;
    float Low;
    float Rainfall;
    float Snowfall;
    String Main0;
    String Forecast0;
    String Forecast1;
    String Forecast2;
    String Description;
    String Time;
    int Dtime;
    int Timezone;
};

Forecast_record_type WxConditions[1];
Forecast_record_type WxForecast[MaxReadings];
String DayOfWeek(int unix_time)
{
    // Returns 'Mon' or 'Tue' ...
    // See http://www.cplusplus.com/reference/ctime/strftime/
    time_t tm = unix_time;
    struct tm *now_tm = localtime(&tm);
    char output[40];
    strftime(output, sizeof(output), "%a", now_tm);
    Serial.println("DayOfWeek: " + String(output));

    return String(output);
}
//#########################################################################################
String HourMinute(int unix_time)
{
    // Returns either '21:12  ' or ' 09:12pm' depending on Units mode
    // See http://www.cplusplus.com/reference/ctime/strftime/
    time_t tm = unix_time;
    struct tm *now_tm = localtime(&tm);
    char output[40];
    strftime(output, sizeof(output), "%H:%M", now_tm);

    Serial.println("HourMinute: " + String(output));

    return String(output);
}
String Weather = "";
bool DecodeWeather(WiFiClient &json, String Type)
{
    Rain_Prob = 0;
    Serial.print(F("Creating object...and "));
    DynamicJsonDocument doc((MaxReadings + 2) * 1024); // allocate memory for the JsonDocument
    DeserializationError error = deserializeJson(doc, json);
    if (error)
    { // Test if parsing succeeds.// Deserialize the JSON document
        Serial.print(F("DeserializeJson() failed: "));
        Serial.println(error.c_str());
        return false;
    }
    JsonObject root = doc.as<JsonObject>(); // convert it to a JsonObject
    Serial.println(" Decoding " + Type + " data");
    Serial.println(doc.memoryUsage());
    if (Type == "weather")
    {
        // All Serial.println statements are for diagnostic purposes and not required, remove if not needed
        WxConditions[0].lon = root["coord"]["lon"];
        WxConditions[0].lat = root["coord"]["lat"];
        WxConditions[0].Main0 = root["weather"][0]["main"].as<char *>();
        Serial.print("Main0: ");
        Serial.println(WxConditions[0].Main0);
        Weather = "Weather : " + String(WxConditions[0].Main0);
        WxConditions[0].Forecast0 = root["weather"][0]["description"].as<char *>();
        Serial.print("Fore0: ");
        Serial.println(WxConditions[0].Forecast0);
        Weather += "\nForecast : " + String(WxConditions[0].Forecast0);

        WxConditions[0].Humidity = root["main"]["humidity"];
        Serial.print("Humi : ");
        Serial.println(WxConditions[0].Humidity);
        Weather += "\nHumidity : " + String(WxConditions[0].Humidity);

        WxConditions[0].Description = root["weather"][0]["description"].as<char *>();
        Serial.print("Desc : ");
        Serial.println(WxConditions[0].Description);
        WxConditions[0].Temperature = root["main"]["temp"];
        Serial.print("Temp : ");
        Serial.println(WxConditions[0].Temperature);
        Weather += "\nTemp : " + String(WxConditions[0].Temperature);

        WxConditions[0].Low = root["main"]["temp_min"];
        Serial.print("TLow : ");
        Serial.println(WxConditions[0].Low);
        Weather += "\nTLow : " + String(WxConditions[0].Low);

        WxConditions[0].High = root["main"]["temp_max"];
        Serial.print("THigh: ");
        Serial.println(WxConditions[0].High);
        Weather += "\nTHigh : " + String(WxConditions[0].High);
    }
    if (Type == "forecast")
    {
        //Serial.println(json);
        JsonArray list = root["list"];
        Serial.print("\nReceiving Forecast period-"); //------------------------------------------------
        for (byte r = 0; r < MaxReadings; r++)
        {
            Serial.println("\nPeriod-" + String(r) + "--------------");
            WxForecast[r].Dt = list[r]["dt"];
            Serial.print("DTime: ");
            Serial.println(WxForecast[r].Dt);
            WxForecast[r].Temperature = list[r]["main"]["temp"];
            Serial.print("Temp : ");
            Serial.println(WxForecast[r].Temperature);
            WxForecast[r].Low = list[r]["main"]["temp_min"];
            Serial.print("TLow : ");
            Serial.println(WxForecast[r].Low);
            WxForecast[r].High = list[r]["main"]["temp_max"];
            Serial.print("THig : ");
            Serial.println(WxForecast[r].High);
            WxForecast[r].Forecast0 = list[r]["weather"][0]["main"].as<char *>();
            Serial.print("Fore0: ");
            Serial.println(WxForecast[r].Forecast0);
            WxForecast[r].Description = list[r]["weather"][0]["description"].as<char *>();
            Serial.print("Desc : ");
            Serial.println(WxForecast[r].Description);
            WxForecast[r].Rainfall = list[r]["rain"]["3h"];
            Serial.print("Rain : ");
            Serial.println(WxForecast[r].Rainfall);
            WxForecast[r].Snowfall = list[r]["snow"]["3h"];
            Serial.print("Snow : ");
            Serial.println(WxForecast[r].Snowfall);
            WxForecast[r].Period = list[r]["dt_txt"].as<char *>();
            Serial.print("Peri : ");
            Serial.println(WxForecast[r].Period);
            DayOfWeek(WxForecast[r].Dt);
            HourMinute(WxForecast[r].Dt);
            if (r < 5 && (WxForecast[r].Rainfall > 0 || WxForecast[r].Forecast0 == "Rain"))
            {
                Rain_Prob++;
            }
        }
        Weather += "\nRain probability : " + String(Rain_Prob * 20) + "%";
        Serial.println("-------");
        Serial.println("\nRain Prob: " + String(Rain_Prob));
        Serial.println(Weather);
        // ------------------------------------------
    }
    return true;
}
bool obtain_wx_data(WiFiClient &client, const String &RequestType)
{
    const String units = (Units == "M" ? "metric" : "imperial");
    client.stop(); // close connection before sending a new request
    HTTPClient http;
    String uri = "/data/2.5/" + RequestType + "?q=" + City + "," + Country + "&APPID=" + apikey + "&mode=json&units=" + units + "&lang=" + Language;
    if (RequestType != "weather")
    {
        uri += "&cnt=" + String(MaxReadings);
    }
    //http.begin(uri,test_root_ca); //HTTPS example connection
    //http.begin(client, "jigsaw.w3.org", 80, "/HTTP/connection.html");
    http.begin(client, wxserver, 80, uri);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        if (!DecodeWeather(http.getStream(), RequestType))
        {
            return false;
        }
        client.stop();
        http.end();
        return true;
    }
    else
    {
        Serial.printf("connection failed, error: %s", http.errorToString(httpCode).c_str());
        client.stop();
        http.end();
        return false;
    }
    http.end();
    return true;
}

void GetWxData()
{
    WiFiClient client; // wifi client object
    byte Attempts = 1;
    RxWeather = false;
    RxForecast = false;
    while ((RxWeather == false || RxForecast == false) && Attempts <= 2)
    { // Try up-to twice for Weather and Forecast data
        if (RxWeather == false)
            RxWeather = obtain_wx_data(client, "weather");
        if (RxForecast == false)
            RxForecast = obtain_wx_data(client, "forecast");
        Attempts++;
    }
    if (RxWeather || RxForecast)
    {
        Wx_Description = WxConditions[0].Main0;
        if (WxConditions[0].Forecast1 != "")
            Wx_Description += ", " + WxConditions[0].Forecast1;
        if (WxConditions[0].Forecast2 != "")
            Wx_Description += ", " + WxConditions[0].Forecast2;
        Serial.println("----------------------" + String(Wx_Description) + "-------------");
    }
}

void Update_weather(int minutes, int seconds)
{
    server1.handleClient();
    if (seconds == 0 && minutes % 10 == 0)
    {
        Serial.println("Updating Wx Data");
        GetWxData();
    }
    // Serial.println(String(minutes) +":"+String(seconds));
}
String Values(String Val)
{
    if (Val == "temperature")
    {
        Serial.println(WxConditions[0].Temperature);
        return String(WxConditions[0].Temperature);
    }
    else if (Val == "Hum")
        return String(WxConditions[0].Humidity);
    return "";
}
