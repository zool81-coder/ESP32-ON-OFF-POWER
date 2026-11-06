#pragma once

// =============================================================================
//  config.hpp — централизованные настройки устройства
// =============================================================================

#include <cstdint> // uint32_t, uint16_t, uint8_t

namespace relay::config
{

    // =========================== ВЕРСИЯ ПРОШИВКИ ================================
    inline constexpr const char *kFirmwareVersion = "1.0.4";

    // =========================== OTA (обновление по воздуху) ====================
    // Имя хоста для ArduinoOTA (прошивка из PlatformIO/Arduino IDE).
    inline constexpr const char *kOtaHostname = "relay";
    // Пароль для OTA и веб-загрузчика. ⚠️ ОБЯЗАТЕЛЬНО СМЕНИТЕ на свой!
    inline constexpr const char *kOtaPassword = "relay12345";

    // =========================== ДЕЖУРНАЯ Wi-Fi СЕТЬ ============================
    inline constexpr const char *kDefaultWifiSsid = "WIFI-RELAY";
    inline constexpr const char *kDefaultWifiPassword = "12345678";

    // =========================== ХРАНИЛИЩЕ СЕТЕЙ ================================
    inline constexpr uint16_t kMaxSavedNetworks = 100;
    inline constexpr const char *kStorageNamespace = "relay";
    inline constexpr uint32_t kWifiConnectTimeoutMs = 10'000;

    // =========================== РЕЖИМ ТОЧКИ ДОСТУПА (AP) =======================
    inline constexpr const char *kApSsid = "Relay-Setup";
    inline constexpr const char *kApPassword = "";
    inline constexpr uint8_t kApChannel = 1;

    // =========================== КНОПКА (BOOT) ==================================
    inline constexpr int kBootButtonPin = 9;
    inline constexpr uint32_t kBootLongPressMs = 3000;

    // =========================== ВСТРОЕННЫЙ СВЕТОДИОД ===========================
    inline constexpr int kLedPin = 8;
    inline constexpr bool kLedActiveLevel = false;

    // =========================== АДРЕСНЫЙ СВЕТОДИОД (NeoPixel) ==================
    inline constexpr int kNeoPixelPin = 3; // пин данных (Din)
    inline constexpr int kNeoPixelCount = 1;

    // =========================== БАТАРЕЯ (ADC) ==================================
    // Измерение напряжения аккумулятора через делитель.
    //   Схема: Аккумулятор(+) --[R1=300k]--+--[R2=100k]-- GND
    //                                      |
    //                                     ADC пин (GPIO4)
    //   Коэффициент делителя = (R1 + R2) / R2 = (300k + 100k) / 100k = 4.0
    inline constexpr int kBatteryAdcPin = 0;       // ADC пин (GPIO0)
    inline constexpr float kBatteryDivider = 4.0f; // R1=300k, R2=100k
    // Пороги заряда (мВ) — скорректированы под ESP32-C3:
    inline constexpr uint32_t kBatteryFullMv = 4000;         // >= 4.0 В = 100%
    inline constexpr uint32_t kBatteryHalfMv = 3800;         // 3.8-4.0 В = половина
    inline constexpr uint32_t kBatteryLowMv = 3600;          // 3.6-3.8 В = низкий
    inline constexpr uint32_t kBatteryEmptyMv = 3400;        // 0% (ниже устройство отключится)
    inline constexpr uint32_t kBatteryStartupDelayMs = 3000; // индикация через 3 сек
    // Калибровочный множитель. Подбирается по мультиметру:
    //   calibration = реальное_напряжение / показанное_платой
    // Для вашего замера: 4.050 / 4.400 = 0.9205
    inline constexpr float kBatteryCalibrationDefault = 1.0f;
    // Калибровка ПО УМОЛЧАНИЮ. Фактическое значение хранится в NVS и
    // меняется через веб-интерфейс (меню "⋮" рядом с батареей).

    // =========================== НАСТРОЙКИ (NVS) ================================
    // Отдельный namespace для настроек (чтобы не конфликтовать с хранилищем сетей,
    // которое очищается при записи).
    inline constexpr const char *kSettingsNamespace = "relay_cfg";

    // =========================== Пины ===========================================
    inline constexpr int kPowerPin = 1; // пин управления нагрузкой (HIGH = вкл)

    // =========================== Тайминги =======================================
    inline constexpr uint32_t kSerialBaud = 115'200;
    inline constexpr uint32_t kReconnectDelayMs = 5'000;

    // =========================== Сторожевой таймер (WDT) ========================
    inline constexpr uint32_t kWatchdogTimeoutSec = 10;

    // =========================== HTTP / mDNS ====================================
    inline constexpr uint16_t kHttpPort = 80;
    inline constexpr const char *kMdnsHostname = "relay";

} // namespace relay::config
