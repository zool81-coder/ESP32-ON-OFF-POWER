#include "watchdog.hpp"

#include <esp_task_wdt.h> // API сторожевого таймера задач
#include <esp_system.h>   // esp_reset_reason()
#include <esp_err.h>      // ESP_OK, ESP_ERR_INVALID_STATE

#include <freertos/FreeRTOS.h> // xTaskGetCurrentTaskHandle
#include <freertos/task.h>     // TaskHandle_t

#include "log.hpp"

namespace relay::watchdog
{

    namespace
    {

        // Преобразование причины сброса в понятный русский текст.
        const char *resetReasonToString(esp_reset_reason_t reason)
        {
            switch (reason)
            {
            case ESP_RST_POWERON:
                return "включение питания";
            case ESP_RST_EXT:
                return "внешний сброс (пин RESET)";
            case ESP_RST_SW:
                return "программный сброс";
            case ESP_RST_PANIC:
                return "паника (исключение/assert)";
            case ESP_RST_INT_WDT:
                return "сторожевой таймер прерываний (IWDT)";
            case ESP_RST_TASK_WDT:
                return "сторожевой таймер задачи (TWDT)";
            case ESP_RST_WDT:
                return "другой сторожевой таймер";
            case ESP_RST_DEEPSLEEP:
                return "выход из глубокого сна";
            case ESP_RST_BROWNOUT:
                return "просадка питания (brownout)";
            case ESP_RST_SDIO:
                return "сброс по SDIO";
            default:
                return "неизвестная причина";
            }
        }

    } // namespace

    // Инициализация сторожевого таймера.
    void begin(uint32_t /*timeoutSeconds*/)
    {
        // ВАЖНО: ядро Arduino core 3.x УЖЕ инициализировало TWDT и подписало loopTask.
        // Повторный esp_task_wdt_init()/reconfigure() ломал подписку и приводил
        // к срабатыванию WDT -> перезагрузке. Поэтому здесь мы НЕ трогаем init/reconfigure.
        //
        // Таймаут задаётся ядром (по умолчанию 5 секунд) — этого с запасом хватает,
        // т.к. цикл loop() быстрый, а во время setup() мы явно "кормим" таймер (см. main.cpp).

        // Пытаемся подписать текущую задачу (loopTask).
        // Если ядро уже подписало её — получаем ESP_ERR_INVALID_STATE, это нормально.
        const esp_err_t addErr = esp_task_wdt_add(xTaskGetCurrentTaskHandle());
        if (addErr == ESP_ERR_INVALID_STATE)
        {
            log::debug("WDT", "loopTask уже подписан ядром Arduino");
        }
        else if (addErr != ESP_OK)
        {
            log::error("WDT", "ошибка подписки задачи (код 0x%x)",
                       static_cast<unsigned>(addErr));
            return;
        }

        log::info("WDT", "сторожевой таймер активен (таймаут задаёт ядро)");
    }

    // Сброс счётчика таймера — вызывается каждый проход loop().
    void feed()
    {
        esp_task_wdt_reset();
    }

    // Логирование причины последней перезагрузки.
    void logResetReason()
    {
        log::info("Загрузка", "причина перезагрузки: %s",
                  resetReasonToString(esp_reset_reason()));
    }

} // namespace relay::watchdog
