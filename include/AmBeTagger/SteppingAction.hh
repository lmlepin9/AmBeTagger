#pragma once

#include "G4UserSteppingAction.hh"

class G4Step;

namespace AmBeTagger
{
class DetectorConstruction;
class EventAction;

class SteppingAction final : public G4UserSteppingAction
{
 public:
  SteppingAction(EventAction* eventAction,
                 const DetectorConstruction* detectorConstruction);

  void UserSteppingAction(const G4Step* step) override;

 private:
  EventAction* eventAction_;
  const DetectorConstruction* detectorConstruction_;
};
}