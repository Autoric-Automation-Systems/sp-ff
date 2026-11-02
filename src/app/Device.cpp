#include "spff/app/Device.h"

#include <Arduino.h>
#include <cstdlib>
#include <esp_system.h>

#include "spff/Config.h"
#include "spff/Logger.h"
#include "spff/core/Event.h"
#include "spff/core/FirmwareVersion.h"
#include "spff/hw/PersistentStorage.h"

namespace spff
{
namespace
{

constexpr int kBatteryReportDelta = 5;
constexpr const char *kBatteryEvent = "battery";
constexpr const char *kWifiEvent = "wifi";
constexpr const char *kFlowEvent = "flow";

const char *resetReasonName(esp_reset_reason_t reason)
{
  switch (reason)
  {
  case ESP_RST_POWERON:
    return "POWERON";
  case ESP_RST_EXT:
    return "EXTERNAL";
  case ESP_RST_SW:
    return "SOFTWARE";
  case ESP_RST_PANIC:
    return "PANIC";
  case ESP_RST_INT_WDT:
    return "INT_WDT";
  case ESP_RST_TASK_WDT:
    return "TASK_WDT";
  case ESP_RST_WDT:
    return "WDT";
  case ESP_RST_DEEPSLEEP:
    return "DEEPSLEEP";
  case ESP_RST_BROWNOUT:
    return "BROWNOUT";
  case ESP_RST_SDIO:
    return "SDIO";
  default:
    return "UNKNOWN";
  }
}

Event makeEvent(const char *name, float value, uint32_t ts)
{
  Event event{};
  setEventName(event, name);
  event.value = roundEventValue(value);
  event.ts = ts;
  return event;
}

} // namespace

Device::Device(const Dependencies &dependencies)
    : dependencies_(dependencies),
      health_(config::kPortalAfterWifiFailures, config::kRestartAfterSendFailures)
{
}

void Device::begin()
{
  bootCount_ = nextBootCount();
  dependencies_.queue.restore();
  log::begin(115200);

  log::blank();
  log::info("Boot Smart Plant %s | %s | boot #%lu | reset: %s | slot: %s",
            config::kFirmwareVersion, config::kDeviceType,
            static_cast<unsigned long>(bootCount_), resetReasonName(esp_reset_reason()),
            dependencies_.power.firmwareSlot());

  // NVS só é acessada depois do boot do Arduino (nunca em construtor global).
  timer_.setPeriod(sanitizeInterval(dependencies_.settings.heartbeatInterval(), config::kHeartbeatLimits));

  mac_ = dependencies_.network.macAddress();
  dependencies_.signal.seed(lastReportedRssi());
  log::info("MAC: %s | intervalo: %lu ms | fila: %u evento(s)",
            mac_, static_cast<unsigned long>(timer_.period()),
            static_cast<unsigned>(dependencies_.queue.size()));

  dependencies_.flow.begin();
  dependencies_.network.autoConnect(config::kPortalTimeoutSeconds);
  dependencies_.clock.ensureSynced(config::kNtpTimeoutMs);

  timer_.start(millis());
}

void Device::tick()
{
  publishFlowEvents();

  if (!timer_.due(millis()))
  {
    return;
  }

  timer_.scheduleNext(millis());
  runNetworkCycle();

  if (health_.consumeReconfigurationRequest())
  {
    dependencies_.network.openPortal(config::kPortalTimeoutSeconds);
  }

  if (health_.requiresRestart())
  {
    log::info("%u ciclos seguidos sem envio, reiniciando...",
              static_cast<unsigned>(health_.sendFailures()));
    dependencies_.power.restart();
  }
}

void Device::runNetworkCycle()
{
  const int batteryPercent = publishBatteryEvent();

  const bool connected = dependencies_.network.connect(config::kWifiConnectRetries);
  health_.recordWifi(connected);

  bool heartbeatOk = false;
  if (connected)
  {
    dependencies_.clock.ensureSynced(config::kNtpTimeoutMs);
    publishSignalEvent();
    publishQueuedEvents();

    HeartbeatReply reply{};
    heartbeatOk = sendHeartbeat(reply);
    if (heartbeatOk && applyFirmwareUpdate(reply, batteryPercent))
    {
      // A imagem já está validada na partição inativa: o reboot a ativa.
      dependencies_.power.restart();
      return;
    }

    dependencies_.network.disconnect();
  }
  else
  {
    log::info("Sem WiFi (%u ciclos seguidos), nada enviado",
              static_cast<unsigned>(health_.wifiFailures()));
  }

  health_.recordSend(heartbeatOk);
  reportStatus();
}

void Device::publishFlowEvents()
{
  const uint32_t liters = dependencies_.flow.consumeLiters();
  for (uint32_t i = 0; i < liters; i++)
  {
    if (!dependencies_.queue.push(makeEvent(kFlowEvent, 1.0f, dependencies_.clock.epochNow())))
    {
      log::info("Fila cheia: evento mais antigo descartado");
    }
  }
}

int Device::publishBatteryEvent()
{
  const BatteryReading reading = dependencies_.battery.read();
  log::info("Bateria: %.2f V (%d%%)", reading.volts, reading.percent);

  if (lastBatteryPercent_ < 0 || abs(reading.percent - lastBatteryPercent_) >= kBatteryReportDelta)
  {
    dependencies_.queue.push(
        makeEvent(kBatteryEvent, static_cast<float>(reading.percent), dependencies_.clock.epochNow()));
    lastBatteryPercent_ = reading.percent;
  }
  return reading.percent;
}

void Device::publishSignalEvent()
{
  const int rssi = dependencies_.network.rssi();
  if (!SignalMonitor::isValid(rssi))
  {
    log::info("RSSI inválido: %d", rssi);
    return;
  }
  if (dependencies_.signal.shouldReport(rssi))
  {
    saveLastReportedRssi(rssi);
    dependencies_.queue.push(makeEvent(kWifiEvent, static_cast<float>(rssi), dependencies_.clock.epochNow()));
  }
}

void Device::publishQueuedEvents()
{
  if (dependencies_.queue.empty())
  {
    return;
  }

  const uint32_t now = dependencies_.clock.epochNow();
  if (now == 0)
  {
    log::info("Sem hora válida: eventos continuam na fila");
    return;
  }

  const size_t delivered = dependencies_.api.sendEvents(
      mac_, dependencies_.queue.data(), dependencies_.queue.size(),
      config::kEventsPerBatch, now);

  if (delivered > 0)
  {
    dependencies_.queue.dropFront(delivered);
  }
  if (!dependencies_.queue.empty())
  {
    log::info("Envio incompleto: %u evento(s) mantidos na fila",
              static_cast<unsigned>(dependencies_.queue.size()));
  }
}

bool Device::sendHeartbeat(HeartbeatReply &reply)
{
  HeartbeatStatus status{};
  status.mac = mac_;
  status.deviceType = config::kDeviceType;
  status.version = config::kFirmwareVersion;
  status.rssi = dependencies_.network.rssi();
  status.ip = dependencies_.network.localIp();
  status.uptimeMs = millis();
  status.boots = bootCount_;
  status.freeHeap = ESP.getFreeHeap();

  const bool ok = dependencies_.api.sendHeartbeat(status, reply);
  applyIntervalHint(reply.intervalMs);
  return ok;
}

bool Device::applyFirmwareUpdate(const HeartbeatReply &reply, int batteryPercent)
{
  const char *url = reply.firmwareUrl;
  bool skipVersionCheck = false;

  // Atalho de bancada: força o download sem depender da resposta do servidor.
  if (config::kForcedUpdateUrl[0] != '\0')
  {
    url = config::kForcedUpdateUrl;
    skipVersionCheck = true;
  }

  if (url == nullptr || url[0] == '\0')
  {
    return false;
  }

  if (batteryPercent >= 0 && batteryPercent < config::kMinBatteryPercentForUpdate)
  {
    log::info("OTA adiado: bateria em %d%% (mínimo %d%%)", batteryPercent,
              config::kMinBatteryPercentForUpdate);
    return false;
  }

  if (!skipVersionCheck && !isNewerVersion(reply.firmwareVersion, config::kFirmwareVersion))
  {
    log::info("OTA: versão %s não é mais nova que %s", reply.firmwareVersion,
              config::kFirmwareVersion);
    return false;
  }

  log::info("OTA: %s -> %s", config::kFirmwareVersion,
            skipVersionCheck ? "forçada (bancada)" : reply.firmwareVersion);
  return dependencies_.updater.install(url);
}

void Device::applyIntervalHint(uint32_t newIntervalMs)
{
  if (newIntervalMs == 0)
  {
    log::info("heartbeatInterval ausente na resposta");
    return;
  }
  if (!isIntervalAllowed(newIntervalMs, config::kHeartbeatLimits))
  {
    log::info("heartbeatInterval fora da faixa, ignorado: %lu",
              static_cast<unsigned long>(newIntervalMs));
    return;
  }
  if (newIntervalMs == timer_.period())
  {
    log::info("heartbeatInterval sem alteração");
    return;
  }

  timer_.setPeriod(newIntervalMs);
  dependencies_.settings.saveHeartbeatInterval(newIntervalMs);
  timer_.scheduleNext(millis());
  log::info("Novo intervalo salvo: %lu ms", static_cast<unsigned long>(newIntervalMs));
}

void Device::reportStatus()
{
  log::info("Heap livre: %lu | fila: %u evento(s) | descartados: %lu",
            static_cast<unsigned long>(ESP.getFreeHeap()),
            static_cast<unsigned>(dependencies_.queue.size()),
            static_cast<unsigned long>(dependencies_.queue.dropped()));
}

} // namespace spff
