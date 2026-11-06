#pragma once

// =============================================================================
//  web_server.hpp — HTTP-сервер
// =============================================================================

#include <WebServer.h>
#include <ArduinoJson.h>

namespace relay::power
{
    class Controller;
}

namespace relay::wifi
{
    class Storage;
    class Manager;
}

namespace relay::ap
{
    class Manager;
}

namespace relay::battery
{
    class Monitor;
}

namespace relay::web
{

    class Server
    {
    public:
        Server(power::Controller &power, wifi::Storage &storage,
               wifi::Manager &wifi, ap::Manager &ap, battery::Monitor &battery);

        void begin();
        void handleClient();
        void restart();

    private:
        power::Controller &power_;
        wifi::Storage &storage_;
        wifi::Manager &wifi_;
        ap::Manager &ap_;
        battery::Monitor &battery_;
        WebServer server_;

        void registerRoutes();
        void sendJson(int code, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
        void sendJsonDoc(int code, const JsonDocument &doc);
    };

} // namespace relay::web
