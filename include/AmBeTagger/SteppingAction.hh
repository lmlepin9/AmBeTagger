#pragma once

#include "G4UserSteppingAction.hh"
#include "AmBeTagger/PmtResponse.hh"

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
  PmtResponse* pmtResponse_;
  const DetectorConstruction* detectorConstruction_;
};
}