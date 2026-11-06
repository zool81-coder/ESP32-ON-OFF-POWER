#include "ap_manager.hpp"

#include <Arduino.h> // String
#include <WiFi.h>    // softAP, softAPIP

#include "config.hpp"
#include "log.hpp"
#include "wifi_manager.hpp"

namespace relay::ap
{

    Manager::Manager(wifi::Manager &wifi) : wifi_(wifi) {}

    // Инициализация: AP включается только по запросу, поэтому пусто.
    void Manager::begin()
    {
        log::info("AP", "режим точки доступа готов (ожидание команды)");
    }

    // Отложенная команда: включить AP.
    void Manager::requestStart()
    {
        if (!active_ && !pendingStart_)
        {
            pendingStart_ = true;
        }
    }

    // Отложенная команда: выключить AP.
    void Manager::requestStop()
    {
        if (active_ && !pendingStop_)
        {
            pendingStop_ = true;
        }
    }

    // Обработка команд и DNS-запросов (вызывается в loop()).
    void Manager::update()
    {
        if (pendingStart_)
        {
            pendingStart_ = false;
            doStart();
        }
        if (pendingStop_)
        {
            pendingStop_ = false;
            doStop();
        }

        // Пока AP активна — обслуживаем DNS (captive portal).
        if (active_)
        {
            dnsServer_.processNextRequest();
        }
    }

    // Реальное включение точки доступа.
    void Manager::doStart()
    {
        wifi_.suspend(); // останавливаем режим станции

        WiFi.mode(WIFI_AP);
        const bool ok = WiFi.softAP(config::kApSsid, config::kApPassword, config::kApChannel);
        if (!ok)
        {
            log::error("AP", "не удалось запустить точку доступа");
            return;
        }

        // Captive portal: все DNS-запросы ведём на IP точки доступа.
        const IPAddress apIp = WiFi.softAPIP();
        dnsServer_.start(53, "*", apIp);

        active_ = true;

        log::info("AP", "точка доступа запущена: %s", config::kApSsid);
        log::info("AP", "IP: %s", apIp.toString().c_str());
        log::info("AP", "подключитесь к сети \"%s\" и откройте http://%s",
                  config::kApSsid, apIp.toString().c_str());
    }

    // Реальное выключение точки доступа.
    void Manager::doStop()
    {
        dnsServer_.stop();
        WiFi.softAPdisconnect(false); // false = не выключаем Wi-Fi полностью
        active_ = false;

        wifi_.resume(); // возвращаемся в режим станции

        log::info("AP", "точка доступа выключена");
    }

} // namespace relay::ap
