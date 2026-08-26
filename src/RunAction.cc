#include "AmBeTagger/RunAction.hh"

#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include <cmath>

namespace AmBeTagger
{
void RunAction::BeginOfRunAction(const G4Run*)
{
  eventCount_ = 0;
  zeroDepositEventCount_ = 0;
  totalEnergyDeposit_ = 0.0;
  minEnergyDeposit_ = 0.0;
  maxEnergyDeposit_ = 0.0;
  totalEnergyDepositSquared_ = 0.0;
  totalScintillationPhotonCount_ = 0;
  totalCerenkovPhotonCount_ = 0;
  totalPmtPhotonCount_ = 0; 
  totalPhotoelectronCount_ = 0; 

}

void RunAction::EndOfRunAction(const G4Run*)
{
  const G4double meanEnergyDeposit =
      eventCount_ > 0 ? totalEnergyDeposit_ / eventCount_ : 0.0;

  const G4double meanSquareEnergyDeposit =
    eventCount_ > 0 ? totalEnergyDepositSquared_ / eventCount_ : 0.0;

  const G4double variance =
      meanSquareEnergyDeposit - meanEnergyDeposit * meanEnergyDeposit;

  const G4double rmsEnergyDeposit =
      variance > 0.0 ? std::sqrt(variance) : 0.0;

  G4cout << "Run BGO energy summary:" << G4endl;
  G4cout << "  events = " << eventCount_ << G4endl;
  G4cout << "  zero-deposit events = " << zeroDepositEventCount_ << G4endl;
  G4cout << "  total Edep = " << totalEnergyDeposit_ / MeV << " MeV" << G4endl;
  G4cout << "  mean Edep = " << meanEnergyDeposit / MeV << " MeV" << G4endl;
  G4cout << "  RMS Edep = " << rmsEnergyDeposit / MeV << " MeV" << G4endl;
  G4cout << "  min Edep = " << minEnergyDeposit_ / MeV << " MeV" << G4endl;
  G4cout << "  max Edep = " << maxEnergyDeposit_ / MeV << " MeV" << G4endl;

  G4cout << "--------------------------------------------------" << G4endl;

  const G4double scintillationPhotonsPerMeV =
    totalEnergyDeposit_ > 0.0
        ? totalScintillationPhotonCount_ / (totalEnergyDeposit_ / MeV)
        : 0.0;

  const G4double cerenkovPhotonsPerMeV = 
    totalEnergyDeposit_ > 0.0
      ? totalCerenkovPhotonCount_ / (totalEnergyDeposit_ / MeV): 0.0;

  G4cout << "Run optical photon summary:" << G4endl;
  G4cout << "  scintillation photons = "
        << totalScintillationPhotonCount_ << G4endl;
  G4cout << "  Cerenkov photons = "
        << totalCerenkovPhotonCount_ << G4endl;
  G4cout << "  scintillation photons / MeV deposited = "
        << scintillationPhotonsPerMeV << G4endl;
  G4cout << " Cerenkov photons / MeV deposited = "
        << cerenkovPhotonsPerMeV << G4endl;
  G4cout << "Run PMT photon summary: " << G4endl;
  G4cout << " PMT photons = "
         << totalPmtPhotonCount_ << G4endl;
  G4cout << "  photoelectrons = " << totalPhotoelectronCount_ << G4endl;


  const G4double effectiveDetectionFraction =
    totalPmtPhotonCount_ > 0
        ? static_cast<G4double>(totalPhotoelectronCount_) / totalPmtPhotonCount_
        : 0.0;

  G4cout << "  photoelectrons / PMT photon = "
        << effectiveDetectionFraction << G4endl;
}

void RunAction::AddEventEnergyDeposit(G4double energyDeposit)
{
  if (eventCount_ == 0) {
    minEnergyDeposit_ = energyDeposit;
    maxEnergyDeposit_ = energyDeposit;
  } else {
    if (energyDeposit < minEnergyDeposit_) {
      minEnergyDeposit_ = energyDeposit;
    }
    if (energyDeposit > maxEnergyDeposit_) {
      maxEnergyDeposit_ = energyDeposit;
    }
  }

  if (energyDeposit == 0.0) {
    ++zeroDepositEventCount_;
  }

  totalEnergyDeposit_ += energyDeposit;
  totalEnergyDepositSquared_ += energyDeposit * energyDeposit;
  ++eventCount_;
}

void RunAction::AddEventPhotonCounts(G4int scintillationPhotons,
                                     G4int cerenkovPhotons,
                                     G4int pmtPhotons,
                                     G4int photoelectrons)
{
  totalScintillationPhotonCount_ += scintillationPhotons;
  totalCerenkovPhotonCount_ += cerenkovPhotons;
  totalPmtPhotonCount_ += pmtPhotons;
  totalPhotoelectronCount_ += photoelectrons; 
}


}