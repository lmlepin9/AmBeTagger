#include "AmBeTagger/WaveformBuilder.hh"

#include "G4SystemOfUnits.hh"

#include <cmath>

namespace
{
constexpr G4int kSampleCount = 200;
constexpr G4double kSampleSpacing = 10.0 * ns;
}

namespace AmBeTagger
{
std::vector<G4double> WaveformBuilder::Build(
    const std::vector<G4double>& photoelectronTimes) const
{
  std::vector<G4double> waveform(SampleCount(), 0.0);

  for (G4int sample = 0; sample < SampleCount(); ++sample) {
    const G4double time = sample * SampleSpacing();
    for (G4double photoelectronTime : photoelectronTimes) {
      waveform[sample] += singlePE_.Amplitude(time - photoelectronTime);
    }
  }

  return waveform;
}

G4int WaveformBuilder::SampleCount() const
{
  return kSampleCount;
}

G4double WaveformBuilder::SampleSpacing() const
{
  return kSampleSpacing;
}

}
