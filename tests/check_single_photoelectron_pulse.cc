#include "AmBeTagger/SinglePhotoelectronPulse.hh"

#include "G4SystemOfUnits.hh"

#include <cmath>
#include <iostream>

namespace
{
constexpr double kTauRiseNs = 1.5;
constexpr double kTauFallNs = 5.0;

bool CloseEnough(double actual, double expected, double tolerance)
{
  return std::abs(actual - expected) < tolerance;
}

double ExpectedPulseAmplitude(double timeAfterTransitNs)
{
  return (1.0 - std::exp(-timeAfterTransitNs / kTauRiseNs))
      * std::exp(-timeAfterTransitNs / kTauFallNs);
}
}

int main()
{
  const AmBeTagger::SinglePhotoelectronPulse pulse;

  if (!CloseEnough(pulse.Amplitude(-1.0 * ns), 0.0, 1.0e-12)) {
    std::cerr << "Negative-time amplitude failed: "
              << pulse.Amplitude(-1.0 * ns) << '\n';
    return 1;
  }

  if (!CloseEnough(pulse.Amplitude(0.0 * ns), 0.0, 1.0e-12)) {
    std::cerr << "Zero-time amplitude failed: "
              << pulse.Amplitude(0.0 * ns) << '\n';
    return 1;
  }

  if (!CloseEnough(pulse.Amplitude(23.0 * ns), 0.0, 1.0e-12)) {
    std::cerr << "Transit-time amplitude failed: "
              << pulse.Amplitude(23.0 * ns) << '\n';
    return 1;
  }

  if (!CloseEnough(pulse.Amplitude(24.0 * ns),
                   ExpectedPulseAmplitude(1.0),
                   1.0e-12)) {
    std::cerr << "24 ns amplitude failed: "
              << pulse.Amplitude(24.0 * ns) << '\n';
    return 1;
  }

  if (!CloseEnough(pulse.Amplitude(28.0 * ns),
                   ExpectedPulseAmplitude(5.0),
                   1.0e-12)) {
    std::cerr << "28 ns amplitude failed: "
              << pulse.Amplitude(28.0 * ns) << '\n';
    return 1;
  }

  std::cout << "SinglePhotoelectronPulse checks passed\n";
  return 0;
}
