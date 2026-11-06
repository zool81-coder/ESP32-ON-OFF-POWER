#include <Arduino.h>

// =============================================================================
//  main.cpp — точка входа программы (Composition Root)
// =============================================================================

#include "config.hpp"
#include "log.hpp"
#include "power_controller.hpp"
#include "watchdog.hpp"
#include "web_server.hpp"
#include "wifi_manager.hpp"
#include "wifi_storage.hpp"
#include "ap_manager.hpp"
#include "indicator.hpp"
#include "rgb_indicator.hpp"
#include "battery.hpp"
#include "ota_manager.hpp"

using namespace relay;

namespace
{
  power::Controller gPower;
  wifi::Storage gWifiStorage;
  wifi::Manager gWifi;
  ap::Manager gAp{gWifi};
  battery::Monitor gBattery;
  ota::Manager gOta;
  web::Server gWeb{gPower, gWifiStorage, gWifi, gAp, gBattery};
  indicator::Led gLed;
  rgb::Led gRgb;

  uint32_t gBootMs = 0;
} // namespace

// -----------------------------------------------------------------------------
// Индикация уровня заряда на RGB-светодиоде (однократно при старте).
// -----------------------------------------------------------------------------
void showBatteryLevel(battery::Level level)
{
  switch (level)
  {
  case battery::Level::Full:
    gRgb.playBlinkSequence(0, 255, 0, 5, 300); // зелёный, 5 раз
    break;
  case battery::Level::Half:
    gRgb.playBlinkSequence(255, 165, 0, 5, 300); // оранжевый, 5 раз
    break;
  case battery::Level::Low:
    gRgb.playBlinkSequence(255, 0, 0, 5, 300); // красный, 5 раз
    break;
  case battery::Level::Critical:
    gRgb.playBlinkSequence(255, 0, 0, 15, 100); // красный, очень быстро
    break;
  }
}

// -----------------------------------------------------------------------------
// Обработка кнопки BOOT: длинное нажатие переключает точку доступа.
// -----------------------------------------------------------------------------
void handleBootButton()
{
  static bool wasPressed = false;
  static uint32_t pressStartMs = 0;
  static bool triggered = false;

  const bool pressed = (digitalRead(config::kBootButtonPin) == LOW);

  if (pressed && !wasPressed)
  {
    pressStartMs = millis();
    triggered = false;
  }

  if (pressed && !triggered &&
      (millis() - pressStartMs >= config::kBootLongPressMs))
  {
    triggered = true;
    if (gAp.isActive())
      gAp.requestStop();
    else
      gAp.requestStart();
  }

  wasPressed = pressed;
}

void setup()
{
  gBootMs = millis();

  log::init(config::kSerialBaud);
  log::info("Загрузка", "%s", "===== ВГШ: запуск =====");
  log::info("Загрузка", "версия прошивки: V%s", config::kFirmwareVersion);

  watchdog::logResetReason();
  watchdog::begin(config::kWatchdogTimeoutSec);

  gLed.begin();
  gRgb.begin();
  pinMode(config::kBootButtonPin, INPUT_PULLUP);
  watchdog::feed();

  gPower.begin();
  gBattery.begin();
  gWifiStorage.begin();
  gWifiStorage.setWriteCallback([]()
                                { gRgb.flashWrite(); });
  gBattery.setSaveCallback([]()
                           { gRgb.flashWrite(); });

  watchdog::feed();

  gWifi.begin(gWifiStorage);
  watchdog::feed();

  gAp.begin();
  gWeb.begin();
  watchdog::feed();

  // OTA: колбэки индикации.
  gOta.setOnStart([]()
                  { gRgb.playBlinkSequence(0, 0, 255, 5, 200); }); // синий
  gOta.setOnEnd([]()
                { gRgb.playBlinkSequence(0, 255, 0, 3, 200); }); // зелёный
  gOta.setOnError([]()
                  { gRgb.playBlinkSequence(255, 0, 0, 5, 200); }); // красный
  gOta.begin();
  watchdog::feed();

  log::info("Загрузка", "%s", "инициализация завершена");
}

void loop()
{
  handleBootButton();

  // Через 3 секунды после старта — однократно показать уровень заряда.
  static bool batteryStartupShown = false;
  if (!batteryStartupShown &&
      millis() - gBootMs >= config::kBatteryStartupDelayMs)
  {
    batteryStartupShown = true;
    showBatteryLevel(gBattery.measure());
  }

  gAp.update();
  gWifi.update();

  static bool apWasActive = false;
  if (gAp.isActive() != apWasActive)
  {
    apWasActive = gAp.isActive();
    gWeb.restart();
  }

  gWeb.handleClient();
  gOta.handle(); // <-- обработка OTA (прошивка по воздуху)

  // Индикация состояния.
  if (gAp.isActive())
  {
    gLed.setPattern(indicator::Led::Pattern::BlinkFast);
  }
  else
  {
    gLed.setPattern(gPower.isOn() ? indicator::Led::Pattern::On
                                  : indicator::Led::Pattern::Off);
  }
  gLed.update();

  if (gAp.isActive())
  {
    gRgb.setMode(rgb::Led::Mode::Ap);
  }
  else
  {
    gRgb.setMode(gPower.isOn() ? rgb::Led::Mode::PowerOn
                               : rgb::Led::Mode::PowerOff);
  }
  gRgb.update();

  watchdog::feed();
}
