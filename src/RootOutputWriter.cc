#include "AmBeTagger/RootOutputWriter.hh"

#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

namespace AmBeTagger
{
RootOutputWriter::RootOutputWriter(G4bool waveformOutputEnabled)
    : waveformOutputEnabled_(waveformOutputEnabled)
{
}

RootOutputWriter::~RootOutputWriter()
{
  Close();
}

G4bool RootOutputWriter::Open(const G4String& fileName)
{
  auto* analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetDefaultFileType("root");

  CreateNtuple();

  fileOpen_ = analysisManager->OpenFile(fileName);
  return fileOpen_;
}

void RootOutputWriter::FillEvent(
    G4int eventID,
    G4double totalEdepBGO,
    G4int numPmtPhotons,
    G4int numCerenkovPhotons,
    G4int numScintillationPhotons,
    G4int numPhotoelectrons,
    G4double earliestPETime,
    const std::vector<G4double>& pmtWaveform)
{
  if (!fileOpen_) {
    return;
  }

  eventID_ = eventID;
  totalEdepBGO_ = totalEdepBGO / MeV;
  numPmtPhotons_ = numPmtPhotons;
  numCerenkovPhotons_ = numCerenkovPhotons;
  numScintillationPhotons_ = numScintillationPhotons;
  earliestPETime_ = numPhotoelectrons > 0 ? earliestPETime / ns : -1.0;

  if (waveformOutputEnabled_) {
    pmtWaveform_ = pmtWaveform;
  }

  auto* analysisManager = G4AnalysisManager::Instance();
  analysisManager->FillNtupleIColumn(0, eventID_);
  analysisManager->FillNtupleDColumn(1, totalEdepBGO_);
  analysisManager->FillNtupleIColumn(2, numPmtPhotons_);
  analysisManager->FillNtupleIColumn(3, numCerenkovPhotons_);
  analysisManager->FillNtupleIColumn(4, numScintillationPhotons_);
  analysisManager->FillNtupleDColumn(5, earliestPETime_);
  analysisManager->AddNtupleRow();
}

void RootOutputWriter::Close()
{
  if (!fileOpen_) {
    return;
  }

  auto* analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();
  fileOpen_ = false;
}

void RootOutputWriter::CreateNtuple()
{
  if (ntupleCreated_) {
    return;
  }

  auto* analysisManager = G4AnalysisManager::Instance();
  analysisManager->CreateNtuple("Events", "AmBeTagger event outputs");
  analysisManager->CreateNtupleIColumn("eventID");
  analysisManager->CreateNtupleDColumn("totalEdepBGO");
  analysisManager->CreateNtupleIColumn("numPmtPhotons");
  analysisManager->CreateNtupleIColumn("numCerenkovPhotons");
  analysisManager->CreateNtupleIColumn("numScintillationPhotons");
  analysisManager->CreateNtupleDColumn("earliestPETime");

  if (waveformOutputEnabled_) {
    analysisManager->CreateNtupleDColumn("pmtWaveform", pmtWaveform_);
  }

  analysisManager->FinishNtuple();
  ntupleCreated_ = true;
}
}
