#include <unity.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>

#include "spff/core/BatteryCurve.h"
#include "spff/core/EventQueue.h"
#include "spff/core/FirmwareVersion.h"
#include "spff/core/HealthPolicy.h"
#include "spff/core/IntervalLimits.h"
#include "spff/core/IntervalTimer.h"
#include "spff/core/SignalMonitor.h"

using namespace spff;

namespace
{

Event makeEvent(const char *name, float value, uint32_t ts)
{
  Event event{};
  setEventName(event, name);
  event.value = value;
  event.ts = ts;
  return event;
}

QueueStorage g_storage;

} // namespace

void test_event_name_is_truncated()
{
  Event event{};
  setEventName(event, "verylongeventname");
  TEST_ASSERT_EQUAL_STRING("verylongeve", event.name);
}

void test_event_queue_keeps_order()
{
  QueueStorage storage{};
  EventQueue queue(storage);
  queue.restore();

  queue.push(makeEvent("flow", 1, 10));
  queue.push(makeEvent("wifi", -60, 20));

  TEST_ASSERT_EQUAL_UINT(2, queue.size());
  TEST_ASSERT_EQUAL_STRING("flow", queue.at(0).name);
  TEST_ASSERT_EQUAL_FLOAT(-60.0f, queue.at(1).value);
  TEST_ASSERT_EQUAL_UINT32(20, queue.at(1).ts);
}

void test_event_queue_drops_oldest_when_full()
{
  EventQueue queue(g_storage);
  queue.restore();

  for (uint16_t i = 0; i < QueueStorage::kCapacity; i++)
  {
    queue.push(makeEvent("flow", static_cast<float>(i), 0));
  }

  const bool kept = queue.push(makeEvent("last", 99, 0));

  TEST_ASSERT_FALSE(kept);
  TEST_ASSERT_EQUAL_UINT(QueueStorage::kCapacity, queue.size());
  TEST_ASSERT_EQUAL_FLOAT(1.0f, queue.at(0).value);
  TEST_ASSERT_EQUAL_STRING("last", queue.at(queue.size() - 1).name);
  TEST_ASSERT_EQUAL_UINT32(1, queue.dropped());
}

void test_event_queue_restore_rejects_unknown_memory()
{
  QueueStorage storage{};
  storage.count = 5;
  storage.magic = 0xDEADBEEF;

  EventQueue queue(storage);
  queue.restore();

  TEST_ASSERT_EQUAL_UINT(0, queue.size());
  TEST_ASSERT_EQUAL_UINT32(QueueStorage::kValidMagic, storage.magic);
}

void test_event_queue_drop_front()
{
  QueueStorage storage{};
  EventQueue queue(storage);
  queue.restore();

  for (int i = 0; i < 5; i++)
  {
    queue.push(makeEvent("flow", static_cast<float>(i), 0));
  }

  queue.dropFront(2);
  TEST_ASSERT_EQUAL_UINT(3, queue.size());
  TEST_ASSERT_EQUAL_FLOAT(2.0f, queue.at(0).value);

  queue.dropFront(10);
  TEST_ASSERT_EQUAL_UINT(0, queue.size());
}

void test_battery_curve_percentages()
{
  TEST_ASSERT_EQUAL_INT(100, battery::percentFromVoltage(4.30f));
  TEST_ASSERT_EQUAL_INT(100, battery::percentFromVoltage(4.20f));
  TEST_ASSERT_EQUAL_INT(60, battery::percentFromVoltage(3.87f));
  TEST_ASSERT_EQUAL_INT(0, battery::percentFromVoltage(3.00f));

  const int interpolated = battery::percentFromVoltage(3.835f);
  TEST_ASSERT_INT_WITHIN(1, 53, interpolated);
}

void test_interval_timer_schedules_and_fires()
{
  IntervalTimer timer(1000);
  timer.start(0);
  TEST_ASSERT_TRUE(timer.due(0));

  timer.scheduleNext(0);
  TEST_ASSERT_FALSE(timer.due(999));
  TEST_ASSERT_TRUE(timer.due(1000));
}

