#include "wifi_manager.hpp"

#include <Arduino.h> // millis

#include "config.hpp"
#include "log.hpp"

namespace relay::wifi
{

    // Инициализация Wi-Fi и первая попытка подключения.
    void Manager::begin(Storage &storage)
    {
        storage_ = &storage;

        WiFi.mode(WIFI_STA);          // режим станции
        WiFi.setAutoReconnect(false); // переподключением управляем сами (много сетей)
        WiFi.persistent(false);       // не хранить настройки во flash

        rebuildCandidates();

        if (candidates_.empty())
        {
            log::error("WiFi", "нет ни одной сети для подключения");
            return;
        }

        candidateIndex_ = 0;
        connectStartMs_ = millis();
        startConnection(candidates_[candidateIndex_].ssid,
                        candidates_[candidateIndex_].pass);
        log::info("WiFi", "пробую сеть \"%s\"...",
                  candidates_[candidateIndex_].ssid.c_str());
    }

    // Главный цикл контроля соединения (вызывается из loop()).
    void Manager::update()
    {
        // Пока активен режим точки доступа — не трогаем соединение.
        if (suspended_)
        {
            return;
        }

        const bool connected = (WiFi.status() == WL_CONNECTED);

        // --- Успех: подключено ---
        if (connected)
        {
            if (status_ != Status::Connected)
            {
                status_ = Status::Connected;
                currentSsid_ = WiFi.SSID();
                const String ip = WiFi.localIP().toString();
                log::info("WiFi", "подключено к \"%s\", IP: %s",
                          currentSsid_.c_str(), ip.c_str());
                log::info("WiFi", "веб-интерфейс: http://%s", ip.c_str());
            }
            return;
        }

        // --- Соединения нет ---
        if (status_ == Status::Connected)
        {
            log::warn("WiFi", "связь потеряна, переподключаюсь...");
        }
        status_ = Status::Connecting;

        // Таймаут текущей попытки истёк — переходим к следующей сети по кругу.
        if (millis() - connectStartMs_ >= config::kWifiConnectTimeoutMs)
        {
            candidateIndex_ = (candidateIndex_ + 1) % candidates_.size();
            connectStartMs_ = millis();

            startConnection(candidates_[candidateIndex_].ssid,
                            candidates_[candidateIndex_].pass);
            log::info("WiFi", "таймаут, пробую сеть \"%s\"...",
                      candidates_[candidateIndex_].ssid.c_str());
        }
    }

    // Приостановка (вход в режим точки доступа).
    void Manager::suspend()
    {
        suspended_ = true;
        WiFi.disconnect();
        status_ = Status::Disconnected;
        currentSsid_ = "";
        log::info("WiFi", "режим станции приостановлен");
    }

    // Возобновление (выход из режима точки доступа).
    void Manager::resume()
    {
        suspended_ = false;

        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(false);
        WiFi.persistent(false);

        rebuildCandidates();

        if (candidates_.empty())
        {
            log::error("WiFi", "нет ни одной сети для подключения");
            return;
        }

        candidateIndex_ = 0;
        connectStartMs_ = millis();
        startConnection(candidates_[candidateIndex_].ssid,
                        candidates_[candidateIndex_].pass);
        log::info("WiFi", "режим станции возобновлён, пробую сеть \"%s\"...",
                  candidates_[candidateIndex_].ssid.c_str());
    }

    // Сборка списка кандидатов: сохранённые сети + дежурная в конце.
    void Manager::rebuildCandidates()
    {
        candidates_.clear();

        if (storage_)
        {
            for (const auto &n : storage_->networks())
            {
                candidates_.push_back({n.ssid, n.pass});
            }
        }

        // Дежурная (дефолтная) сеть — последний резерв.
        candidates_.push_back({config::kDefaultWifiSsid, config::kDefaultWifiPassword});
    }

    // Полный сброс текущего соединения и запуск нового.
    void Manager::startConnection(const String &ssid, const String &pass)
    {
        WiFi.disconnect();
        delay(50);
        WiFi.begin(ssid.c_str(), pass.c_str());
    }

} // namespace relay::wifi
