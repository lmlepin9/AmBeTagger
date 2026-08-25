#pragma once

#include "G4UserEventAction.hh"
#include "globals.hh"

class G4Event;

namespace AmBeTagger
{
class EventAction final : public G4UserEventAction
{
 public:
  void BeginOfEventAction(const G4Event* event) override;
  void EndOfEventAction(const G4Event* event) override;

  void AddEnergyDeposit(G4double energyDeposit);

 private:
  G4double eventEnergyDeposit_ = 0.0;
};
}