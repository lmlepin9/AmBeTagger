#pragma once

#include "globals.hh"
#include "AmBeTagger/SinglePhotoelectronPulse.hh"
#include <vector>

namespace AmBeTagger
{
class WaveformBuilder final
{
 public:
  std::vector<G4double> Build(const std::vector<G4double>& photoelectronTimes) const;
  G4int SampleCount() const;
  G4double SampleSpacing() const;

 private:
  SinglePhotoelectronPulse singlePE_;
};
}
