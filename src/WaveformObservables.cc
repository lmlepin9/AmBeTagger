#include "AmBeTagger/WaveformObservables.hh"

#include <cstddef>

namespace AmBeTagger
{
WaveformSummary WaveformObservables::Analyze(
    const std::vector<G4double>& waveform,
    G4double sampleSpacing) const
{
  WaveformSummary summary;

  if (waveform.empty()) {
    return summary;
  }

  summary.peakAmplitude = waveform[0];

  for (std::size_t i = 0; i < waveform.size(); ++i) {
    const G4double sample = waveform[i];
    summary.integral += sample * sampleSpacing;

    if (sample > summary.peakAmplitude) {
      summary.peakAmplitude = sample;
      summary.peakSample = static_cast<G4int>(i);
    }
  }

  summary.peakTime = summary.peakSample * sampleSpacing;
  return summary;
}
}
