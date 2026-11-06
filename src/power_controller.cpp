#include "power_controller.hpp"

#include <Arduino.h>
#include "config.hpp"
#include "log.hpp"

namespace relay::power
{

    void Controller::begin()
    {
        pinMode(config::kPowerPin, OUTPUT);
        apply(State::On);
    }

    void Controller::setOn(bool on)
    {
        apply(on ? State::On : State::Off);
    }

    void Controller::toggle()
    {
        apply(isOn() ? State::Off : State::On);
    }

    void Controller::apply(State state)
    {
        state_ = state;
        const bool on = (state == State::On);

        // Прямая логика: ВКЛ = HIGH (3,3 В), ВЫКЛ = LOW (0 В)
        const int level = on ? HIGH : LOW;
        digitalWrite(config::kPowerPin, level);

        log::info("Питание", "%s (на пине: %s)",
                  on ? "ВКЛ" : "ВЫКЛ", level == HIGH ? "HIGH" : "LOW");
    }

} // namespace relay::power
