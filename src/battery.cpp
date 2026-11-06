#include "battery.hpp"

#include <Arduino.h>             // delay
#include <Preferences.h>         // NVS для калибровки
#include <esp_adc/adc_oneshot.h> // новый ADC-драйвер

#include "config.hpp"
#include "log.hpp"

namespace relay::battery
{

    namespace
    {
        adc_oneshot_unit_handle_t gAdcUnit = nullptr;

        // GPIO -> канал ADC1 для ESP32-C3 (GPIO0..4 -> ADC_CHANNEL_0..4).
        adc_channel_t gpioToAdc1Channel(int gpio)
        {
            switch (gpio)
            {
            case 0:
                return ADC_CHANNEL_0;
            case 1:
                return ADC_CHANNEL_1;
            case 2:
                return ADC_CHANNEL_2;
            case 3:
                return ADC_CHANNEL_3;
            case 4:
                return ADC_CHANNEL_4;
            default:
                return ADC_CHANNEL_0;
            }
        }
    } // namespace

    void Monitor::begin()
    {
        // 1. ADC-юнит.
        adc_oneshot_unit_init_cfg_t unitCfg = {};
        unitCfg.unit_id = ADC_UNIT_1;
        adc_oneshot_new_unit(&unitCfg, &gAdcUnit);

        // 2. Канал с аттенюацией ~1.25 В (DB_2_5).
        //    ВАЖНО: DB_0 даёт диапазон всего ~0.75 В — при делителе 4.0 и батарее
        //    выше 3.0 В на пине будет >0.75 В, ADC уходит в насыщение и показания
        //    "залипают" на максимуме. DB_2_5 (до ~1.25 В) решает эту проблему.
        const adc_channel_t channel = gpioToAdc1Channel(config::kBatteryAdcPin);
        adc_oneshot_chan_cfg_t chanCfg = {};
        chanCfg.atten = ADC_ATTEN_DB_2_5;
        chanCfg.bitwidth = ADC_BITWIDTH_12;
        adc_oneshot_config_channel(gAdcUnit, channel, &chanCfg);

        // 3. "Прогрев" ADC.
        int dummy = 0;
        adc_oneshot_read(gAdcUnit, channel, &dummy);
        delay(10);

        // 4. Калибровка из NVS.
        loadCalibration();

        log::info("Батарея", "монитор инициализирован (пин %d, делитель %.1f, калибровка %.4f)",
                  config::kBatteryAdcPin, static_cast<double>(config::kBatteryDivider),
                  static_cast<double>(calibration_));
    }

    Level Monitor::measure()
    {
        voltageMv_ = readVoltageMv();
        level_ = voltageToLevel(voltageMv_);

        const int32_t full = static_cast<int32_t>(config::kBatteryFullMv);
        const int32_t empty = static_cast<int32_t>(config::kBatteryEmptyMv);
        const int32_t v = static_cast<int32_t>(voltageMv_);

        int32_t pct = 0;
        if (full > empty)
        {
            pct = (v - empty) * 100 / (full - empty);
        }
        if (pct > 100)
            pct = 100;
        if (pct < 0)
            pct = 0;
        percent_ = static_cast<uint8_t>(pct);

        log::info("Батарея", "напряжение %u мВ, уровень %u, заряд %u%%",
                  static_cast<unsigned>(voltageMv_),
                  static_cast<unsigned>(level_),
                  static_cast<unsigned>(percent_));
        return level_;
    }

    Level Monitor::voltageToLevel(uint32_t voltageMv)
    {
        if (voltageMv >= config::kBatteryFullMv)
            return Level::Full;
        if (voltageMv >= config::kBatteryHalfMv)
            return Level::Half;
        if (voltageMv >= config::kBatteryLowMv)
            return Level::Low;
        return Level::Critical;
    }

    void Monitor::setCalibration(float value)
    {
        if (value <= 0.0f)
        {
            return;
        }
        calibration_ = value;

        Preferences prefs;
        if (prefs.begin(config::kSettingsNamespace, false))
        {
            prefs.putFloat("cal", value);
            prefs.end();
            log::info("Батарея", "калибровка сохранена: %.4f",
                      static_cast<double>(value));
            if (onSave_)
            {
                onSave_();
            }
        }
        else
        {
            log::error("Батарея", "не удалось сохранить калибровку в NVS");
        }
    }

    void Monitor::loadCalibration()
    {
        calibration_ = config::kBatteryCalibrationDefault;

        Preferences prefs;
        if (prefs.begin(config::kSettingsNamespace, true))
        {
            calibration_ = prefs.getFloat("cal", calibration_);
            prefs.end();
        }
    }

    uint32_t Monitor::readVoltageMv()
    {
        constexpr int kSamples = 32;
        const adc_channel_t channel = gpioToAdc1Channel(config::kBatteryAdcPin);

        uint32_t sum = 0;
        for (int i = 0; i < kSamples; ++i)
        {
            int raw = 0;
            adc_oneshot_read(gAdcUnit, channel, &raw);
            sum += static_cast<uint32_t>(raw);
            delay(2);
        }
        const float avgRaw = static_cast<float>(sum) / static_cast<float>(kSamples);

        // Предупреждение о насыщении ADC (полезно для диагностики).
        if (avgRaw >= 4094.0f)
        {
            log::warn("Батарея", "ADC в насыщении (raw=%.0f) — проверьте аттенюацию/делитель",
                      avgRaw);
        }

        // 12 бит (0–4095) -> ~0–1.250 В при аттенюации ADC_ATTEN_DB_2_5.
        const float adcVoltage = (avgRaw / 4095.0f) * 1.250f;

        // Напряжение батареи = напряжение на пине * делитель * калибровка.
        const float batteryVoltage =
            adcVoltage * config::kBatteryDivider * calibration_;

        return static_cast<uint32_t>(batteryVoltage * 1000.0f);
    }

} // namespace relay::battery
