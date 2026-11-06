#pragma once

// =============================================================================
//  log.hpp — модуль логирования в Serial
// -----------------------------------------------------------------------------
//  Обёртка над Serial.print с уровнями важности и проверкой формата.
//  Уровни: Debug < Info < Warn < Error.
//  В kMinLevel задаётся минимальный выводимый уровень.
// =============================================================================

#include <cstdint>

namespace relay::log
{

    // --- Уровни важности сообщения ---
    enum class Level : uint8_t
    {
        Debug = 0, // отладочная информация
        Info = 1,  // штатные события
        Warn = 2,  // предупреждения
        Error = 3, // ошибки
    };

    // Минимальный уровень, который выводится в Serial.
    // Поставьте Level::Warn, чтобы скрыть Debug/Info.
    inline constexpr Level kMinLevel = Level::Debug;

    // Инициализация Serial (вызывается первой в setup()).
    void init(uint32_t baud);

    // Функции логирования.
    // Атрибут __attribute__((format(printf, 2, 3))) заставляет компилятор
    // проверять соответствие аргументов строке формата (как в printf).
    void debug(const char *tag, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    void info(const char *tag, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    void warn(const char *tag, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    void error(const char *tag, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

} // namespace relay::log
