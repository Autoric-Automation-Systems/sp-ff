#include <Arduino.h>

#include "spff/Config.h"
#include "spff/app/Device.h"
#include "spff/app/SettingsStore.h"
#include "spff/core/EventQueue.h"
#include "spff/hw/BatteryGauge.h"
#include "spff/hw/FlowSensor.h"
#include "spff/hw/PersistentStorage.h"
#include "spff/core/SignalMonitor.h"
#include "spff/net/ApiClient.h"
#include "spff/net/HttpOtaUpdater.h"
#include "spff/net/SecureHttpTransport.h"
#include "spff/net/SystemClock.h"
#include "spff/net/WifiLink.h"
#include "spff/power/RuntimeController.h"

namespace
{

using namespace spff;

// Composition root: o único lugar que conhece todas as implementações.
SystemClock g_clock("pool.ntp.org", -3 * 3600, 0);
WifiLink g_network(config::kPortalSsid);
SecureHttpTransport g_transport;
ApiClient g_api(g_transport, config::kBaseUrl);
EventQueue g_queue(persistentQueueStorage());
FlowSensor g_flow(config::kFlowPin, config::kPulsesPerLiter);
BatteryGauge g_battery(config::kBatteryPin, config::kBatteryDivider, config::kBatterySamples);
SignalMonitor g_signal(config::kRssiReportDeltaDbm);
SettingsStore g_settings(config::kHeartbeatLimits, "spff");
HttpOtaUpdater g_updater;
RuntimeController g_power;

Device g_device(Device::Dependencies{g_clock, g_network, g_api, g_queue,
                                     g_flow, g_battery, g_signal, g_settings,
                                     g_updater, g_power});

} // namespace

void setup()
{
  g_device.begin();
}

void loop()
{
  g_device.tick();
}
