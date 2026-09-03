#include "WaveformCharge.hh"

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

namespace
{
bool CloseEnough(double actual, double expected, double tolerance = 1.0e-12)
{
  return std::abs(actual - expected) < tolerance;
}

bool CheckStatus(const AmBeTagger::Analysis::ChargeResult& result,
                 AmBeTagger::Analysis::ChargeStatus expected,
                 const char* testName)
{
  if (result.status == expected) {
    return true;
  }

  std::cerr << testName << " returned an unexpected status\n";
  return false;
}
}

int main()
{
  using AmBeTagger::Analysis::CalculateIntegratedCharge;
  using AmBeTagger::Analysis::ChargeIntegrationConfig;
  using AmBeTagger::Analysis::ChargeStatus;

  const ChargeIntegrationConfig config{2.0, 0, 3, 3, 6};

  const auto knownPulse =
      CalculateIntegratedCharge({5.0, 5.0, 5.0, 6.0, 8.0, 5.0, 100.0}, config);
  if (!knownPulse.IsValid() || !CloseEnough(knownPulse.baseline, 5.0) ||
      !CloseEnough(knownPulse.integratedCharge, 8.0)) {
    std::cerr << "Known-pulse charge calculation failed\n";
    return 1;
  }

  const auto constantBaseline =
      CalculateIntegratedCharge({4.0, 4.0, 4.0, 4.0, 4.0, 4.0}, config);
  if (!constantBaseline.IsValid() ||
      !CloseEnough(constantBaseline.integratedCharge, 0.0)) {
    std::cerr << "Constant-baseline charge calculation failed\n";
    return 1;
  }

  const ChargeIntegrationConfig offsetBaselineConfig{0.5, 1, 3, 3, 5};
  const auto offsetBaseline = CalculateIntegratedCharge(
      {99.0, 2.0, 4.0, 6.0, 8.0}, offsetBaselineConfig);
  if (!offsetBaseline.IsValid() || !CloseEnough(offsetBaseline.baseline, 3.0) ||
      !CloseEnough(offsetBaseline.integratedCharge, 4.0)) {
    std::cerr << "Offset-baseline charge calculation failed\n";
    return 1;
  }

  const ChargeIntegrationConfig negativeChargeConfig{1.0, 0, 2, 2, 4};
  const auto negativeCharge =
      CalculateIntegratedCharge({2.0, 2.0, 1.0, 0.0}, negativeChargeConfig);
  if (!negativeCharge.IsValid() ||
      !CloseEnough(negativeCharge.integratedCharge, -3.0)) {
    std::cerr << "Negative-charge calculation failed\n";
    return 1;
  }

  if (!CheckStatus(CalculateIntegratedCharge({}, config),
                   ChargeStatus::EmptyWaveform,
                   "Empty waveform")) {
    return 1;
  }

  if (!CheckStatus(CalculateIntegratedCharge({1.0, 1.0}, config),
                   ChargeStatus::IncompleteWaveform,
                   "Incomplete baseline window")) {
    return 1;
  }

  if (!CheckStatus(
          CalculateIntegratedCharge({1.0, 1.0, 1.0, 1.0}, config),
          ChargeStatus::IncompleteWaveform,
          "Incomplete integration window")) {
    return 1;
  }

  const std::vector<ChargeIntegrationConfig> invalidConfigurations = {
      {0.0, 0, 3, 3, 6},
      {-1.0, 0, 3, 3, 6},
      {1.0, 3, 3, 3, 6},
      {1.0, 0, 3, 6, 6},
      {1.0, 0, 4, 3, 6},
  };
  for (const auto& invalidConfig : invalidConfigurations) {
    if (!CheckStatus(CalculateIntegratedCharge(
                         {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, invalidConfig),
                     ChargeStatus::InvalidConfiguration,
                     "Invalid configuration")) {
      return 1;
    }
  }

  const auto nonFiniteSample = CalculateIntegratedCharge(
      {1.0,
       1.0,
       1.0,
       2.0,
       std::numeric_limits<double>::quiet_NaN(),
       2.0},
      config);
  if (!CheckStatus(nonFiniteSample,
                   ChargeStatus::NonFiniteSample,
                   "Non-finite waveform sample")) {
    return 1;
  }

  std::cout << "Waveform charge checks passed\n";
  return 0;
}
