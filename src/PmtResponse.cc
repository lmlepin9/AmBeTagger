#include "AmBeTagger/PmtResponse.hh"

#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace
{
  static const std::vector<G4double> energy = {
    1.771 * eV, 1.784 * eV, 1.797 * eV, 1.810 * eV, 1.823 * eV,
    1.837 * eV, 1.850 * eV, 1.864 * eV, 1.878 * eV, 1.893 * eV,
    1.907 * eV, 1.922 * eV, 1.937 * eV, 1.952 * eV, 1.968 * eV,
    1.984 * eV, 2.000 * eV, 2.016 * eV, 2.032 * eV, 2.049 * eV,
    2.066 * eV, 2.084 * eV, 2.101 * eV, 2.119 * eV, 2.138 * eV,
    2.156 * eV, 2.175 * eV, 2.194 * eV, 2.214 * eV, 2.234 * eV,
    2.254 * eV, 2.275 * eV, 2.296 * eV, 2.317 * eV, 2.339 * eV,
    2.362 * eV, 2.384 * eV, 2.407 * eV, 2.431 * eV, 2.455 * eV,
    2.480 * eV, 2.505 * eV, 2.530 * eV, 2.556 * eV, 2.583 * eV,
    2.610 * eV, 2.638 * eV, 2.666 * eV, 2.695 * eV, 2.725 * eV,
    2.755 * eV, 2.786 * eV, 2.818 * eV, 2.850 * eV, 2.883 * eV,
    2.917 * eV, 2.952 * eV, 2.987 * eV, 3.024 * eV, 3.061 * eV,
    3.100 * eV, 3.139 * eV, 3.179 * eV, 3.220 * eV, 3.263 * eV,
    3.306 * eV, 3.351 * eV, 3.397 * eV, 3.444 * eV, 3.492 * eV,
    3.542 * eV, 3.594 * eV, 3.646 * eV, 3.701 * eV, 3.757 * eV,
    3.815 * eV, 3.874 * eV, 3.936 * eV, 3.999 * eV, 4.065 * eV,
    4.133 * eV, 4.203 * eV, 4.275 * eV, 4.350 * eV, 4.428 * eV,
    4.508 * eV, 4.592 * eV, 4.678 * eV, 4.768 * eV, 4.862 * eV,
    4.959 * eV, 5.060 * eV, 5.166 * eV, 5.276 * eV, 5.390 * eV,
    5.510 * eV, 5.635 * eV, 5.767 * eV, 5.904 * eV, 6.048 * eV,
    6.199 * eV, 6.358 * eV, 6.525 * eV, 6.702 * eV, 6.888 * eV,
    7.085 * eV};

  static const std::vector<G4double> qe = {
    0.00000, 0.00000, 0.00000, 0.00042, 0.00057,
    0.00083, 0.00116, 0.00153, 0.00197, 0.00256,
    0.00336, 0.00441, 0.00568, 0.00716, 0.00886,
    0.01075, 0.01283, 0.01512, 0.01764, 0.02040,
    0.02338, 0.02658, 0.03001, 0.03370, 0.03770,
    0.04204, 0.04676, 0.05189, 0.05741, 0.06322,
    0.06923, 0.07533, 0.08144, 0.08763, 0.09380,
    0.09997, 0.10634, 0.11302, 0.12025, 0.12699,
    0.13401, 0.14094, 0.14800, 0.15489, 0.16190,
    0.16911, 0.17656, 0.18365, 0.19010, 0.19542,
    0.20040, 0.20541, 0.21102, 0.21692, 0.22310,
    0.22883, 0.23360, 0.23673, 0.23903, 0.24082,
    0.24276, 0.24408, 0.24363, 0.24240, 0.24074,
    0.23900, 0.23718, 0.23505, 0.23170, 0.22827,
    0.22414, 0.21974, 0.21498, 0.20970, 0.20491,
    0.20076, 0.19691, 0.19317, 0.18954, 0.18584,
    0.18180, 0.17704, 0.17156, 0.16541, 0.15937,
    0.15321, 0.14699, 0.14044, 0.13355, 0.12648,
    0.11913, 0.11153, 0.10384, 0.09636, 0.08906,
    0.08192, 0.07494, 0.06788, 0.06053, 0.05289,
    0.04577, 0.04003, 0.03408, 0.00000, 0.00000,
    0.00000};
}

namespace AmBeTagger
{
G4double PmtResponse::QuantumEfficiency(G4double photonEnergy) const
{
  if (photonEnergy < energy.front() || photonEnergy > energy.back()) {
    return 0.0;
  }

  std::vector<G4double>::const_iterator upper =
      std::upper_bound(energy.begin(), energy.end(), photonEnergy);

  const std::size_t i =
      static_cast<std::size_t>(std::distance(energy.begin(), upper) - 1);

  const G4double x0 = energy[i];
  const G4double x1 = energy[i + 1];
  const G4double y0 = qe[i];
  const G4double y1 = qe[i + 1];

  return y0 + (photonEnergy - x0) * (y1 - y0) / (x1 - x0);
}

G4bool PmtResponse::ProducesPhotoelectron(G4double photonEnergy) const
{
  return G4UniformRand() < QuantumEfficiency(photonEnergy);
}
}