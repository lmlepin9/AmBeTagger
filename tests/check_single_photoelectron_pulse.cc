#include "AmBeTagger/SinglePhotoelectronPulse.hh"

#include "G4SystemOfUnits.hh"

#include <cmath>
#include <iostream>

namespace
{
bool CloseEnough(double actual, double expected, double tolerance)
{
  return std::abs(actual - expected) < tolerance;
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

  if (!CloseEnough(pulse.Amplitude(0.0 * ns), 1.0, 1.0e-12)) {
    std::cerr << "Zero-time amplitude failed: "
              << pulse.Amplitude(0.0 * ns) << '\n';
    return 1;
  }

  if (!CloseEnough(pulse.Amplitude(10.0 * ns),
                   std::exp(-10.0 / 25.0),
                   1.0e-12)) {
    std::cerr << "10 ns amplitude failed: "
              << pulse.Amplitude(10.0 * ns) << '\n';
    return 1;
  }

  if (!CloseEnough(pulse.Amplitude(20.0 * ns),
                   std::exp(-20.0 / 25.0),
                   1.0e-12)) {
    std::cerr << "20 ns amplitude failed: "
              << pulse.Amplitude(20.0 * ns) << '\n';
    return 1;
  }

  std::cout << "SinglePhotoelectronPulse checks passed\n";
  return 0;
}
