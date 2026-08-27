#pragma once

#include "globals.hh"

namespace AmBeTagger
{
class SinglePhotoelectronGain final
{
 public:
  explicit SinglePhotoelectronGain(G4double mean = 0.005,
                                   G4double sigma = 0.0006);

  G4double Sample() const;
  G4double Mean() const;
  G4double Sigma() const;

 private:
  G4double mean_ = 1.0;
  G4double sigma_ = 0.0;
};
}
