#pragma once

#include "globals.hh"

namespace AmBeTagger
{
class PmtResponse final
{
 public:
  G4double QuantumEfficiency(G4double photonEnergy) const;
  G4bool ProducesPhotoelectron(G4double photonEnergy) const;
};
}