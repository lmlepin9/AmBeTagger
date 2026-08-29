#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TTree.h"

void analyze_events(const char* fileName = "output.root")
{
  TFile* inputFile = TFile::Open(fileName, "READ");
  if ((!inputFile || inputFile->IsZombie()) && std::string(fileName) == "output.root") {
    delete inputFile;
    fileName = "../output.root";
    inputFile = TFile::Open(fileName, "READ");
  }

  if (!inputFile || inputFile->IsZombie()) {
    std::cerr << "Could not open ROOT file: " << fileName << std::endl;
    return;
  }

  TTree* events = nullptr;
  inputFile->GetObject("Events", events);
  if (!events) {
    std::cerr << "Could not find TTree named 'Events' in " << fileName << std::endl;
    inputFile->Close();
    return;
  }

  Int_t eventID = 0;
  Double_t totalEdepBGO = 0.0;
  Int_t numPmtPhotons = 0;
  Int_t numCerenkovPhotons = 0;
  Int_t numScintillationPhotons = 0;
  Double_t earliestPETime = 0.0;
  std::vector<double>* pmtWaveform = nullptr;

  events->SetBranchAddress("eventID", &eventID);
  events->SetBranchAddress("totalEdepBGO", &totalEdepBGO);
  events->SetBranchAddress("numPmtPhotons", &numPmtPhotons);
  events->SetBranchAddress("numCerenkovPhotons", &numCerenkovPhotons);
  events->SetBranchAddress("numScintillationPhotons", &numScintillationPhotons);
  events->SetBranchAddress("earliestPETime", &earliestPETime);

  const bool hasPmtWaveform = events->GetBranch("pmtWaveform") != nullptr;
  if (hasPmtWaveform) {
    events->SetBranchAddress("pmtWaveform", &pmtWaveform);
  }

  constexpr double waveformSampleSpacing = 2.0;  // ns

  std::vector<double> selectedEdepBGO;
  std::vector<double> selectedEarliestPETime;
  std::vector<int> selectedNumPmtPhotons;
  std::vector<int> selectedNumCerenkovPhotons;
  std::vector<int> selectedNumScintillationPhotons;
  std::vector<double> selectedIntegratedCharge;
  std::vector<int> waveformEventIDs;
  std::vector<std::vector<double>> firstWaveforms;

  const Long64_t nEntries = events->GetEntries();
  for (Long64_t iEntry = 0; iEntry < nEntries; ++iEntry) {
    events->GetEntry(iEntry);
    if (totalEdepBGO == 0) {
      continue;
    }

    selectedEdepBGO.push_back(totalEdepBGO);
    selectedEarliestPETime.push_back(earliestPETime);
    selectedNumPmtPhotons.push_back(numPmtPhotons);
    selectedNumCerenkovPhotons.push_back(numCerenkovPhotons);
    selectedNumScintillationPhotons.push_back(numScintillationPhotons);
    if (hasPmtWaveform) {
      const double integratedCharge = pmtWaveform
          ? std::accumulate(pmtWaveform->begin(), pmtWaveform->end(), 0.0)
              * waveformSampleSpacing
          : 0.0;
      selectedIntegratedCharge.push_back(integratedCharge);
    }
    if (pmtWaveform && firstWaveforms.size() < 3) {
      waveformEventIDs.push_back(eventID);
      firstWaveforms.push_back(*pmtWaveform);
    }

    // Add event-level analysis here.
  }

  const Long64_t nEventsNonZero = selectedEdepBGO.size();
  std::cout << "Number of entries with non-zero Edep in the BGO: " << nEventsNonZero << std::endl;

  if (nEventsNonZero == 0) {
    std::cerr << "No events with non-zero BGO energy deposition were found." << std::endl;
    inputFile->Close();
    return;
  }

  auto makeUpperEdge = [](double maximum) {
    if (maximum <= 0.) {
      return 1.;
    }

    const double decade = std::pow(10., std::floor(std::log10(maximum)));
    return std::ceil(maximum / decade) * decade;
  };

  const double maxEdepBGO = *std::max_element(selectedEdepBGO.begin(), selectedEdepBGO.end());
  const auto earliestTimeRange =
      std::minmax_element(selectedEarliestPETime.begin(), selectedEarliestPETime.end());
  double earliestTimeMin = std::floor(*earliestTimeRange.first);
  double earliestTimeMax = std::ceil(*earliestTimeRange.second);
  if (earliestTimeMin == earliestTimeMax) {
    earliestTimeMax += 1.;
  }

  const int maxPhotonCount = std::max({
      *std::max_element(selectedNumPmtPhotons.begin(), selectedNumPmtPhotons.end()),
      *std::max_element(selectedNumCerenkovPhotons.begin(), selectedNumCerenkovPhotons.end()),
      *std::max_element(selectedNumScintillationPhotons.begin(), selectedNumScintillationPhotons.end()),
  });
  const double photonUpperEdge = makeUpperEdge(maxPhotonCount);

  TH1D* histEdepBGO = new TH1D("edep",
                               "BGO Energy Deposition;Energy [MeV];Number of entries",
                               100,
                               0.,
                               makeUpperEdge(maxEdepBGO));
  histEdepBGO->SetDirectory(nullptr);

  TH1D* histEarliestPETime =
      new TH1D("earliest_pe_time",
               "Earliest PE Time for Events with BGO Energy Deposition;Earliest PE time [ns];Number of entries",
               100,
               earliestTimeMin,
               earliestTimeMax);
  histEarliestPETime->SetDirectory(nullptr);

  TH1D* histNumPmtPhotons =
      new TH1D("num_pmt_photons",
               "Photon Counts for Events with BGO Energy Deposition;Number of photons;Number of entries",
               100,
               0.,
               photonUpperEdge);
  histNumPmtPhotons->SetDirectory(nullptr);
  histNumPmtPhotons->SetLineColor(kBlack);
  histNumPmtPhotons->SetLineWidth(2);

  TH1D* histNumCerenkovPhotons =
      new TH1D("num_cerenkov_photons",
               "Photon Counts for Events with BGO Energy Deposition;Number of photons;Number of entries",
               100,
               0.,
               photonUpperEdge);
  histNumCerenkovPhotons->SetDirectory(nullptr);
  histNumCerenkovPhotons->SetLineColor(kBlue + 1);
  histNumCerenkovPhotons->SetLineWidth(2);

  TH1D* histNumScintillationPhotons =
      new TH1D("num_scintillation_photons",
               "Photon Counts for Events with BGO Energy Deposition;Number of photons;Number of entries",
               100,
               0.,
               photonUpperEdge);
  histNumScintillationPhotons->SetDirectory(nullptr);
  histNumScintillationPhotons->SetLineColor(kRed + 1);
  histNumScintillationPhotons->SetLineWidth(2);

  TH1D* histIntegratedCharge = nullptr;
  if (!selectedIntegratedCharge.empty()) {
    const auto integratedChargeRange =
        std::minmax_element(selectedIntegratedCharge.begin(), selectedIntegratedCharge.end());
    double integratedChargeMin = *integratedChargeRange.first;
    double integratedChargeMax = *integratedChargeRange.second;

    if (integratedChargeMin == integratedChargeMax) {
      const double halfWidth = integratedChargeMin == 0.
          ? 0.5
          : 0.05 * std::abs(integratedChargeMin);
      integratedChargeMin -= halfWidth;
      integratedChargeMax += halfWidth;
    } else {
      const double padding = 0.05 * (integratedChargeMax - integratedChargeMin);
      integratedChargeMin -= padding;
      integratedChargeMax += padding;
    }

    histIntegratedCharge =
        new TH1D("integrated_charge",
                 "Integrated charge;Integrated charge [a.u.];Number of entries",
                 100,
                 integratedChargeMin,
                 integratedChargeMax);
    histIntegratedCharge->SetDirectory(nullptr);
  }

  for (std::size_t iEvent = 0; iEvent < selectedEdepBGO.size(); ++iEvent) {
    histEdepBGO->Fill(selectedEdepBGO[iEvent]);
    histEarliestPETime->Fill(selectedEarliestPETime[iEvent]);
    histNumPmtPhotons->Fill(selectedNumPmtPhotons[iEvent]);
    histNumCerenkovPhotons->Fill(selectedNumCerenkovPhotons[iEvent]);
    histNumScintillationPhotons->Fill(selectedNumScintillationPhotons[iEvent]);
    if (histIntegratedCharge) {
      histIntegratedCharge->Fill(selectedIntegratedCharge[iEvent]);
    }
  }

  TCanvas* c1 = new TCanvas("c1", "BGO energy deposition");
  c1->cd();
  histEdepBGO->Draw("HIST");
  c1->Update();

  TCanvas* c2 = new TCanvas("c2", "Earliest PE time");
  c2->cd();
  histEarliestPETime->Draw("HIST");
  c2->Update();

  TCanvas* c3 = new TCanvas("c3", "Photon counts");
  c3->cd();
  const double photonMaximum = std::max({
      histNumPmtPhotons->GetMaximum(),
      histNumCerenkovPhotons->GetMaximum(),
      histNumScintillationPhotons->GetMaximum(),
  });
  histNumPmtPhotons->SetMaximum(1.1 * photonMaximum);
  histNumPmtPhotons->Draw("HIST");
  histNumCerenkovPhotons->Draw("HIST SAME");
  histNumScintillationPhotons->Draw("HIST SAME");

  TLegend* photonLegend = new TLegend(0.58, 0.68, 0.88, 0.88);
  photonLegend->AddEntry(histNumPmtPhotons, "PMT photons", "l");
  photonLegend->AddEntry(histNumCerenkovPhotons, "Cerenkov photons", "l");
  photonLegend->AddEntry(histNumScintillationPhotons, "Scintillation photons", "l");
  photonLegend->Draw();
  c3->Update();

  if (histIntegratedCharge) {
    TCanvas* c4 = new TCanvas("c4", "Integrated charge");
    c4->cd();
    histIntegratedCharge->Draw("HIST");
    c4->Update();
  }

  if (!hasPmtWaveform) {
    std::cerr << "No pmtWaveform branch found; waveform figures and the integrated-charge "
                 "histogram were not created."
              << std::endl;
  }

  for (std::size_t iWaveform = 0; iWaveform < firstWaveforms.size(); ++iWaveform) {
    const std::vector<double>& waveform = firstWaveforms[iWaveform];
    if (waveform.empty()) {
      continue;
    }

    const std::string histName = "pmt_waveform_event_" + std::to_string(waveformEventIDs[iWaveform]);
    const std::string histTitle = "PMT Waveform for Event " + std::to_string(waveformEventIDs[iWaveform]) +
                                  ";Sample;Amplitude";
    TH1D* histWaveform =
        new TH1D(histName.c_str(), histTitle.c_str(), waveform.size(), 0., waveform.size());
    histWaveform->SetDirectory(nullptr);

    for (std::size_t iSample = 0; iSample < waveform.size(); ++iSample) {
      histWaveform->SetBinContent(iSample + 1, waveform[iSample]);
    }

    const std::string canvasName = "c_waveform_" + std::to_string(iWaveform + 1);
    const std::string canvasTitle = "PMT waveform event " + std::to_string(waveformEventIDs[iWaveform]);
    TCanvas* waveformCanvas = new TCanvas(canvasName.c_str(), canvasTitle.c_str());
    waveformCanvas->cd();
    histWaveform->Draw("HIST");
    waveformCanvas->Update();
  }

  inputFile->Close();
}
