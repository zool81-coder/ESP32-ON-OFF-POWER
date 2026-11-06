#include "log.hpp"

#include <Arduino.h> // Serial
#include <cstdarg>   // va_list
#include <cstdio>    // vsnprintf

namespace relay::log
{

    namespace
    { // анонимный namespace — внутренние детали не видны снаружи

        // Короткие текстовые метки уровней (индекс совпадает со значением enum Level).
        constexpr const char *kLevelNames[] = {"DEBUG", "INFO ", "WARN ", "ERROR"};

        // Общая функция вывода: фильтрует по уровню и печатает строку в Serial.
        void write(Level level, const char *tag, const char *fmt, va_list args)
        {
            // Пропускаем сообщения ниже минимального уровня.
            if (static_cast<uint8_t>(level) < static_cast<uint8_t>(kMinLevel))
            {
                return;
            }

            // Формируем сообщение в буфере (vsnprintf не выйдет за границы).
            char buffer[192];
            vsnprintf(buffer, sizeof(buffer), fmt, args);

            // Формат вывода: [УРОВЕНЬ][ТЕГ] сообщение
            Serial.print('[');
            Serial.print(kLevelNames[static_cast<uint8_t>(level)]);
            Serial.print("][");
            Serial.print(tag);
            Serial.print("] ");
            Serial.println(buffer);
        }

    } // namespace

    void init(uint32_t baud)
    {
        Serial.begin(baud);
        delay(100); // небольшая пауза, чтобы Serial успел "подняться"
    }

    void debug(const char *tag, const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        write(Level::Debug, tag, fmt, args);
        va_end(args);
    }

    void info(const char *tag, const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        write(Level::Info, tag, fmt, args);
        va_end(args);
    }

    void warn(const char *tag, const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        write(Level::Warn, tag, fmt, args);
        va_end(args);
    }

    void error(const char *tag, const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        write(Level::Error, tag, fmt, args);
        va_end(args);
    }

} // namespace relay::log
