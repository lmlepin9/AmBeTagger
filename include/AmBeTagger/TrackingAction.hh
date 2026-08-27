#pragma once

#include "G4UserTrackingAction.hh"
#include "globals.hh"

class G4Track;

namespace AmBeTagger
{
class TrackingAction final : public G4UserTrackingAction
{
 public:
  explicit TrackingAction(G4int maxVisibleOpticalPhotons = 100);

  void PreUserTrackingAction(const G4Track* track) override;
  void PostUserTrackingAction(const G4Track* track) override;

 private:
  G4int maxVisibleOpticalPhotons_;
  G4int visibleOpticalPhotonsThisEvent_ = 0;
  G4int currentEventId_ = -1;
  G4int storedTrajectorySettingBeforeSuppression_ = 0;
  G4bool suppressedThisTrack_ = false;
};
}
