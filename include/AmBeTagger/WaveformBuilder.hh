#pragma once

#include "AmBeTagger/SinglePhotoelectronGain.hh"
#include "AmBeTagger/SinglePhotoelectronPulse.hh"
#include "globals.hh"

#include <vector>

namespace AmBeTagger
{
class WaveformBuilder final
{
 public:
  explicit WaveformBuilder(G4double noiseSigma = 0.001,
                           G4double gainMean = 0.005,
                           G4double gainSigma = 0.0006);
  std::vector<G4double> Build(const std::vector<G4double>& photoelectronTimes) const;
  G4int SampleCount() const;
  G4double SampleSpacing() const;

 private:
  SinglePhotoelectronPulse singlePE_;
  SinglePhotoelectronGain gain_;
  G4double noiseSigma_ = 0.0;
};
}
