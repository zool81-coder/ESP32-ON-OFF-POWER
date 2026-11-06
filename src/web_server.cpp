#include "web_server.hpp"

#include <Arduino.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <cstdarg>
#include <cstdio>
#include <Update.h>            // запись прошивки
#include <esp_task_wdt.h>      // кормление watchdog во время OTA
#include "web/update_page.hpp" // HTML страница обновления
#include "ap_manager.hpp"
#include "battery.hpp"
#include "config.hpp"
#include "log.hpp"
#include "power_controller.hpp"
#include "wifi_manager.hpp"
#include "wifi_storage.hpp"
#include "web/index_page.hpp"

namespace relay::web
{

    Server::Server(power::Controller &power, wifi::Storage &storage,
                   wifi::Manager &wifi, ap::Manager &ap, battery::Monitor &battery)
        : power_(power), storage_(storage), wifi_(wifi), ap_(ap), battery_(battery),
          server_(config::kHttpPort) {}

    void Server::begin()
    {
        registerRoutes();
        server_.begin();

        if (MDNS.begin(config::kMdnsHostname))
        {
            MDNS.addService("http", "tcp", config::kHttpPort);
            log::info("HTTP", "mDNS включён: http://%s.local", config::kMdnsHostname);
        }
        else
        {
            log::warn("HTTP", "не удалось запустить mDNS");
        }

        log::info("HTTP", "сервер запущен на порту %u",
                  static_cast<unsigned>(config::kHttpPort));
    }

    void Server::handleClient()
    {
        server_.handleClient();
    }

    void Server::restart()
    {
        server_.stop();
        server_.begin();
        log::info("HTTP", "сервер перезапущен (смена режима сети)");
    }

