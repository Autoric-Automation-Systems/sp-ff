#pragma once

namespace spff
{

// Só reporta o RSSI quando ele muda o suficiente, para não transformar a
// oscilação natural do rádio em uma enxurrada de eventos.
class SignalMonitor
{
public:
  explicit SignalMonitor(int reportDeltaDbm) : reportDeltaDbm_(reportDeltaDbm) {}

  static bool isValid(int rssi) { return rssi <= 0 && rssi >= -100; }

  // Semeia o último valor conhecido (persistido em RTC) para que a histerese
  // continue valendo depois de um auto-restart.
  void seed(int rssi);

  bool shouldReport(int rssi);

private:
  int reportDeltaDbm_;
  int lastReportedRssi_ = 0;
  bool hasReading_ = false;
};

} // namespace spff
