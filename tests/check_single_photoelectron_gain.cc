#include "AmBeTagger/SinglePhotoelectronGain.hh"

#include "Randomize.hh"

#include <cmath>
#include <iostream>

namespace
{
bool CloseEnough(double actual, double expected, double tolerance)
{
  return std::abs(actual - expected) < tolerance;
}
}

int main()
{
  const AmBeTagger::SinglePhotoelectronGain fixedGain(0.005, 0.0);

  if (!CloseEnough(fixedGain.Sample(), 0.005, 1.0e-12)) {
    std::cerr << "Fixed gain sample failed: " << fixedGain.Sample() << '\n';
    return 1;
  }

  if (!CloseEnough(fixedGain.Mean(), 0.005, 1.0e-12)) {
    std::cerr << "Gain mean accessor failed: " << fixedGain.Mean() << '\n';
    return 1;
  }

  if (!CloseEnough(fixedGain.Sigma(), 0.0, 1.0e-12)) {
    std::cerr << "Gain sigma accessor failed: " << fixedGain.Sigma() << '\n';
    return 1;
  }

  constexpr double gainMean = 0.005;
  constexpr double gainSigma = 0.0006;
  constexpr int sampleCount = 100000;

  CLHEP::HepRandom::setTheSeed(12345);
  const AmBeTagger::SinglePhotoelectronGain smearedGain(
      gainMean, gainSigma);

  double sum = 0.0;
  double sumSquares = 0.0;
  for (int i = 0; i < sampleCount; ++i) {
    const double sample = smearedGain.Sample();
    sum += sample;
    sumSquares += sample * sample;
  }

  const double mean = sum / sampleCount;
  const double meanSquare = sumSquares / sampleCount;
  const double rms = std::sqrt(meanSquare - mean * mean);

  if (std::abs(mean - gainMean) > 1.0e-5) {
    std::cerr << "Smeared gain mean failed: " << mean << '\n';
    return 1;
  }

  if (std::abs(rms - gainSigma) > 3.0e-6) {
    std::cerr << "Smeared gain RMS failed: " << rms << '\n';
    return 1;
  }

  CLHEP::HepRandom::setTheSeed(67890);
  const double firstSample = smearedGain.Sample();

  CLHEP::HepRandom::setTheSeed(67890);
  const double repeatedSample = smearedGain.Sample();

  if (!CloseEnough(firstSample, repeatedSample, 1.0e-12)) {
    std::cerr << "Seeded gain reproducibility failed: " << firstSample
              << " vs " << repeatedSample << '\n';
    return 1;
  }

  std::cout << "SinglePhotoelectronGain checks passed\n";
  return 0;
}
