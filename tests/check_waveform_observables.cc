#include "AmBeTagger/WaveformObservables.hh"

#include "G4SystemOfUnits.hh"

#include <cmath>
#include <iostream>
#include <vector>

namespace
{
bool CloseEnough(double actual, double expected, double tolerance)
{
  return std::abs(actual - expected) < tolerance;
}
}

int main()
{
  const AmBeTagger::WaveformObservables observables;
  constexpr G4double sampleSpacing = 2.0 * ns;

  const AmBeTagger::WaveformSummary emptySummary =
      observables.Analyze({}, sampleSpacing);

  if (!CloseEnough(emptySummary.peakAmplitude, 0.0, 1.0e-12) ||
      emptySummary.peakSample != 0 ||
      !CloseEnough(emptySummary.peakTime, 0.0, 1.0e-12) ||
      !CloseEnough(emptySummary.integral, 0.0, 1.0e-12)) {
    std::cerr << "Empty waveform summary failed\n";
    return 1;
  }

  const std::vector<G4double> zeroWaveform = {0.0, 0.0, 0.0};
  const AmBeTagger::WaveformSummary zeroSummary =
      observables.Analyze(zeroWaveform, sampleSpacing);

  if (!CloseEnough(zeroSummary.peakAmplitude, 0.0, 1.0e-12) ||
      zeroSummary.peakSample != 0 ||
      !CloseEnough(zeroSummary.peakTime, 0.0, 1.0e-12) ||
      !CloseEnough(zeroSummary.integral, 0.0, 1.0e-12)) {
    std::cerr << "Zero waveform summary failed\n";
    return 1;
  }

  const std::vector<G4double> waveform = {0.0, 1.0, 3.0, 2.0};
  const AmBeTagger::WaveformSummary summary =
      observables.Analyze(waveform, sampleSpacing);

  if (!CloseEnough(summary.peakAmplitude, 3.0, 1.0e-12)) {
    std::cerr << "Peak amplitude failed: " << summary.peakAmplitude << '\n';
    return 1;
  }

  if (summary.peakSample != 2) {
    std::cerr << "Peak sample failed: " << summary.peakSample << '\n';
    return 1;
  }

  if (!CloseEnough(summary.peakTime, 4.0 * ns, 1.0e-12)) {
    std::cerr << "Peak time failed: " << summary.peakTime / ns << " ns\n";
    return 1;
  }

  if (!CloseEnough(summary.integral, 12.0 * ns, 1.0e-12)) {
    std::cerr << "Integral failed: " << summary.integral / ns << '\n';
    return 1;
  }

  const std::vector<G4double> negativeWaveform = {-4.0, -2.0, -3.0};
  const AmBeTagger::WaveformSummary negativeSummary =
      observables.Analyze(negativeWaveform, sampleSpacing);

  if (!CloseEnough(negativeSummary.peakAmplitude, -2.0, 1.0e-12) ||
      negativeSummary.peakSample != 1 ||
      !CloseEnough(negativeSummary.peakTime, 2.0 * ns, 1.0e-12) ||
      !CloseEnough(negativeSummary.integral, -18.0 * ns, 1.0e-12)) {
    std::cerr << "Negative waveform maximum-value convention failed\n";
    return 1;
  }

  std::cout << "WaveformObservables checks passed\n";
  return 0;
}
