#include "AmBeTagger/WaveformBuilder.hh"

#include "G4SystemOfUnits.hh"

#include <cmath>
#include <iostream>
#include <vector>
#include <cstddef>

namespace
{
bool CloseEnough(double actual, double expected, double tolerance)
{
  return std::abs(actual - expected) < tolerance;
}
}

int main()
{
  const AmBeTagger::WaveformBuilder builder;

  if (!CloseEnough(builder.SampleSpacing(), 10.0 * ns, 1.0e-12)) {
  std::cerr << "Sample spacing failed: " << builder.SampleSpacing() / ns
            << " ns\n";
  return 1;
}

  const std::vector<G4double> peTimes = {0.0 * ns};
  const std::vector<G4double> waveform = builder.Build(peTimes);


 if (waveform.size() != static_cast<std::size_t>(builder.SampleCount())) {
  std::cerr << "Expected " << builder.SampleCount()
            << " samples, got " << waveform.size() << '\n';
  return 1;
}

  if (!CloseEnough(waveform[0], 1.0, 1.0e-12)) {
    std::cerr << "Sample 0 failed: " << waveform[0] << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[1], std::exp(-10.0 / 25.0), 1.0e-12)) {
    std::cerr << "Sample 1 failed: " << waveform[1] << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[2], std::exp(-20.0 / 25.0), 1.0e-12)) {
    std::cerr << "Sample 2 failed: " << waveform[2] << '\n';
    return 1;
  }

const std::vector<G4double> delayedPeTimes = {20.0 * ns};
const std::vector<G4double> delayedWaveform =
    builder.Build(delayedPeTimes);

if (!CloseEnough(delayedWaveform[0], 0.0, 1.0e-12)) {
  std::cerr << "Delayed sample 0 failed: " << delayedWaveform[0] << '\n';
  return 1;
}

if (!CloseEnough(delayedWaveform[1], 0.0, 1.0e-12)) {
  std::cerr << "Delayed sample 1 failed: " << delayedWaveform[1] << '\n';
  return 1;
}

if (!CloseEnough(delayedWaveform[2], 1.0, 1.0e-12)) {
  std::cerr << "Delayed sample 2 failed: " << delayedWaveform[2] << '\n';
  return 1;
}

const std::vector<G4double> twoPeTimes = {0.0 * ns, 20.0 * ns};
const std::vector<G4double> twoPeWaveform =
    builder.Build(twoPeTimes);

const double expectedSample2 =
    std::exp(-20.0 / 25.0) + 1.0;

if (!CloseEnough(twoPeWaveform[2], expectedSample2, 1.0e-12)) {
  std::cerr << "Two-PE sample 2 failed: " << twoPeWaveform[2] << '\n';
  return 1;
}



  std::cout << "WaveformBuilder checks passed\n";
  return 0;
}