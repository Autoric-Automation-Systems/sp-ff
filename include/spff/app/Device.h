#pragma once

#include <cstdint>

#include "spff/app/SettingsStore.h"
#include "spff/core/EventQueue.h"
#include "spff/core/HealthPolicy.h"
#include "spff/core/IntervalTimer.h"
#include "spff/hw/BatteryGauge.h"
#include "spff/hw/FlowSensor.h"
#include "spff/core/SignalMonitor.h"
#include "spff/net/ApiClient.h"
#include "spff/net/IClock.h"
#include "spff/net/IFirmwareUpdater.h"
#include "spff/net/INetworkLink.h"
#include "spff/power/IPowerController.h"

namespace spff
{

// Orquestra o ciclo de vida do dispositivo. Conhece apenas abstrações
// (IClock, INetworkLink, ApiClient, IFirmwareUpdater) — nunca WiFi, HTTP ou ADC.
class Device
{
public:
  struct Dependencies
  {
    IClock &clock;
    INetworkLink &network;
    ApiClient &api;
    EventQueue &queue;
    FlowSensor &flow;
    BatteryGauge &battery;
    SignalMonitor &signal;
    SettingsStore &settings;
    IFirmwareUpdater &updater;
    IPowerController &power;
  };

  explicit Device(const Dependencies &dependencies);

  void begin();
  void tick();

private:
  void publishFlowEvents();
  int publishBatteryEvent();
  void publishSignalEvent();
  void publishQueuedEvents();
  void runNetworkCycle();
  bool sendHeartbeat(HeartbeatReply &reply);
  bool applyFirmwareUpdate(const HeartbeatReply &reply, int batteryPercent);
  void applyIntervalHint(uint32_t newIntervalMs);
  void reportStatus();

  Dependencies dependencies_;
  IntervalTimer timer_;
  HealthPolicy health_;
  const char *mac_ = "";
  uint32_t bootCount_ = 0;
  int lastBatteryPercent_ = -1;
};

} // namespace spff
