String processor(const String &var)
{
    // Serial.println(var);
    if (var == "Last_Update")
    {
        return (String(Time) + String(Date));
    }
    else if (var == "Temp")
    {
        return String(bmp.readTemperature());
    }
    else if (var == "Battery")
    {
        return String(BattLevel());
    }
    else if (var == "Hum")
    {
        return String(Values("Hum"));
    }
    else if (var == "Pressure")
    {
        return String(bmp.readPressure());
    }
    else if (var == "Altitude")
    {
        return String(abs(bmp.readAltitude()));
    }
    else if (var == "PumpTime")
    {
        return String(Pump_status ? Remain_Pump_ON : Remain_Pump_OFF);
    }
    else if (var == "WP")
    {
        return String(Pump_status ? "ON" : "OFF");
    }

    else
        return "";
}

void Server() // Update values in background
{
    server.on("/TimeUpdate", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", (String(Time) + String(Date)).c_str());
    });
    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", processor("Temp").c_str());
    });
    server.on("/Battery", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", processor("Battery").c_str());
    });
    server.on("/Humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", processor("Hum").c_str());
    });
    server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", processor("Pressure").c_str());
    });
    server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", processor("Altitude").c_str());
    });
    server.on("/Pump_Time", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", processor("PumpTime").c_str());
    });
    server.on("/WP", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", processor("WP").c_str());
    });
    // Start server
    server.begin();
}

void File_System()
{
    if (!SPIFFS.begin())
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->authenticate(http_username, http_password))
            return request->requestAuthentication();
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/clock", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/clock.png", "image/png");
    });
}
