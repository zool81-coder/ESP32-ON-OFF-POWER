#pragma once

// =============================================================================
//  ap_manager.hpp — режим точки доступа (AP) + captive portal
// -----------------------------------------------------------------------------
//  Включается по запросу (из веб-интерфейса или длинным нажатием кнопки BOOT).
//
//  При активации:
//    - Wi-Fi станция приостанавливается (wifi::Manager::suspend);
//    - поднимается SoftAP с именем из конфига (kApSsid);
//    - запускается DNS-перехват (captive portal): любой домен -> IP точки.
//
//  Запуск/остановка ОТЛОЖЕННЫЕ: requestStart()/requestStop() лишь ставят флаг,
//  а само переключение выполняется в update() (в loop()). Это позволяет
//  веб-ответу уйти по старой сети до её отключения.
// =============================================================================

#include <DNSServer.h> // DNSServer
#include <cstdint>

namespace relay::wifi
{
    class Manager;
}

namespace relay::ap
{

    class Manager
    {
    public:
        // Принимает ссылку на Wi-Fi менеджер (для suspend/resume).
        explicit Manager(wifi::Manager &wifi);

        // Инициализация (AP включается только по запросу, здесь ничего не делаем).
        void begin();

        // Попросить включить режим AP (отложено).
        void requestStart();

        // Попросить выключить режим AP (отложено).
        void requestStop();

        // Вызывать в loop(): обрабатывает отложенные команды + DNS-запросы.
        void update();

        // Активен ли режим AP.
        [[nodiscard]] bool isActive() const noexcept { return active_; }

    private:
        wifi::Manager &wifi_; // для suspend/resume режима станции
        DNSServer dnsServer_; // DNS-перехват captive portal
        bool active_ = false;
        bool pendingStart_ = false; // отложенная команда "включить"
        bool pendingStop_ = false;  // отложенная команда "выключить"

        void doStart(); // реальное включение AP
        void doStop();  // реальное выключение AP
    };

} // namespace relay::ap
