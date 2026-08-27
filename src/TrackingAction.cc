#include "AmBeTagger/TrackingAction.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"

#include <algorithm>

namespace
{
G4int CurrentEventId()
{
  const auto* event =
      G4EventManager::GetEventManager()->GetConstCurrentEvent();
  return event == nullptr ? -1 : event->GetEventID();
}
}

namespace AmBeTagger
{
TrackingAction::TrackingAction(G4int maxVisibleOpticalPhotons)
    : maxVisibleOpticalPhotons_(std::max(0, maxVisibleOpticalPhotons))
{
}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
  suppressedThisTrack_ = false;

  const G4int eventId = CurrentEventId();
  if (eventId != currentEventId_) {
    currentEventId_ = eventId;
    visibleOpticalPhotonsThisEvent_ = 0;
  }

  if (track->GetDefinition() != G4OpticalPhoton::Definition()) {
    return;
  }

  if (visibleOpticalPhotonsThisEvent_ < maxVisibleOpticalPhotons_) {
    ++visibleOpticalPhotonsThisEvent_;
    return;
  }

  storedTrajectorySettingBeforeSuppression_ =
      fpTrackingManager->GetStoreTrajectory();
  if (storedTrajectorySettingBeforeSuppression_ != 0) {
    fpTrackingManager->SetStoreTrajectory(0);
    suppressedThisTrack_ = true;
  }
}

void TrackingAction::PostUserTrackingAction(const G4Track*)
{
  if (suppressedThisTrack_) {
    fpTrackingManager->SetStoreTrajectory(
        storedTrajectorySettingBeforeSuppression_);
    suppressedThisTrack_ = false;
  }
}
}
