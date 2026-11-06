#pragma once

// =============================================================================
//  ota_manager.hpp — обновление прошивки по воздуху (ArduinoOTA)
// -----------------------------------------------------------------------------
//  Реализует OTA-протокол Arduino (используется из PlatformIO/Arduino IDE).
//  Прогресс и события передаются через колбэки (для индикации и логов).
//  Пароль защищает от несанкционированной перепрошивки.
// =============================================================================

#include <cstdint>

namespace relay::ota
{

    class Manager
    {
    public:
        // Колбэки событий (устанавливаются из main.cpp).
        using VoidCallback = void (*)();

        void begin();  // настройка ArduinoOTA
        void handle(); // вызывать в loop()

        void setOnStart(VoidCallback cb) { onStart_ = cb; }
        void setOnEnd(VoidCallback cb) { onEnd_ = cb; }
        void setOnError(VoidCallback cb) { onError_ = cb; }

    private:
        VoidCallback onStart_ = nullptr;
        VoidCallback onEnd_ = nullptr;
        VoidCallback onError_ = nullptr;
    };

} // namespace relay::ota
