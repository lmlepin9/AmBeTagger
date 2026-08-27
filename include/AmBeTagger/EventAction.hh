#pragma once

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <vector> 

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
  void AddScintillationPhoton();
  void AddCherenkovPhoton();
  void AddPmtPhoton(); 
  void AddPhotoelectron(G4double time);

 private:
  RunAction* runAction_;
  G4double eventEnergyDeposit_ = 0.0;
  G4int scintillationPhotonCount_ = 0;
  G4int cherenkovPhotonCount_ = 0; 
  G4int pmtPhotonCount_ = 0; 
  G4int photoelectronCount_ = 0; 
  G4double earliestPhotoelectronTime_ = 0.0;
  G4double sumPhotoelectronTime_ = 0.0;
  std::vector<G4double> photoelectronTimes_;
};
}