#include "AmBeTagger/SteppingAction.hh"

#include "AmBeTagger/DetectorConstruction.hh"
#include "AmBeTagger/EventAction.hh"

#include "G4LogicalVolume.hh"
#include "G4Step.hh"
#include "G4VPhysicalVolume.hh"

#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4TrackStatus.hh"
#include "G4VProcess.hh"

#include "G4SystemOfUnits.hh"
#include "Randomize.hh"


#include <algorithm>
#include <cstddef>
#include <vector>


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
  G4LogicalVolume* pmtPlaneVolume = detectorConstruction_->GetPmtPlaneVolume();
  
  G4Track* track = step->GetTrack();

  G4VPhysicalVolume* prePhysicalVolume =
      step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();

  G4VPhysicalVolume* postPhysicalVolume = 
      step->GetPostStepPoint()->GetTouchableHandle()->GetVolume();

  if (track->GetDefinition() == G4OpticalPhoton::Definition() &&
    postPhysicalVolume != nullptr &&
    postPhysicalVolume->GetLogicalVolume() == pmtPlaneVolume) {
  eventAction_->AddPmtPhoton();

  if (pmtResponse_.ProducesPhotoelectron(track->GetKineticEnergy())) {
  eventAction_->AddPhotoelectron(track->GetGlobalTime());
  }

  track->SetTrackStatus(fStopAndKill);
  return;
  }


  if (prePhysicalVolume->GetLogicalVolume() != scoringVolume) {
    return;
  }


  if (track->GetDefinition() != G4OpticalPhoton::Definition()) {
  eventAction_->AddEnergyDeposit(step->GetTotalEnergyDeposit());
}



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