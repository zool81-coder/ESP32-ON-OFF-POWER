#pragma once

// =============================================================================
//  wifi_storage.hpp — хранение Wi-Fi сетей в независимой памяти (NVS)
// -----------------------------------------------------------------------------
//  Сети сохраняются в энергонезависимой памяти (NVS через Preferences),
//  поэтому переживают перезагрузку и отключение питания.
//
//  Формат хранения: каждая сеть — отдельный ключ "net_<номер>" со значением
//  JSON {"s":"ssid","p":"пароль"}. Отдельные ключи позволяют надёжно хранить
//  до 100 сетей, не упираясь в лимит размера одного значения NVS.
// =============================================================================

#include <Arduino.h> // String
#include <vector>    // std::vector
#include <cstddef>   // size_t

namespace relay::wifi
{

    // Одна сохранённая Wi-Fi сеть.
    struct Network
    {
        String ssid; // имя сети
        String pass; // пароль
    };

    // Хранилище сетей с кэшем в оперативной памяти.
    class Storage
    {
    public:
        // Колбэк, вызываемый ПОСЛЕ успешной записи в NVS.
        // Используется для световой индикации (фиолетовая вспышка).
        using WriteCallback = void (*)();

        // Загружает список сетей из NVS. Вызывать один раз при старте.
        bool begin();

        // Установить колбэк "запись выполнена".
        void setWriteCallback(WriteCallback cb) { onWrite_ = cb; }

        // Текущий список (новые сети — в начале).
        const std::vector<Network> &networks() const { return networks_; }

        // Количество сохранённых сетей.
        size_t count() const { return networks_.size(); }

        // Добавить сеть. Если SSID уже существует — обновляет пароль и
        // перемещает сеть в начало списка. Лимит — kMaxSavedNetworks.
        // Возвращает true при успешной записи в NVS.
        bool add(const String &ssid, const String &pass);

        // Удалить сеть по индексу (0 — самая новая).
        bool remove(size_t index);

    private:
        std::vector<Network> networks_;   // кэш в RAM: [0] = самая новая
        WriteCallback onWrite_ = nullptr; // колбэк записи в NVS

        // Записать весь список в NVS.
        bool persist();
    };

} // namespace relay::wifi
