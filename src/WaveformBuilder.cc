#include "AmBeTagger/WaveformBuilder.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <cstddef>

namespace
{
constexpr G4int kSampleCount = 1500;
constexpr G4double kSampleSpacing = 2.0 * ns;
}

namespace AmBeTagger
{

WaveformBuilder::WaveformBuilder(G4double noiseSigma,
                                 G4double gainMean,
                                 G4double gainSigma)
    : gain_(gainMean, gainSigma),
      noiseSigma_(noiseSigma)
{
}


std::vector<G4double> WaveformBuilder::Build(
    const std::vector<G4double>& photoelectronTimes) const
{
  std::vector<G4double> photoelectronGains;
  photoelectronGains.reserve(photoelectronTimes.size());

  for (std::size_t i = 0; i < photoelectronTimes.size(); ++i) {
    photoelectronGains.push_back(gain_.Sample());
  }

  std::vector<G4double> waveform(SampleCount(), 0.0);
  for (G4int sample = 0; sample < SampleCount(); ++sample) {
    if (noiseSigma_ > 0.0) {
      waveform[sample] = G4RandGauss::shoot(0.0, noiseSigma_);
    }

    const G4double time = sample * SampleSpacing();
    for (std::size_t i = 0; i < photoelectronTimes.size(); ++i) {
      waveform[sample] += photoelectronGains[i]
          * singlePE_.Amplitude(time - photoelectronTimes[i]);
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
