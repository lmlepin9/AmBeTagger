#include "AmBeTagger/EventAction.hh"

#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

namespace AmBeTagger
{
void EventAction::BeginOfEventAction(const G4Event*)
{
  eventEnergyDeposit_ = 0.0;
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  G4cout << "Event " << event->GetEventID()
         << " test-volume energy deposit = "
         << eventEnergyDeposit_ / MeV << " MeV" << G4endl;
}

void EventAction::AddEnergyDeposit(G4double energyDeposit)
{
  eventEnergyDeposit_ += energyDeposit;
}
}