#include "AmBeTagger/WaveformBuilder.hh"

#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <cstddef>
#include <cmath>
#include <iostream>
#include <vector>

namespace
{
constexpr double kTauRiseNs = 1.5;
constexpr double kTauFallNs = 5.0;
constexpr double kTransitTimeNs = 23.0;
constexpr G4double kBinWidth = 2.0 * ns;
constexpr G4int kSampleCount = 1500;
constexpr G4double kNoiseSigma = 0.001;
constexpr G4double kGainMean = 0.005;
constexpr G4double kGainSigma = 0.0006;

bool CloseEnough(double actual, double expected, double tolerance)
{
  return std::abs(actual - expected) < tolerance;
}

double ExpectedPulseAmplitude(double sampleTimeNs, double peTimeNs)
{
  const double pulseTimeNs = sampleTimeNs - peTimeNs - kTransitTimeNs;
  if (pulseTimeNs < 0.0) {
    return 0.0;
  }

  return (1.0 - std::exp(-pulseTimeNs / kTauRiseNs))
      * std::exp(-pulseTimeNs / kTauFallNs);
}
}

int main()
{
  const AmBeTagger::WaveformBuilder builder(0.0, 1.0, 0.0);

  if (!CloseEnough(builder.SampleSpacing(), kBinWidth, 1.0e-12)) {
    std::cerr << "Sample spacing failed: " << builder.SampleSpacing() / ns
              << " ns\n";
    return 1;
  }

  if (builder.SampleCount() != kSampleCount) {
    std::cerr << "Sample count failed: " << builder.SampleCount() << '\n';
    return 1;
  }

  const std::vector<G4double> zeroPeWaveform = builder.Build({});

  if (zeroPeWaveform.size() != static_cast<std::size_t>(kSampleCount)) {
    std::cerr << "Zero-PE waveform size failed: " << zeroPeWaveform.size()
              << '\n';
    return 1;
  }

  for (G4double sample : zeroPeWaveform) {
    if (!CloseEnough(sample, 0.0, 1.0e-12)) {
      std::cerr << "Noiseless zero-PE sample failed: " << sample << '\n';
      return 1;
    }
  }

  const std::vector<G4double> peTimes = {0.0 * ns};
  const std::vector<G4double> waveform = builder.Build(peTimes);

  if (waveform.size() != static_cast<std::size_t>(builder.SampleCount())) {
    std::cerr << "Expected " << builder.SampleCount()
              << " samples, got " << waveform.size() << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[0], 0.0, 1.0e-12)) {
    std::cerr << "Sample 0 failed: " << waveform[0] << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[1], 0.0, 1.0e-12)) {
    std::cerr << "Sample 1 failed: " << waveform[1] << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[2], 0.0, 1.0e-12)) {
    std::cerr << "Sample 2 failed: " << waveform[2] << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[11], 0.0, 1.0e-12)) {
    std::cerr << "Sample 11 failed: " << waveform[11] << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[12],
                   ExpectedPulseAmplitude(24.0, 0.0),
                   1.0e-12)) {
    std::cerr << "Sample 12 failed: " << waveform[12] << '\n';
    return 1;
  }

  const std::vector<G4double> delayedPeTimes = {20.0 * ns};
  const std::vector<G4double> delayedWaveform =
      builder.Build(delayedPeTimes);

  if (!CloseEnough(delayedWaveform[21], 0.0, 1.0e-12)) {
    std::cerr << "Delayed sample 21 failed: " << delayedWaveform[21] << '\n';
    return 1;
  }

  if (!CloseEnough(delayedWaveform[22],
                   ExpectedPulseAmplitude(44.0, 20.0),
                   1.0e-12)) {
    std::cerr << "Delayed sample 22 failed: " << delayedWaveform[22] << '\n';
    return 1;
  }

  const std::vector<G4double> simultaneousPeTimes = {0.0 * ns, 0.0 * ns};
  const std::vector<G4double> simultaneousPeWaveform =
      builder.Build(simultaneousPeTimes);

  const double expectedSimultaneousSample =
      2.0 * ExpectedPulseAmplitude(24.0, 0.0);

  if (!CloseEnough(simultaneousPeWaveform[12],
                   expectedSimultaneousSample,
                   1.0e-12)) {
    std::cerr << "Two simultaneous PE sample 12 failed: "
              << simultaneousPeWaveform[12] << '\n';
    return 1;
  }

  const std::vector<G4double> twoPeTimes = {0.0 * ns, 20.0 * ns};
  const std::vector<G4double> twoPeWaveform =
      builder.Build(twoPeTimes);

  const double expectedSample22 =
      ExpectedPulseAmplitude(44.0, 0.0)
      + ExpectedPulseAmplitude(44.0, 20.0);

  if (!CloseEnough(twoPeWaveform[22], expectedSample22, 1.0e-12)) {
    std::cerr << "Two separated PE sample 22 failed: " << twoPeWaveform[22]
              << '\n';
    return 1;
  }

  CLHEP::HepRandom::setTheSeed(12345);
  const AmBeTagger::WaveformBuilder noisyBuilder(kNoiseSigma);
  const std::vector<G4double> noiseOnly = noisyBuilder.Build({});

  double sum = 0.0;
  double sumSquares = 0.0;
  for (G4double sample : noiseOnly) {
    sum += sample;
    sumSquares += sample * sample;
  }

  const double mean = sum / noiseOnly.size();
  const double meanSquare = sumSquares / noiseOnly.size();
  const double rms = std::sqrt(meanSquare - mean * mean);

  if (std::abs(mean) > 0.00015) {
    std::cerr << "Noise mean failed: " << mean << '\n';
    return 1;
  }

  if (rms < 0.0008 || rms > 0.0012) {
    std::cerr << "Noise RMS failed: " << rms << '\n';
    return 1;
  }

  CLHEP::HepRandom::setTheSeed(67890);
  const AmBeTagger::WaveformBuilder gainBuilder(
      0.0, kGainMean, kGainSigma);
  const std::vector<G4double> gainWaveform =
      gainBuilder.Build({0.0 * ns});

  CLHEP::HepRandom::setTheSeed(67890);
  const std::vector<G4double> repeatedGainWaveform =
      gainBuilder.Build({0.0 * ns});

  if (!CloseEnough(gainWaveform[12], repeatedGainWaveform[12], 1.0e-12)) {
    std::cerr << "Seeded gain reproducibility failed: "
              << gainWaveform[12] << " vs " << repeatedGainWaveform[12]
              << '\n';
    return 1;
  }

  if (gainWaveform[12] <= 0.0) {
    std::cerr << "Smeared gain sample should be positive: "
              << gainWaveform[12] << '\n';
    return 1;
  }

  const double meanGainSample =
      kGainMean * ExpectedPulseAmplitude(24.0, 0.0);

  if (CloseEnough(gainWaveform[12], meanGainSample, 1.0e-12)) {
    std::cerr << "Gain sample was not smeared away from the mean-only value\n";
    return 1;
  }


  std::cout << "WaveformBuilder checks passed\n";
  return 0;
}
