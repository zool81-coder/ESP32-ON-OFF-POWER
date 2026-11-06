#pragma once

// =============================================================================
//  battery.hpp — монитор заряда аккумулятора (измерение через ADC)
// -----------------------------------------------------------------------------
//  Измеряет напряжение на делителе, классифицирует в 4 уровня заряда
//  и считает процент. Калибровочный множитель хранится в NVS и может
//  меняться из веб-интерфейса.
// =============================================================================

#include <cstdint>

namespace relay::battery
{

    enum class Level : uint8_t
    {
        Full = 0,     // полностью заряжен
        Half = 1,     // половина заряда
        Low = 2,      // низкий заряд
        Critical = 3, // очень мало заряда
    };

    class Monitor
    {
    public:
        // Колбэк, вызываемый после успешного сохранения калибровки в NVS
        // (используется для фиолетовой вспышки RGB).
        using SaveCallback = void (*)();

        void begin();
        Level measure();

        [[nodiscard]] uint32_t voltageMv() const { return voltageMv_; }
        [[nodiscard]] Level level() const { return level_; }
        [[nodiscard]] uint8_t percent() const { return percent_; }

        // Текущий калибровочный множитель.
        [[nodiscard]] float calibration() const { return calibration_; }

        // Установить и СОХРАНИТЬ в NVS новый калибровочный множитель.
        void setCalibration(float value);

        // Установить колбэк "калибровка сохранена".
        void setSaveCallback(SaveCallback cb) { onSave_ = cb; }

        static Level voltageToLevel(uint32_t voltageMv);

    private:
        uint32_t voltageMv_ = 0;
        Level level_ = Level::Full;
        uint8_t percent_ = 100;
        float calibration_ = 1.0f; // загружается из NVS в begin()
        SaveCallback onSave_ = nullptr;

        uint32_t readVoltageMv();
        void loadCalibration();
    };

} // namespace relay::battery
