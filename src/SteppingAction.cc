#include "AmBeTagger/SteppingAction.hh"

#include "AmBeTagger/DetectorConstruction.hh"
#include "AmBeTagger/EventAction.hh"

#include "G4LogicalVolume.hh"
#include "G4Step.hh"
#include "G4VPhysicalVolume.hh"

#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"


namespace AmBeTagger
{
SteppingAction::SteppingAction(EventAction* eventAction,
                               const DetectorConstruction* detectorConstruction)
    : eventAction_(eventAction),
      detectorConstruction_(detectorConstruction)
{
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  G4LogicalVolume* scoringVolume = detectorConstruction_->GetScoringVolume();

  G4VPhysicalVolume* physicalVolume =
      step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();

  if (physicalVolume->GetLogicalVolume() != scoringVolume) {
    return;
  }

  // This is the total energy, regardless of the process... 
  eventAction_->AddEnergyDeposit(step->GetTotalEnergyDeposit());

  const std::vector<const G4Track*>* secondaries =
    step->GetSecondaryInCurrentStep();

  // Inspect secondaries produced in this step 
  for (const G4Track* secondary : *secondaries) {

    // Is it an optical photon? 
    if (secondary->GetDefinition() != G4OpticalPhoton::Definition()) {
      continue;
    }


    // Check if it is a scintillation photon 
    const G4VProcess* creator = secondary->GetCreatorProcess();
    if (creator != nullptr && creator->GetProcessName() == "Scintillation") {
      eventAction_->AddScintillationPhoton();
    }

    else if(creator != nullptr && creator->GetProcessName() == "Cerenkov"){
      eventAction_->AddCherenkovPhoton();
    }
  }
}
}