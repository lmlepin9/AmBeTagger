#pragma once

#include "G4UserEventAction.hh"
#include "globals.hh"

class G4Event;


namespace AmBeTagger
{
class RunAction;
class EventAction final : public G4UserEventAction
{
 public:
  explicit EventAction(RunAction* runAction);
  void BeginOfEventAction(const G4Event* event) override;
  void EndOfEventAction(const G4Event* event) override;
  void AddEnergyDeposit(G4double energyDeposit);

 private:
  RunAction* runAction_;
  G4double eventEnergyDeposit_ = 0.0;
};
}