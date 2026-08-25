#include "AmBeTagger/EventAction.hh"
#include "AmBeTagger/SteppingAction.hh"
#include "AmBeTagger/ActionInitialization.hh"
#include "AmBeTagger/PrimaryGeneratorAction.hh"



namespace AmBeTagger
{
ActionInitialization::ActionInitialization(
    const DetectorConstruction* detectorConstruction)
    : detectorConstruction_(detectorConstruction)
{
}

void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction);

  EventAction* eventAction = new EventAction;
  SetUserAction(eventAction);

  SetUserAction(new SteppingAction(eventAction, detectorConstruction_));
}
}