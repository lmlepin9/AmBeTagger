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
constexpr G4double kTriggerTime = 700.0 * ns;
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

bool WaveformsExactlyEqual(const std::vector<G4double>& first,
                           const std::vector<G4double>& second)
{
  if (first.size() != second.size()) {
    return false;
  }

  for (std::size_t i = 0; i < first.size(); ++i) {
    if (first[i] != second[i]) {
      return false;
    }
  }

  return true;
}

bool AnySampleDiffers(const std::vector<G4double>& first,
                      const std::vector<G4double>& second)
{
  if (first.size() != second.size()) {
    return true;
  }

  for (std::size_t i = 0; i < first.size(); ++i) {
    if (first[i] != second[i]) {
      return true;
    }
  }

  return false;
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

  if (!CloseEnough(builder.TriggerTime(), kTriggerTime, 1.0e-12)) {
    std::cerr << "Trigger time failed: " << builder.TriggerTime() / ns
              << " ns\n";
    return 1;
  }

  if (!CloseEnough(builder.StartTime(), -kTriggerTime, 1.0e-12)) {
    std::cerr << "Start time failed: " << builder.StartTime() / ns
              << " ns\n";
    return 1;
  }

  if (!CloseEnough(builder.SampleTime(0), -700.0 * ns, 1.0e-12)) {
    std::cerr << "Sample 0 time failed: " << builder.SampleTime(0) / ns
              << " ns\n";
    return 1;
  }

  if (!CloseEnough(builder.SampleTime(12), -676.0 * ns, 1.0e-12)) {
    std::cerr << "Sample 12 time failed: " << builder.SampleTime(12) / ns
              << " ns\n";
    return 1;
  }

  if (!CloseEnough(builder.SampleTime(350), 0.0 * ns, 1.0e-12)) {
    std::cerr << "Trigger sample time failed: "
              << builder.SampleTime(350) / ns << " ns\n";
    return 1;
  }

  if (!CloseEnough(builder.EndTime(), 2298.0 * ns, 1.0e-12)) {
    std::cerr << "End time failed: " << builder.EndTime() / ns
              << " ns\n";
    return 1;
  }

  if (!CloseEnough(builder.Duration(), 3000.0 * ns, 1.0e-12)) {
    std::cerr << "Duration failed: " << builder.Duration() / ns
              << " ns\n";
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

  if (!CloseEnough(waveform[350], 0.0, 1.0e-12)) {
    std::cerr << "Trigger sample failed: " << waveform[350] << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[351], 0.0, 1.0e-12)) {
    std::cerr << "Sample 351 failed: " << waveform[351] << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[352], 0.0, 1.0e-12)) {
    std::cerr << "Sample 352 failed: " << waveform[352] << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[361], 0.0, 1.0e-12)) {
    std::cerr << "Sample 361 failed: " << waveform[361] << '\n';
    return 1;
  }

  if (!CloseEnough(waveform[362],
                   ExpectedPulseAmplitude(24.0, 0.0),
                   1.0e-12)) {
    std::cerr << "Sample 362 failed: " << waveform[362] << '\n';
    return 1;
  }

  const std::vector<G4double> delayedPeTimes = {20.0 * ns};
  const std::vector<G4double> delayedWaveform =
      builder.Build(delayedPeTimes);

  if (!CloseEnough(delayedWaveform[371], 0.0, 1.0e-12)) {
    std::cerr << "Delayed sample 371 failed: " << delayedWaveform[371]
              << '\n';
    return 1;
  }

  if (!CloseEnough(delayedWaveform[372],
                   ExpectedPulseAmplitude(44.0, 20.0),
                   1.0e-12)) {
    std::cerr << "Delayed sample 372 failed: " << delayedWaveform[372]
              << '\n';
    return 1;
  }

  const std::vector<G4double> simultaneousPeTimes = {0.0 * ns, 0.0 * ns};
  const std::vector<G4double> simultaneousPeWaveform =
      builder.Build(simultaneousPeTimes);

  const double expectedSimultaneousSample =
      2.0 * ExpectedPulseAmplitude(24.0, 0.0);

  if (!CloseEnough(simultaneousPeWaveform[362],
                   expectedSimultaneousSample,
                   1.0e-12)) {
    std::cerr << "Two simultaneous PE sample 362 failed: "
              << simultaneousPeWaveform[362] << '\n';
    return 1;
  }

  const std::vector<G4double> twoPeTimes = {0.0 * ns, 20.0 * ns};
  const std::vector<G4double> twoPeWaveform =
      builder.Build(twoPeTimes);

  const double expectedSample22 =
      ExpectedPulseAmplitude(44.0, 0.0)
      + ExpectedPulseAmplitude(44.0, 20.0);

  if (!CloseEnough(twoPeWaveform[372], expectedSample22, 1.0e-12)) {
    std::cerr << "Two separated PE sample 372 failed: " << twoPeWaveform[372]
              << '\n';
    return 1;
  }

  const std::vector<G4double> earlyPeWaveform =
      builder.Build({-724.0 * ns});

  if (!CloseEnough(earlyPeWaveform[0],
                   ExpectedPulseAmplitude(-700.0, -724.0),
                   1.0e-12)) {
    std::cerr << "Early PE sample 0 failed: " << earlyPeWaveform[0] << '\n';
    return 1;
  }

  if (earlyPeWaveform[0] <= 0.0) {
    std::cerr << "Early PE should leave a visible in-window tail\n";
    return 1;
  }

  const std::vector<G4double> latePeWaveform =
      builder.Build({2300.0 * ns});

  for (G4double sample : latePeWaveform) {
    if (!CloseEnough(sample, 0.0, 1.0e-12)) {
      std::cerr << "Late out-of-window PE sample failed: " << sample << '\n';
      return 1;
    }
  }

  const std::vector<G4double> endPeWaveform =
      builder.Build({2274.0 * ns});

  if (!CloseEnough(endPeWaveform[1498], 0.0, 1.0e-12)) {
    std::cerr << "End PE sample 1498 failed: " << endPeWaveform[1498]
              << '\n';
    return 1;
  }

  if (!CloseEnough(endPeWaveform[1499],
                   ExpectedPulseAmplitude(builder.EndTime() / ns, 2274.0),
                   1.0e-12)) {
    std::cerr << "End PE sample 1499 failed: " << endPeWaveform[1499]
              << '\n';
    return 1;
  }

  if (endPeWaveform[1499] <= 0.0) {
    std::cerr << "End PE should contribute to the final sample\n";
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

  if (!CloseEnough(gainWaveform[362], repeatedGainWaveform[362], 1.0e-12)) {
    std::cerr << "Seeded gain reproducibility failed: "
              << gainWaveform[362] << " vs " << repeatedGainWaveform[362]
              << '\n';
    return 1;
  }

  if (gainWaveform[362] <= 0.0) {
    std::cerr << "Smeared gain sample should be positive: "
              << gainWaveform[362] << '\n';
    return 1;
  }

  const double meanGainSample =
      kGainMean * ExpectedPulseAmplitude(24.0, 0.0);

  if (CloseEnough(gainWaveform[362], meanGainSample, 1.0e-12)) {
    std::cerr << "Gain sample was not smeared away from the mean-only value\n";
    return 1;
  }

  const AmBeTagger::WaveformBuilder stochasticBuilder(
      kNoiseSigma, kGainMean, kGainSigma);
  const std::vector<G4double> stochasticPeTimes = {
      0.0 * ns,
      20.0 * ns};

  CLHEP::HepRandom::setTheSeed(24680);
  const std::vector<G4double> firstStochasticWaveform =
      stochasticBuilder.Build(stochasticPeTimes);

  CLHEP::HepRandom::setTheSeed(24680);
  const std::vector<G4double> repeatedStochasticWaveform =
      stochasticBuilder.Build(stochasticPeTimes);

  if (!WaveformsExactlyEqual(firstStochasticWaveform,
                             repeatedStochasticWaveform)) {
    std::cerr << "Full waveform reproducibility failed\n";
    return 1;
  }

  CLHEP::HepRandom::setTheSeed(13579);
  const std::vector<G4double> differentSeedWaveform =
      stochasticBuilder.Build(stochasticPeTimes);

  if (!AnySampleDiffers(firstStochasticWaveform, differentSeedWaveform)) {
    std::cerr << "Different seed did not change stochastic waveform\n";
    return 1;
  }

  CLHEP::HepRandom::setTheSeed(11223);
  const std::vector<G4double> noisyZeroPeWaveform =
      stochasticBuilder.Build({});

  if (!AnySampleDiffers(noisyZeroPeWaveform, zeroPeWaveform)) {
    std::cerr << "Noisy zero-PE waveform stayed all zero\n";
    return 1;
  }

  if (!AnySampleDiffers(firstStochasticWaveform, waveform)) {
    std::cerr << "Stochastic PE waveform matched noiseless unit-gain waveform\n";
    return 1;
  }


  std::cout << "WaveformBuilder checks passed\n";
  return 0;
}