void test_interval_timer_survives_millis_rollover()
{
  IntervalTimer timer(1000);
  const uint32_t nearRollover = 0xFFFFFE00UL;
  timer.start(nearRollover);
  timer.scheduleNext(nearRollover);

  TEST_ASSERT_FALSE(timer.due(nearRollover + 500));
  TEST_ASSERT_TRUE(timer.due(nearRollover + 1000));
}

void test_interval_limits_sanitize()
{
  const IntervalLimits limits{5000, 60000, 10000};

  TEST_ASSERT_TRUE(isIntervalAllowed(5000, limits));
  TEST_ASSERT_TRUE(isIntervalAllowed(60000, limits));
  TEST_ASSERT_FALSE(isIntervalAllowed(4999, limits));
  TEST_ASSERT_FALSE(isIntervalAllowed(60001, limits));

  TEST_ASSERT_EQUAL_UINT32(10000, sanitizeInterval(0, limits));
  TEST_ASSERT_EQUAL_UINT32(10000, sanitizeInterval(99999999UL, limits));
  TEST_ASSERT_EQUAL_UINT32(30000, sanitizeInterval(30000, limits));
}

void test_health_policy_restarts_after_send_failures()
{
  HealthPolicy policy(3, 6);

  for (int i = 0; i < 5; i++)
  {
    policy.recordSend(false);
  }
  TEST_ASSERT_FALSE(policy.requiresRestart());

  policy.recordSend(false);
  TEST_ASSERT_TRUE(policy.requiresRestart());

  policy.recordSend(true);
  TEST_ASSERT_FALSE(policy.requiresRestart());
  TEST_ASSERT_EQUAL_UINT8(0, policy.sendFailures());
}

void test_health_policy_requests_portal_once_per_block()
{
  HealthPolicy policy(3, 6);

  policy.recordWifi(false);
  policy.recordWifi(false);
  TEST_ASSERT_FALSE(policy.consumeReconfigurationRequest());

  policy.recordWifi(false);
  TEST_ASSERT_TRUE(policy.consumeReconfigurationRequest());
  TEST_ASSERT_FALSE(policy.consumeReconfigurationRequest());

  policy.recordWifi(true);
  TEST_ASSERT_EQUAL_UINT8(0, policy.wifiFailures());
}

void test_signal_monitor_uses_hysteresis()
{
  SignalMonitor monitor(10);

  TEST_ASSERT_TRUE(monitor.shouldReport(-60));
  TEST_ASSERT_FALSE(monitor.shouldReport(-65));
  TEST_ASSERT_TRUE(monitor.shouldReport(-45));

  // Depois de um auto-restart o último valor é semeado a partir do RTC
  SignalMonitor afterRestart(10);
  afterRestart.seed(-60);
  TEST_ASSERT_FALSE(afterRestart.shouldReport(-64));
  TEST_ASSERT_TRUE(afterRestart.shouldReport(-40));

  // Semente inválida é ignorada (0 = nunca medido)
  SignalMonitor empty(10);
  empty.seed(0);
  TEST_ASSERT_TRUE(empty.shouldReport(-70));
}

void test_event_value_is_rounded_to_two_decimals()
{
  TEST_ASSERT_EQUAL_FLOAT(29.64f, roundEventValue(29.63562012f));
  TEST_ASSERT_EQUAL_FLOAT(50.79f, roundEventValue(50.78725815f));
  TEST_ASSERT_EQUAL_FLOAT(-37.0f, roundEventValue(-37.0f));
  TEST_ASSERT_EQUAL_FLOAT(1.0f, roundEventValue(1.0f));
}

