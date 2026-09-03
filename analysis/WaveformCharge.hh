#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

namespace AmBeTagger
{
namespace Analysis
{
struct ChargeIntegrationConfig
{
  double sampleSpacing = 2.0;
  std::size_t baselineStartSample = 0;
  std::size_t baselineEndSample = 300;
  std::size_t integrationStartSample = 300;
  std::size_t integrationEndSample = 1200;
};

enum class ChargeStatus
{
  Valid,
  EmptyWaveform,
  IncompleteWaveform,
  NonFiniteSample,
  InvalidConfiguration,
};

struct ChargeResult
{
  double integratedCharge = 0.0;
  double baseline = 0.0;
  ChargeStatus status = ChargeStatus::InvalidConfiguration;

  bool IsValid() const
  {
    return status == ChargeStatus::Valid;
  }
};

inline ChargeResult CalculateIntegratedCharge(
    const std::vector<double>& waveform,
    const ChargeIntegrationConfig& config)
{
  ChargeResult result;

  const bool hasValidSampleSpacing =
      std::isfinite(config.sampleSpacing) && config.sampleSpacing > 0.0;
  const bool hasValidBaselineWindow =
      config.baselineStartSample < config.baselineEndSample;
  const bool hasValidIntegrationWindow =
      config.integrationStartSample < config.integrationEndSample;
  const bool hasNonOverlappingWindows =
      config.baselineEndSample <= config.integrationStartSample;

  if (!hasValidSampleSpacing || !hasValidBaselineWindow ||
      !hasValidIntegrationWindow || !hasNonOverlappingWindows) {
    return result;
  }

  if (waveform.empty()) {
    result.status = ChargeStatus::EmptyWaveform;
    return result;
  }

  if (config.baselineEndSample > waveform.size() ||
      config.integrationEndSample > waveform.size()) {
    result.status = ChargeStatus::IncompleteWaveform;
    return result;
  }

  double baselineSum = 0.0;
  for (std::size_t iSample = config.baselineStartSample;
       iSample < config.baselineEndSample;
       ++iSample) {
    if (!std::isfinite(waveform[iSample])) {
      result.status = ChargeStatus::NonFiniteSample;
      return result;
    }
    baselineSum += waveform[iSample];
  }

  const std::size_t baselineSampleCount =
      config.baselineEndSample - config.baselineStartSample;
  result.baseline = baselineSum / baselineSampleCount;

  double baselineSubtractedSum = 0.0;
  for (std::size_t iSample = config.integrationStartSample;
       iSample < config.integrationEndSample;
       ++iSample) {
    if (!std::isfinite(waveform[iSample])) {
      result.status = ChargeStatus::NonFiniteSample;
      return result;
    }
    baselineSubtractedSum += waveform[iSample] - result.baseline;
  }

  result.integratedCharge = baselineSubtractedSum * config.sampleSpacing;
  result.status = ChargeStatus::Valid;
  return result;
}
}
}
