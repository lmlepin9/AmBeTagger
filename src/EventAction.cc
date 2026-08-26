#include "AmBeTagger/EventAction.hh"
#include "AmBeTagger/RunAction.hh"
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
         << " BGO energy deposit = "
         << eventEnergyDeposit_ / MeV << " MeV" << G4endl;
         
  runAction_->AddEventEnergyDeposit(eventEnergyDeposit_);
}

void EventAction::AddEnergyDeposit(G4double energyDeposit)
{
  eventEnergyDeposit_ += energyDeposit;
}

EventAction::EventAction(RunAction* runAction)
    : runAction_(runAction)
{
}

}