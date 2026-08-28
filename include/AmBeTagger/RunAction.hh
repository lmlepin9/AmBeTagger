#pragma once

#include "G4UserRunAction.hh"
#include "globals.hh"

#include <memory>
#include <vector>

class G4Run;
class G4GenericMessenger;

namespace AmBeTagger
{
class RootOutputWriter;

class RunAction final : public G4UserRunAction
{
 public:
  RunAction();
  ~RunAction() override;

  void BeginOfRunAction(const G4Run* run) override;
  void EndOfRunAction(const G4Run* run) override;

  const G4String& OutputFileName() const;
  G4bool IsOutputEnabled() const;
  G4bool IsWaveformOutputEnabled() const;

  void RecordEvent(G4int eventID,
                   G4double totalEdepBGO,
                   G4int numPmtPhotons,
                   G4int numCerenkovPhotons,
                   G4int numScintillationPhotons,
                   G4int numPhotoelectrons,
                   G4double earliestPETime,
                   const std::vector<G4double>& pmtWaveform);

  void AddEventEnergyDeposit(G4double energyDeposit);
  void AddEventPhotonCounts(G4int scintillationPhotons, 
                            G4int cerenkovPhotons,
                            G4int pmtPhotons,
                            G4int photoelectrons);

  void AddEventPhotoelectronTiming(G4int photoelectrons,
                                   G4double earliestTime,
                                   G4double timSum);

 private:
  std::unique_ptr<G4GenericMessenger> outputMessenger_;
  std::unique_ptr<RootOutputWriter> outputWriter_;
  G4String outputFileName_;
  G4bool waveformOutputEnabled_ = true;

  G4int eventCount_ = 0;
  G4int zeroDepositEventCount_ = 0;
  G4double totalEnergyDeposit_ = 0.0;
  G4double minEnergyDeposit_ = 0.0;
  G4double maxEnergyDeposit_ = 0.0;
  G4double totalEnergyDepositSquared_ = 0.0;
  G4int totalScintillationPhotonCount_ = 0;
  G4int totalCerenkovPhotonCount_ = 0;
  G4int totalPmtPhotonCount_ = 0; 
  G4int totalPhotoelectronCount_ = 0; 
  G4double earliestPhotoelectronTime_ = 0.0;
  G4double totalPhotoelectronTime_ = 0.0; 
};
}
