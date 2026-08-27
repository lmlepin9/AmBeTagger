#include "AmBeTagger/SinglePhotoelectronGain.hh"

#include "Randomize.hh"

namespace AmBeTagger
{
SinglePhotoelectronGain::SinglePhotoelectronGain(G4double mean,
                                                 G4double sigma)
    : mean_(mean),
      sigma_(sigma)
{
}

G4double SinglePhotoelectronGain::Sample() const
{
  if (sigma_ <= 0.0) {
    return mean_;
  }

  return G4RandGauss::shoot(mean_, sigma_);
}

G4double SinglePhotoelectronGain::Mean() const
{
  return mean_;
}

G4double SinglePhotoelectronGain::Sigma() const
{
  return sigma_;
}
}
