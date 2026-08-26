#pragma once

#include "G4UserRunAction.hh"
#include "globals.hh"

class G4Run;

namespace AmBeTagger
{
class RunAction final : public G4UserRunAction
{
 public:
  void BeginOfRunAction(const G4Run* run) override;
  void EndOfRunAction(const G4Run* run) override;

  void AddEventEnergyDeposit(G4double energyDeposit);
  void AddEventPhotonCounts(G4int scintillationPhotons, 
                            G4int cerenkovPhotons,
                            G4int pmtPhotons);

 private:
  G4int eventCount_ = 0;
  G4int zeroDepositEventCount_ = 0;
  G4double totalEnergyDeposit_ = 0.0;
  G4double minEnergyDeposit_ = 0.0;
  G4double maxEnergyDeposit_ = 0.0;
  G4double totalEnergyDepositSquared_ = 0.0;
  G4int totalScintillationPhotonCount_ = 0;
  G4int totalCerenkovPhotonCount_ = 0;
  G4int totalPmtPhotonCount_ = 0; 
};
}