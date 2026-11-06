#include "ota_manager.hpp"

#include <ArduinoOTA.h> // OTA
#include <Arduino.h>    // millis

#include "config.hpp"
#include "log.hpp"

namespace relay::ota
{

    // Настройка ArduinoOTA.
    void Manager::begin()
    {
        ArduinoOTA.setHostname(config::kOtaHostname);
        ArduinoOTA.setPassword(config::kOtaPassword);

        // --- События OTA ---
        ArduinoOTA.onStart([this]()
                           {
        const String type = (ArduinoOTA.getCommand() == U_FLASH)
                                ? "прошивка"
                                : "файловая система (SPIFFS)";
        log::info("OTA", "начало обновления: %s", type.c_str());
        if (onStart_) onStart_(); });

        ArduinoOTA.onEnd([this]()
                         {
        log::info("OTA", "обновление завершено успешно");
        if (onEnd_) onEnd_(); });

        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                              {
        // Логируем прогресс каждые 25% (чтобы не спамить Serial).
        const unsigned int pct = (progress * 100) / total;
        if (pct % 25 == 0 && progress != total) {
            log::info("OTA", "прогресс: %u%%", static_cast<unsigned>(pct));
        } });

        ArduinoOTA.onError([this](ota_error_t error)
                           {
        switch (error) {
            case OTA_AUTH_ERROR:    log::error("OTA", "ошибка авторизации"); break;
            case OTA_BEGIN_ERROR:   log::error("OTA", "ошибка начала"); break;
            case OTA_CONNECT_ERROR: log::error("OTA", "ошибка соединения"); break;
            case OTA_RECEIVE_ERROR: log::error("OTA", "ошибка приёма"); break;
            case OTA_END_ERROR:     log::error("OTA", "ошибка завершения"); break;
            default:                log::error("OTA", "неизвестная ошибка"); break;
        }
        if (onError_) onError_(); });

        ArduinoOTA.begin();
        log::info("OTA", "обновление по воздуху готово: %s.local",
                  config::kOtaHostname);
    }

    // Вызов обработчика OTA (в loop()).
    void Manager::handle()
    {
        ArduinoOTA.handle();
    }

} // namespace relay::ota
