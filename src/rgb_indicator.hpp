#pragma once

// =============================================================================
//  rgb_indicator.hpp — АДРЕСНЫЙ светодиод (WS2812/NeoPixel) на Adafruit NeoPixel
// -----------------------------------------------------------------------------
//  Базовые режимы:
//    - PowerOn  (зелёный)         — питание включено;
//    - PowerOff (красный)         — питание выключено;
//    - Ap       (голубое дыхание) — режим точки доступа.
//
//  Разовые последовательности (перекрывают базовый режим, затем возвращаются):
//    - flashWrite()          — фиолетовое мигание (запись в NVS);
//    - playBlinkSequence()   — произвольное цветное мигание (индикация батареи).
// =============================================================================

#include <Adafruit_NeoPixel.h> // адресный светодиод
#include <cstdint>

namespace relay::rgb
{

    class Led
    {
    public:
        enum class Mode : uint8_t
        {
            PowerOn = 0,  // зелёный
            PowerOff = 1, // красный
            Ap = 2,       // голубое "дыхание"
        };

        Led();
        ~Led();

        void begin();
        void setMode(Mode mode);
        void update();

        // Разовая фиолетовая вспышка (сигнал записи в NVS).
        void flashWrite();

        // Разовая последовательность миганий заданным цветом.
        //   r,g,b        — цвет (0..255);
        //   blinkCount   — сколько раз мигнуть;
        //   intervalMs   — длительность одной половины мигания (вкл/выкл), мс.
        void playBlinkSequence(uint8_t r, uint8_t g, uint8_t b,
                               uint8_t blinkCount, uint32_t intervalMs);

    private:
        Adafruit_NeoPixel strip_; // драйвер ленты
        Mode mode_ = Mode::PowerOn;

        // Состояние разовой последовательности миганий.
        bool seqActive_ = false;
        uint32_t seqLastToggleMs_ = 0;
        bool seqOn_ = false;
        uint8_t seqR_ = 0, seqG_ = 0, seqB_ = 0;
        uint8_t seqBlinkCount_ = 0;
        uint8_t seqShown_ = 0;
        uint32_t seqIntervalMs_ = 0;

        void setPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
        void refresh();
        void render();
    };

} // namespace relay::rgb
