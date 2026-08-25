#include "AmBeTagger/SteppingAction.hh"

#include "AmBeTagger/DetectorConstruction.hh"
#include "AmBeTagger/EventAction.hh"

#include "G4LogicalVolume.hh"
#include "G4Step.hh"
#include "G4VPhysicalVolume.hh"

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
}
}