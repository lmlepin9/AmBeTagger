#include "AmBeTagger/RunAction.hh"

#include "AmBeTagger/RootOutputWriter.hh"
#include "G4GenericMessenger.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include <cmath>

namespace AmBeTagger
{
RunAction::RunAction()
{
  outputMessenger_ = std::make_unique<G4GenericMessenger>(
      this, "/AmBeTagger/output/", "AmBeTagger ROOT output control");

  auto& rootFileCommand = outputMessenger_->DeclareProperty(
      "rootFile", outputFileName_,
      "Run-level ROOT output file. Leave unset to disable all file output.");
  rootFileCommand.SetStates(G4State_PreInit, G4State_Idle);

  auto& waveformCommand = outputMessenger_->DeclareProperty(
      "enableWaveform", waveformOutputEnabled_,
      "Store the PMT waveform branch when ROOT output is enabled.");
  waveformCommand.SetStates(G4State_PreInit, G4State_Idle);
}

RunAction::~RunAction() = default;

const G4String& RunAction::OutputFileName() const
{
  return outputFileName_;
}

G4bool RunAction::IsOutputEnabled() const
{
  return !outputFileName_.empty();
}

G4bool RunAction::IsWaveformOutputEnabled() const
{
  return IsOutputEnabled() && waveformOutputEnabled_;
}

void RunAction::RecordEvent(G4int eventID,
                            G4double totalEdepBGO,
                            G4int numPmtPhotons,
                            G4int numCerenkovPhotons,
                            G4int numScintillationPhotons,
                            G4int numPhotoelectrons,
                            G4double earliestPETime,
                            const std::vector<G4double>& pmtWaveform)
{
  if (!outputWriter_) {
    return;
  }

  outputWriter_->FillEvent(eventID,
                           totalEdepBGO,
                           numPmtPhotons,
                           numCerenkovPhotons,
                           numScintillationPhotons,
                           numPhotoelectrons,
                           earliestPETime,
                           pmtWaveform);
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  if (IsOutputEnabled()) {
    outputWriter_ =
        std::make_unique<RootOutputWriter>(waveformOutputEnabled_);

    if (!outputWriter_->Open(outputFileName_)) {
      G4cerr << "Unable to open ROOT output file: "
             << outputFileName_ << G4endl;
      outputWriter_.reset();
    }
  }

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
  earliestPhotoelectronTime_ = 0.0;
  totalPhotoelectronTime_ = 0.0;

}

void RunAction::EndOfRunAction(const G4Run*)
{
  if (outputWriter_) {
    outputWriter_->Close();
    outputWriter_.reset();
  }

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

  if (totalPhotoelectronCount_ > 0) {
  G4cout << "  earliest PE time = "
         << earliestPhotoelectronTime_ / ns << " ns" << G4endl;

  G4cout << "  mean PE time = "
         << (totalPhotoelectronTime_ / totalPhotoelectronCount_) / ns
         << " ns" << G4endl;
}
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

void RunAction::AddEventPhotoelectronTiming(G4int photoelectrons,
                                            G4double earliestTime,
                                            G4double timeSum)
{
  if (photoelectrons == 0) {
    return;
  }

  if (totalPhotoelectronCount_ == 0 ||
      earliestTime < earliestPhotoelectronTime_) {
    earliestPhotoelectronTime_ = earliestTime;
  }

  totalPhotoelectronTime_ += timeSum;
}


}
