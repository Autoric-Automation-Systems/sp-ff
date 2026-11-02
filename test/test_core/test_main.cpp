#include <unity.h>

#include <Arduino.h>
#include <cstring>

#include "spff/core/BatteryCurve.h"
#include "spff/core/EventQueue.h"
#include "spff/core/HealthPolicy.h"
#include "spff/core/IntervalLimits.h"
#include "spff/core/IntervalTimer.h"

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

void setup()
{
  // Espera o monitor serial anexar depois do reset do gravador.
  delay(5000);
  UNITY_BEGIN();
  RUN_TEST(test_event_name_is_truncated);
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
  UNITY_END();
}

void loop()
{
}
