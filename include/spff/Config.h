#pragma once

#include <cstdint>

#include "spff/core/IntervalLimits.h"

namespace spff
{
namespace config
{

// A versão pode ser sobrescrita no build para ensaiar o OTA:
//   -DSPFF_FIRMWARE_VERSION='"1.3"'
#ifndef SPFF_FIRMWARE_VERSION
#define SPFF_FIRMWARE_VERSION "1.3"
#endif

constexpr const char *kFirmwareVersion = SPFF_FIRMWARE_VERSION;
constexpr const char *kDeviceType = "SP-FF";
constexpr const char *kPortalSsid = "SmartPlant SP-FF";

// 1 = servidor local de desenvolvimento
#ifndef SPFF_LOCAL_SERVER
#define SPFF_LOCAL_SERVER 0
#endif

#if SPFF_LOCAL_SERVER
constexpr const char *kBaseUrl = "http://192.168.100.103:3000";
#else
constexpr const char *kBaseUrl = "https://smartplant.app.br";
#endif

// ================= Hardware =================
constexpr uint8_t kBatteryPin = 1;
constexpr uint8_t kFlowPin = 5;

// Placa medidora 0-25 VDC (divisor 30k/7.5k -> 5:1). Vbat = Vpin * 5.
constexpr float kBatteryDivider = 5.0f;
constexpr uint8_t kBatterySamples = 8;
constexpr uint16_t kPulsesPerLiter = 450;

// ================= Fila de eventos =================
constexpr uint16_t kEventCapacity = 64;
constexpr uint8_t kEventsPerBatch = 20;

// ================= Heartbeat =================
// 10 s é o padrão de bancada; o servidor devolve o valor de produção
// (normalmente 600000 ms = 10 min) e ele é persistido em NVS.
#ifndef DEFAULT_HEARTBEAT_MS
#define DEFAULT_HEARTBEAT_MS 10000UL
#endif
#ifndef MIN_HEARTBEAT_MS
#define MIN_HEARTBEAT_MS 5000UL
#endif
#ifndef MAX_HEARTBEAT_MS
#define MAX_HEARTBEAT_MS 3600000UL
#endif

constexpr uint32_t kDefaultHeartbeatMs = DEFAULT_HEARTBEAT_MS;
constexpr uint32_t kMinHeartbeatMs = MIN_HEARTBEAT_MS;
constexpr uint32_t kMaxHeartbeatMs = MAX_HEARTBEAT_MS;

constexpr IntervalLimits kHeartbeatLimits{kMinHeartbeatMs, kMaxHeartbeatMs, kDefaultHeartbeatMs};

// ================= Rede =================
constexpr uint16_t kWifiConnectRetries = 20; // 20 x 500 ms = 10 s
constexpr uint16_t kPortalTimeoutSeconds = 90;
constexpr uint16_t kNtpTimeoutMs = 5000;
constexpr int kRssiReportDeltaDbm = 10;

// ================= Política de saúde =================
constexpr uint8_t kPortalAfterWifiFailures = 3;
constexpr uint8_t kRestartAfterSendFailures = 6;

// ================= HTTP / TLS =================
constexpr uint32_t kHttpConnectTimeoutMs = 10000;
constexpr uint32_t kHttpReadTimeoutMs = 15000;

// 1 = valida o certificado do servidor (include/spff/roots_ca.h)
#ifndef TLS_VERIFY
#define TLS_VERIFY 1
#endif

// Chave compartilhada do servidor (DEVICE_API_KEY). Vazia = header não enviado.
// Defina no build: -DDEVICE_API_KEY='"chave"' (nunca commite o segredo)
#ifndef DEVICE_API_KEY
#define DEVICE_API_KEY ""
#endif
constexpr const char *kDeviceApiKey = DEVICE_API_KEY;

// ================= OTA =================
// A atualização é aplicada na partição inativa e só com bateria suficiente:
// uma queda no meio do download não afeta o firmware em execução.
constexpr int kMinBatteryPercentForUpdate = 20;
constexpr uint32_t kOtaConnectTimeoutMs = 15000;
constexpr uint32_t kOtaReadTimeoutMs = 30000;

// Somente bancada: força a atualização a partir deste URL no próximo ciclo,
// ignorando o que o servidor informa. Vazio = comportamento normal.
#ifndef SPFF_OTA_FORCE_URL
#define SPFF_OTA_FORCE_URL ""
#endif
constexpr const char *kForcedUpdateUrl = SPFF_OTA_FORCE_URL;

} // namespace config
} // namespace spff
