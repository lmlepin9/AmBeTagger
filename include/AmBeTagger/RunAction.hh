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

 private:
  G4int eventCount_ = 0;
  G4int zeroDepositEventCount_ = 0;
  G4double totalEnergyDeposit_ = 0.0;
  G4double minEnergyDeposit_ = 0.0;
  G4double maxEnergyDeposit_ = 0.0;
  G4double totalEnergyDepositSquared_ = 0.0;
};
}