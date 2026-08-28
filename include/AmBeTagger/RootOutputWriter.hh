#pragma once

#include "globals.hh"

#include <vector>

namespace AmBeTagger
{
class RootOutputWriter final
{
 public:
  explicit RootOutputWriter(G4bool waveformOutputEnabled);
  ~RootOutputWriter();

  RootOutputWriter(const RootOutputWriter&) = delete;
  RootOutputWriter& operator=(const RootOutputWriter&) = delete;

  G4bool Open(const G4String& fileName);
  void FillEvent(G4int eventID,
                 G4double totalEdepBGO,
                 G4int numPmtPhotons,
                 G4int numCerenkovPhotons,
                 G4int numScintillationPhotons,
                 G4int numPhotoelectrons,
                 G4double earliestPETime,
                 const std::vector<G4double>& pmtWaveform);
  void Close();

 private:
  void CreateNtuple();

  G4bool waveformOutputEnabled_ = true;
  G4bool ntupleCreated_ = false;
  G4bool fileOpen_ = false;

  G4int eventID_ = 0;
  G4double totalEdepBGO_ = 0.0;
  G4int numPmtPhotons_ = 0;
  G4int numCerenkovPhotons_ = 0;
  G4int numScintillationPhotons_ = 0;
  G4double earliestPETime_ = -1.0;
  std::vector<G4double> pmtWaveform_;
};
}
