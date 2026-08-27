#pragma once

#include "globals.hh"

#include <vector>

namespace AmBeTagger
{
class WaveformBuilder final
{
 public:
  std::vector<G4double> Build(const std::vector<G4double>& photoelectronTimes) const;

 private:
  G4double ToySinglePhotoelectronPulse(G4double timeAfterPe) const;
};
}