#pragma once

#include "globals.hh"

#include <vector>

namespace AmBeTagger
{
struct WaveformSummary
{
  G4double peakAmplitude = 0.0;
  G4int peakSample = 0;
  G4double peakTime = 0.0;
  G4double integral = 0.0;
};

class WaveformObservables final
{
 public:
  WaveformSummary Analyze(const std::vector<G4double>& waveform,
                          G4double sampleSpacing) const;
};
}
