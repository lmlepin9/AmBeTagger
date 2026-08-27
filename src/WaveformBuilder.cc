#include "AmBeTagger/WaveformBuilder.hh"

#include "G4SystemOfUnits.hh"

#include <cmath>

namespace AmBeTagger
{
std::vector<G4double> WaveformBuilder::Build(
    const std::vector<G4double>& photoelectronTimes) const
{
  constexpr G4int sampleCount = 200;
  constexpr G4double sampleSpacing = 10.0 * ns;

  std::vector<G4double> waveform(sampleCount, 0.0);

  for (G4int sample = 0; sample < sampleCount; ++sample) {
    const G4double time = sample * sampleSpacing;

    for (G4double photoelectronTime : photoelectronTimes) {
      waveform[sample] += ToySinglePhotoelectronPulse(time - photoelectronTime);
    }
  }

  return waveform;
}

G4double WaveformBuilder::ToySinglePhotoelectronPulse(G4double timeAfterPe) const
{
  if (timeAfterPe < 0.0) {
    return 0.0;
  }

  constexpr G4double amplitude = 1.0;
  constexpr G4double decayTime = 25.0 * ns;

  return amplitude * std::exp(-timeAfterPe / decayTime);
}
}