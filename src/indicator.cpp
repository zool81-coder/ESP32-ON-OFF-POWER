#include "indicator.hpp"

#include <Arduino.h> // pinMode, digitalWrite, millis

#include "config.hpp"

namespace relay::indicator
{

    // Инициализация пина светодиода.
    void Led::begin()
    {
        pinMode(config::kLedPin, OUTPUT);
        setPattern(Pattern::On); // при старте питание включено -> LED горит
    }

    // Установка паттерна. Функция идемпотентна: если паттерн не изменился,
    // состояние мигания НЕ сбрасывается (иначе мигание никогда не сработает,
    // т.к. setPattern вызывается каждый проход loop()).
    void Led::setPattern(Pattern p)
    {
        // Паттерн не изменился — ничего не делаем.
        if (p == pattern_)
        {
            return;
        }

        pattern_ = p;

        if (p == Pattern::On)
        {
            applyLevel(true);
        }
        else if (p == Pattern::Off)
        {
            applyLevel(false);
        }
        else
        {
            // Мигание: начинаем со "включено" и запускаем отсчёт.
            levelOn_ = true;
            applyLevel(true);
            lastToggleMs_ = millis();
        }
    }

    // Реализация мигания (вызывается каждый проход loop()).
    void Led::update()
    {
        // Статичные состояния не требуют таймера.
        if (pattern_ != Pattern::BlinkFast && pattern_ != Pattern::BlinkSlow)
        {
            return;
        }

        const uint32_t interval = intervalFor(pattern_);
        if (millis() - lastToggleMs_ >= interval)
        {
            lastToggleMs_ = millis();
            levelOn_ = !levelOn_; // переключаем уровень
            applyLevel(levelOn_);
        }
    }

    // Применение физического уровня с учётом инверсии светодиода.
    void Led::applyLevel(bool on)
    {
        digitalWrite(config::kLedPin,
                     on ? config::kLedActiveLevel : !config::kLedActiveLevel);
    }

    // Период мигания для паттерна (мс).
    uint32_t Led::intervalFor(Pattern p) const
    {
        switch (p)
        {
        case Pattern::BlinkFast:
            return 100; // 100 мс -> ~5 Гц (быстро)
        case Pattern::BlinkSlow:
            return 500; // 500 мс -> 1 Гц
        default:
            return 100;
        }
    }

} // namespace relay::indicator
