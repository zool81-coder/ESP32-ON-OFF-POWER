#pragma once

// =============================================================================
//  wifi_manager.hpp — неблокирующее управление Wi-Fi (STA, много сетей)
// -----------------------------------------------------------------------------
//  Последовательно пробует: пользовательские сети (новая сверху) →
//  дежурная сеть из прошивки. Умеет "приостанавливаться" на время работы
//  точки доступа (suspend/resume), чтобы не конфликтовать с режимом AP.
// =============================================================================

#include <WiFi.h> // WiFi, IPAddress
#include <vector> // std::vector
#include <cstdint>

#include "wifi_storage.hpp" // Storage

namespace relay::wifi
{

    class Manager
    {
    public:
        enum class Status : uint8_t
        {
            Disconnected = 0, // нет соединения
            Connecting = 1,   // идёт попытка подключения
            Connected = 2,    // подключено
        };

        // Инициализация: загружает сети из хранилища и начинает подключение.
        void begin(Storage &storage);

        // Вызывать в loop(): следит за состоянием, перебирает сети при обрыве.
        void update();

        // Приостановить работу (при входе в режим точки доступа).
        void suspend();

        // Возобновить работу (после выхода из режима точки доступа).
        void resume();

        // Приостановлен ли менеджер.
        [[nodiscard]] bool isSuspended() const noexcept { return suspended_; }

        [[nodiscard]] Status status() const noexcept { return status_; }
        [[nodiscard]] bool isConnected() const noexcept { return status_ == Status::Connected; }
        [[nodiscard]] IPAddress localIP() const { return WiFi.localIP(); }
        [[nodiscard]] const String &currentSsid() const { return currentSsid_; }
        [[nodiscard]] int rssi() const { return WiFi.RSSI(); }

    private:
        // Кандидат для подключения (одна сеть).
        struct Candidate
        {
            String ssid;
            String pass;
        };

        Storage *storage_ = nullptr;
        std::vector<Candidate> candidates_; // сохранённые сети + дежурная в конце
        size_t candidateIndex_ = 0;         // какой кандидат пробуем сейчас
        uint32_t connectStartMs_ = 0;       // время начала текущей попытки
        Status status_ = Status::Disconnected;
        String currentSsid_;     // имя активной сети
        bool suspended_ = false; // приостановлен (активен AP)

        // Пересобрать список кандидатов.
        void rebuildCandidates();

        // Полный сброс и попытка подключения к конкретной сети.
        void startConnection(const String &ssid, const String &pass);
    };

} // namespace relay::wifi
