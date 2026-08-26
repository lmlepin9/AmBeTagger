#include "AmBeTagger/RunAction.hh"
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

  RunAction* runAction = new RunAction;
  SetUserAction(runAction);

  EventAction* eventAction = new EventAction(runAction);
  SetUserAction(eventAction);

  SetUserAction(new SteppingAction(eventAction, detectorConstruction_));
}
}