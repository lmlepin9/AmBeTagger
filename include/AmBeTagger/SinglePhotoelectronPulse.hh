#pragma once

#include "globals.hh"

namespace AmBeTagger
{
class SinglePhotoelectronPulse final
{
 public:
  G4double Amplitude(G4double timeAfterPe) const;
};
}