    void Server::registerRoutes()
    {
        server_.on("/", HTTP_GET, [this]()
                   { server_.send(200, "text/html", index_page::kHtml); });

        // --- Питание ---
        server_.on("/api/state", HTTP_GET, [this]()
                   { sendJson(200, R"({"power":%s})", power_.isOn() ? "true" : "false"); });

        server_.on("/api/power", HTTP_POST, [this]()
                   {
        if (!server_.hasArg("on")) {
            sendJson(400, R"({"ok":false,"error":"отсутствует параметр 'on'"})");
            return;
        }
        const String value = server_.arg("on");
        const bool on = (value == "1" || value == "true" || value == "on");
        power_.setOn(on);
        sendJson(200, R"({"ok":true,"power":%s})", power_.isOn() ? "true" : "false"); });

        // --- Батарея: измерить и вернуть состояние ---
        server_.on("/api/battery", HTTP_GET, [this]()
                   {
        battery_.measure();

        JsonDocument doc;
        doc["level"]      = static_cast<uint8_t>(battery_.level());
        doc["voltage_mv"] = battery_.voltageMv();
        doc["percent"]    = battery_.percent();
        sendJsonDoc(200, doc); });

        // --- Калибровка батареи: получить текущее значение ---
        server_.on("/api/battery/config", HTTP_GET, [this]()
                   {
        JsonDocument doc;
        doc["calibration"] = battery_.calibration();
        doc["default"]     = config::kBatteryCalibrationDefault;
        sendJsonDoc(200, doc); });

        // --- Калибровка батареи: сохранить новое значение ---
        server_.on("/api/battery/config", HTTP_POST, [this]()
                   {
        JsonDocument doc;
        const DeserializationError err = deserializeJson(doc, server_.arg("plain"));
        if (err) {
            sendJson(400, R"({"ok":false,"error":"неверный запрос"})");
            return;
        }

        const float cal = doc["calibration"] | 0.0f;
        if (cal < 0.1f || cal > 3.0f) {
            sendJson(400, R"({"ok":false,"error":"значение должно быть от 0.1 до 3.0"})");
            return;
        }

        battery_.setCalibration(cal);
        sendJson(200, R"({"ok":true})"); });

        // --- Wi-Fi ---
        server_.on("/api/wifi/status", HTTP_GET, [this]()
                   {
        JsonDocument doc;
        doc["connected"] = wifi_.isConnected();
        doc["ssid"]      = wifi_.isConnected() ? wifi_.currentSsid() : "";
        doc["ip"]        = wifi_.isConnected() ? WiFi.localIP().toString() : "";
        doc["rssi"]      = wifi_.isConnected() ? wifi_.rssi() : 0;
        sendJsonDoc(200, doc); });

        server_.on("/api/wifi/networks", HTTP_GET, [this]()
                   {
        JsonDocument doc;
        JsonArray arr = doc["networks"].to<JsonArray>();
        const auto& nets = storage_.networks();
        for (size_t i = 0; i < nets.size(); ++i) {
            JsonObject o = arr.add<JsonObject>();
            o["index"] = i;
            o["ssid"]  = nets[i].ssid;
        }
        sendJsonDoc(200, doc); });

        server_.on("/api/wifi/add", HTTP_POST, [this]()
                   {
        JsonDocument doc;
        const DeserializationError err = deserializeJson(doc, server_.arg("plain"));
        if (err) {
            sendJson(400, R"({"ok":false,"error":"неверный запрос"})");
            return;
        }
        const String ssid = doc["ssid"] | "";
        const String pass = doc["pass"] | "";
        if (ssid.length() == 0) {
            sendJson(400, R"({"ok":false,"error":"SSID не указан"})");
            return;
        }
        if (storage_.add(ssid, pass)) {
            sendJson(200, R"({"ok":true})");
        } else {
            sendJson(500, R"({"ok":false,"error":"не удалось сохранить"})");
        } });

        server_.on("/api/wifi/remove", HTTP_POST, [this]()
                   {
        JsonDocument doc;
        const DeserializationError err = deserializeJson(doc, server_.arg("plain"));
        if (err) {
            sendJson(400, R"({"ok":false,"error":"неверный запрос"})");
            return;
        }
        const int index = doc["index"] | -1;
        if (index < 0 || !storage_.remove(static_cast<size_t>(index))) {
            sendJson(400, R"({"ok":false,"error":"неверный индекс"})");
            return;
        }
        sendJson(200, R"({"ok":true})"); });

        // --- Точка доступа ---
        server_.on("/api/ap/status", HTTP_GET, [this]()
                   { sendJson(200, R"({"active":%s,"ssid":"%s"})",
                              ap_.isActive() ? "true" : "false", config::kApSsid); });

        server_.on("/api/ap/start", HTTP_POST, [this]()
                   {
        sendJson(200, R"({"ok":true,"message":"переключаюсь в режим точки доступа"})");
        ap_.requestStart(); });

        server_.on("/api/ap/stop", HTTP_POST, [this]()
                   {
        sendJson(200, R"({"ok":true,"message":"возвращаюсь в режим станции"})");
        ap_.requestStop(); });

        // --- Перезагрузка ---
        server_.on("/api/reboot", HTTP_POST, [this]()
                   {
        server_.send(200, "application/json", R"({"ok":true})");
        server_.client().flush();
        delay(500);
        ESP.restart(); });

        // --- Служебная информация ---
        server_.on("/api/info", HTTP_GET, [this]()
                   {
        char buffer[128];
        snprintf(buffer, sizeof(buffer),
                 R"({"uptime_ms":%lu,"free_heap":%u,"power":%s})",
                 static_cast<unsigned long>(millis()),
                 static_cast<unsigned>(ESP.getFreeHeap()),
                 power_.isOn() ? "true" : "false");
        server_.send(200, "application/json", buffer); });

        // --- Версия прошивки ---
        server_.on("/api/version", HTTP_GET, [this]()
                   { sendJson(200, R"({"version":"%s"})", config::kFirmwareVersion); });

        // ==================== ОБНОВЛЕНИЕ ПРОШИВКИ (веб-загрузчик) ====================

        // --- Страница загрузки прошивки ---
        server_.on("/update", HTTP_GET, [this]()
                   { server_.send(200, "text/html", update_page::kHtml); });

        // --- Приём файла прошивки ---
        server_.on("/update", HTTP_POST, [this]()
                   {
            server_.sendHeader("Connection", "close");
            if (Update.hasError()) {
                server_.send(200, "text/plain", "FAIL");
            } else {
                server_.send(200, "text/plain", "OK");
                server_.client().flush();
                delay(500);
                ESP.restart();
            } }, [this]()
                   {
            HTTPUpload& upload = server_.upload();

            if (upload.status == UPLOAD_FILE_START) {
                const String pass = server_.arg("password");
                if (pass != config::kOtaPassword) {
                    log::warn("OTA", "неверный пароль веб-обновления");
                    Update.abort();
                    return;
                }
                log::info("OTA", "веб-обновление начато: %s", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Update.printError(Serial);
                }
                esp_task_wdt_reset();
            }
            else if (upload.status == UPLOAD_FILE_WRITE) {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                    Update.printError(Serial);
                }
                esp_task_wdt_reset();
            }
            else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) {
                    log::info("OTA", "файл получен: %u байт",
                              static_cast<unsigned>(upload.totalSize));
                } else {
                    Update.printError(Serial);
                }
                esp_task_wdt_reset();
            } });

        server_.onNotFound([this]()
                           {
        if (ap_.isActive()) {
            server_.sendHeader("Location", "/", true);
            server_.send(302, "text/plain", "");
        } else {
            sendJson(404, R"({"ok":false,"error":"не найдено"})");
        } });
    }

    void Server::sendJson(int code, const char *fmt, ...)
    {
        char buffer[192];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        server_.send(code, "application/json", buffer);
    }

    void Server::sendJsonDoc(int code, const JsonDocument &doc)
    {
        String body;
        serializeJson(doc, body);
        server_.send(code, "application/json", body);
    }

} // namespace relay::web
