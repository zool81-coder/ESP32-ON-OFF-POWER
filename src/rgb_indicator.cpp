#include "rgb_indicator.hpp"

#include <Arduino.h> // millis
#include <cmath>     // sinf

#include "config.hpp"
#include "log.hpp"

namespace relay::rgb
{

    namespace
    {

        // Базовая яркость обычных состояний (0..255).
        constexpr uint8_t kBaseBrightness = 64;

        // Период полного цикла "дыхания" (мс).
        constexpr uint32_t kBreathPeriodMs = 1200;

        // Цвета (RGB).
        constexpr uint8_t kGreenR = 0, kGreenG = 255, kGreenB = 0;
        constexpr uint8_t kRedR = 255, kRedG = 0, kRedB = 0;
        constexpr uint8_t kBlueR = 0, kBlueG = 0, kBlueB = 255;
        constexpr uint8_t kPurpleR = 160, kPurpleG = 0, kPurpleB = 255;

    } // namespace

    // Конструктор: пин/количество/тип передаём сразу — это надёжный способ
    // инициализации (не setPin+updateLength+updateType).
    Led::Led()
        : strip_(config::kNeoPixelCount, config::kNeoPixelPin, NEO_GRB + NEO_KHZ800) {}

    Led::~Led()
    {
        // Adafruit_NeoPixel не требует ручного освобождения — деструктор
        // библиотеки сам завершит работу с RMT-каналом.
    }

    void Led::begin()
    {
        strip_.begin();
        strip_.setBrightness(kBaseBrightness);
        strip_.clear();
        strip_.show();

        log::info("RGB", "адресный светодиод инициализирован (Adafruit NeoPixel), пин %d",
                  config::kNeoPixelPin);

        render(); // сразу показываем зелёный (питание включено по умолчанию)
    }

    void Led::setMode(Mode mode)
    {
        if (mode == mode_)
        {
            return;
        }
        mode_ = mode;
        render();
    }

    void Led::update()
    {
        // --- Идёт разовая последовательность миганий ---
        if (seqActive_)
        {
            const uint32_t now = millis();
            if (now - seqLastToggleMs_ >= seqIntervalMs_)
            {
                seqLastToggleMs_ = now;
                seqOn_ = !seqOn_;

                if (seqOn_)
                {
                    seqShown_++;
                    setPixel(seqR_, seqG_, seqB_, kBaseBrightness);
                }
                else
                {
                    setPixel(0, 0, 0, 255);
                }
                refresh();

                // Последовательность завершена: показали нужное число вспышек
                // и последний раз выключили светодиод.
                if (!seqOn_ && seqShown_ >= seqBlinkCount_)
                {
                    seqActive_ = false;
                    render(); // возврат к базовому режиму
                }
            }
            return;
        }

        // "Дыхание" требует постоянного пересчёта яркости.
        if (mode_ == Mode::Ap)
        {
            render();
        }
    }

    void Led::flashWrite()
    {
        // Фиолетовое мигание ~2 сек: 10 вспышек по 100 мс.
        playBlinkSequence(kPurpleR, kPurpleG, kPurpleB, 10, 100);
    }

    void Led::playBlinkSequence(uint8_t r, uint8_t g, uint8_t b,
                                uint8_t blinkCount, uint32_t intervalMs)
    {
        seqActive_ = true;
        seqR_ = r;
        seqG_ = g;
        seqB_ = b;
        seqBlinkCount_ = blinkCount;
        seqIntervalMs_ = intervalMs;
        seqShown_ = 0;
        seqOn_ = true; // первое состояние — сразу "включено"

        seqShown_ = 1;
        setPixel(seqR_, seqG_, seqB_, kBaseBrightness);
        refresh();
        seqLastToggleMs_ = millis();
    }

    void Led::render()
    {
        switch (mode_)
        {
        case Mode::PowerOn:
            setPixel(kGreenR, kGreenG, kGreenB, kBaseBrightness);
            break;

        case Mode::PowerOff:
            setPixel(kRedR, kRedG, kRedB, kBaseBrightness);
            break;

        case Mode::Ap:
        {
            // Плавное "дыхание" голубым: яркость по синусоиде (0..255).
            const uint32_t t = millis() % kBreathPeriodMs;
            const float phase =
                static_cast<float>(t) / static_cast<float>(kBreathPeriodMs);
            const float wave = (sinf(phase * 2.0f * 3.14159265f) + 1.0f) / 2.0f;
            const uint8_t brightness = static_cast<uint8_t>(wave * 255.0f);

            setPixel(kBlueR, kBlueG, kBlueB, brightness);
            break;
        }
        }
        refresh();
    }

    void Led::setPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
    {
        // У библиотеки есть глобальная яркость setBrightness(), но она уже
        // применяется в show(). Чтобы менять яркость для "дыхания" на лету,
        // масштабируем компоненты вручную и сбрасываем глобальную яркость в максимум.
        strip_.setBrightness(255);
        const uint8_t rr = static_cast<uint8_t>((static_cast<uint16_t>(r) * brightness) / 255);
        const uint8_t gg = static_cast<uint8_t>((static_cast<uint16_t>(g) * brightness) / 255);
        const uint8_t bb = static_cast<uint8_t>((static_cast<uint16_t>(b) * brightness) / 255);
        strip_.setPixelColor(0, rr, gg, bb);
    }

    void Led::refresh()
    {
        strip_.show();
    }

} // namespace relay::rgb
