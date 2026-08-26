#include "AmBeTagger/EventAction.hh"
#include "AmBeTagger/RunAction.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

namespace AmBeTagger
{

EventAction::EventAction(RunAction* runAction)
    : runAction_(runAction)
{
}

void EventAction::BeginOfEventAction(const G4Event*)
{
  eventEnergyDeposit_ = 0.0;
  scintillationPhotonCount_ = 0;
  cherenkovPhotonCount_ = 0;
  pmtPhotonCount_ = 0; 
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  if (event->GetEventID() < 10) {
  G4cout << "Event " << event->GetEventID()
         << " BGO energy deposit = "
         << eventEnergyDeposit_ / MeV << " MeV" << G4endl;

  G4cout << "Scintillation photons = " << scintillationPhotonCount_ << G4endl;
  G4cout << "Cherenkov photons = " << cherenkovPhotonCount_ << G4endl; 
  G4cout << "PMT plane photon = " << pmtPhotonCount_ << G4endl; 
}

     
  runAction_->AddEventEnergyDeposit(eventEnergyDeposit_);
  runAction_->AddEventPhotonCounts(scintillationPhotonCount_, cherenkovPhotonCount_, pmtPhotonCount_);
}

void EventAction::AddEnergyDeposit(G4double energyDeposit)
{
  eventEnergyDeposit_ += energyDeposit;
}

void EventAction::AddScintillationPhoton()
{
  ++scintillationPhotonCount_;
}

void EventAction::AddCherenkovPhoton()
{
  ++cherenkovPhotonCount_;
}

void EventAction::AddPmtPhoton()
{
  ++pmtPhotonCount_;
}




}