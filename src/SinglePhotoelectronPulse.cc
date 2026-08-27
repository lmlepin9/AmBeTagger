#include "AmBeTagger/SinglePhotoelectronPulse.hh"

#include "G4SystemOfUnits.hh"

#include <cmath>

namespace AmBeTagger
{
namespace
{
constexpr G4double kAmplitude = 1.0;
constexpr G4double kDecayTime = 25.0 * ns;
}

G4double SinglePhotoelectronPulse::Amplitude(G4double timeAfterPe) const
{
  if (timeAfterPe < 0.0) {
    return 0.0;
  }

  return kAmplitude * std::exp(-timeAfterPe / kDecayTime);
}
}  // namespace AmBeTagger