void test_event_queue_rejects_garbage_with_valid_magic()
{
  QueueStorage storage{};
  {
    EventQueue first(storage);
    first.restore();
    first.push(makeEvent("temperature", 20.0f, 100));
  }

  // Lixo deixado por outro firmware (flash/teste): magic e contagem plausíveis
  storage.slots[0].name[0] = '\x01';
  storage.slots[0].value = 0.0f;
  storage.slots[0].ts = 0;

  EventQueue restored(storage);
  restored.restore();

  TEST_ASSERT_EQUAL_UINT(0, restored.size());
}

void test_payload_has_at_most_two_decimals()
{
  StaticJsonDocument<96> document;
  char text[16];
  char buffer[48];

  formatEventValue(29.63562012f, text, sizeof(text));
  document["value"] = serialized(text);
  serializeJson(document, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"value\":29.64}", buffer, buffer);

  formatEventValue(50.78725815f, text, sizeof(text));
  document["value"] = serialized(text);
  serializeJson(document, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"value\":50.79}", buffer, buffer);

  // Inteiros continuam inteiros no JSON (sem .00)
  formatEventValue(-37.0f, text, sizeof(text));
  document["value"] = serialized(text);
  serializeJson(document, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"value\":-37}", buffer, buffer);

  formatEventValue(100.0f, text, sizeof(text));
  document["value"] = serialized(text);
  serializeJson(document, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"value\":100}", buffer, buffer);

  // Valor não finito nunca vira JSON inválido
  formatEventValue(NAN, text, sizeof(text));
  document["value"] = serialized(text);
  serializeJson(document, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"value\":0}", buffer, buffer);
}

void test_firmware_version_parsing_and_ordering()
{
  const FirmwareVersion full = parseFirmwareVersion("1.2.3");
  TEST_ASSERT_EQUAL_UINT16(1, full.major);
  TEST_ASSERT_EQUAL_UINT16(2, full.minor);
  TEST_ASSERT_EQUAL_UINT16(3, full.patch);

  const FirmwareVersion prefixed = parseFirmwareVersion("v1.4-beta");
  TEST_ASSERT_EQUAL_UINT16(1, prefixed.major);
  TEST_ASSERT_EQUAL_UINT16(4, prefixed.minor);

  // Comparacao numerica: texto puro diria que "1.10" < "1.9"
  TEST_ASSERT_TRUE(isNewerVersion("1.10", "1.9"));
  TEST_ASSERT_FALSE(isNewerVersion("1.9", "1.10"));
  TEST_ASSERT_TRUE(isNewerVersion("2.0", "1.99"));
  TEST_ASSERT_TRUE(isNewerVersion("1.2.1", "1.2"));
  TEST_ASSERT_FALSE(isNewerVersion("1.2.0", "1.2"));
  TEST_ASSERT_FALSE(isNewerVersion("1.2", "1.2"));
  TEST_ASSERT_FALSE(isNewerVersion("", "1.2"));
}

void setup()
{
  // Espera o monitor serial anexar depois do reset do gravador.
  delay(5000);
  UNITY_BEGIN();
  RUN_TEST(test_event_name_is_truncated);
  RUN_TEST(test_event_value_is_rounded_to_two_decimals);
  RUN_TEST(test_payload_has_at_most_two_decimals);
  RUN_TEST(test_event_queue_keeps_order);
  RUN_TEST(test_event_queue_drops_oldest_when_full);
  RUN_TEST(test_event_queue_restore_rejects_unknown_memory);
  RUN_TEST(test_event_queue_drop_front);
  RUN_TEST(test_battery_curve_percentages);
  RUN_TEST(test_interval_timer_schedules_and_fires);
  RUN_TEST(test_interval_timer_survives_millis_rollover);
  RUN_TEST(test_interval_limits_sanitize);
  RUN_TEST(test_health_policy_restarts_after_send_failures);
  RUN_TEST(test_health_policy_requests_portal_once_per_block);
  RUN_TEST(test_signal_monitor_uses_hysteresis);
  RUN_TEST(test_firmware_version_parsing_and_ordering);
  UNITY_END();
}

void loop()
{
}
