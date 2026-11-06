#include "wifi_storage.hpp"

#include <Preferences.h> // NVS-хранилище
#include <ArduinoJson.h> // JSON
#include <cstdio>        // snprintf

#include "config.hpp"
#include "log.hpp"

namespace relay::wifi
{

    // Загрузка сетей из NVS в оперативную память.
    bool Storage::begin()
    {
        networks_.clear();

        Preferences prefs;
        if (!prefs.begin(config::kStorageNamespace, true))
        { // режим "только чтение"
            log::warn("Хранилище", "не удалось открыть NVS (чтение)");
            return false;
        }

        const uint32_t count = prefs.getUInt("count", 0);
        const uint32_t limit = count > config::kMaxSavedNetworks
                                   ? config::kMaxSavedNetworks
                                   : count;

        char key[16];
        for (uint32_t i = 0; i < limit; ++i)
        {
            snprintf(key, sizeof(key), "net_%u", static_cast<unsigned>(i));
            const String json = prefs.getString(key, "");

            // Ключа нет — дальше пустых быть не должно, выходим.
            if (json.length() == 0)
            {
                break;
            }

            JsonDocument doc;
            const DeserializationError err = deserializeJson(doc, json);
            if (err)
            {
                log::warn("Хранилище", "ошибка разбора ключа %s", key);
                continue;
            }

            Network n;
            n.ssid = doc["s"] | "";
            n.pass = doc["p"] | "";
            if (n.ssid.length() > 0)
            {
                networks_.push_back(n);
            }
        }

        prefs.end();
        log::info("Хранилище", "загружено сетей: %u",
                  static_cast<unsigned>(networks_.size()));
        return true;
    }

    // Добавление сети (с дедупликацией и лимитом).
    bool Storage::add(const String &ssid, const String &pass)
    {
        if (ssid.length() == 0)
        {
            return false;
        }

        // SSID уже есть? Обновляем пароль и переносим в начало (приоритет).
        for (size_t i = 0; i < networks_.size(); ++i)
        {
            if (networks_[i].ssid == ssid)
            {
                networks_[i].pass = pass;
                const Network n = networks_[i];
                networks_.erase(networks_.begin() + i);
                networks_.insert(networks_.begin(), n);
                return persist();
            }
        }

        // Новая сеть. Если лимит исчерпан — удаляем самую старую (в конце).
        if (networks_.size() >= config::kMaxSavedNetworks)
        {
            networks_.pop_back();
        }

        networks_.insert(networks_.begin(), Network{ssid, pass});
        return persist();
    }

    // Удаление сети по индексу.
    bool Storage::remove(size_t index)
    {
        if (index >= networks_.size())
        {
            return false;
        }
        networks_.erase(networks_.begin() + index);
        return persist();
    }

    // Запись всего списка в NVS (перезаписываем целиком для простоты и надёжности).
    bool Storage::persist()
    {
        Preferences prefs;
        if (!prefs.begin(config::kStorageNamespace, false))
        { // режим "чтение+запись"
            log::error("Хранилище", "не удалось открыть NVS (запись)");
            return false;
        }

        // Очищаем namespace от старых ключей и пишем всё заново.
        prefs.clear();

        prefs.putUInt("count", static_cast<uint32_t>(networks_.size()));

        char key[16];
        for (size_t i = 0; i < networks_.size(); ++i)
        {
            snprintf(key, sizeof(key), "net_%u", static_cast<unsigned>(i));

            JsonDocument doc;
            doc["s"] = networks_[i].ssid;
            doc["p"] = networks_[i].pass;

            String json;
            serializeJson(doc, json);
            prefs.putString(key, json);
        }

        const bool ok = prefs.getUInt("count", 0) == networks_.size();
        prefs.end();

        if (ok)
        {
            log::info("Хранилище", "сохранено сетей: %u",
                      static_cast<unsigned>(networks_.size()));

            // Сообщаем об успешной записи (для световой индикации).
            if (onWrite_)
            {
                onWrite_();
            }
        }
        else
        {
            log::error("Хранилище", "ошибка записи в NVS");
        }
        return ok;
    }

} // namespace relay::wifi
