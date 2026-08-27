#include "AmBeTagger/SinglePhotoelectronPulse.hh"

#include "G4SystemOfUnits.hh"

#include <cmath>

namespace AmBeTagger
{
namespace
{
constexpr G4double kTauRise = 1.5 * ns;
constexpr G4double kTauFall = 5.0 * ns;
constexpr G4double kTransitTime = 23.0 * ns;
}

G4double SinglePhotoelectronPulse::Amplitude(G4double timeAfterPe) const
{
  const G4double pulseTime = timeAfterPe - kTransitTime;
  if (pulseTime < 0.0) {
    return 0.0;
  }

  return (1.0 - std::exp(-pulseTime / kTauRise))
      * std::exp(-pulseTime / kTauFall);
}
}  // namespace AmBeTagger
