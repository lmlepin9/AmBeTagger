#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "WaveformCharge.hh"

#include "TCanvas.h"
#include "TFile.h"
#include "TGaxis.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TPad.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TTree.h"

namespace AmBeTagger
{
namespace Analysis
{
struct SampleSpec
{
  std::string fileName;
  std::string label;
  std::string identifier;
  int color = kBlack;
  int lineStyle = 1;
};

struct HistogramConfig
{
  int binCount;
  double minimum;
  double maximum;
};

struct AnalysisConfig
{
  ChargeIntegrationConfig charge;
  std::size_t exampleWaveformCount = 3;
  HistogramConfig bgoEnergyHistogram{80, 0.0, 10.0};
  HistogramConfig integratedChargeHistogram{80, 0.0, 15.0};
  std::string figureOutputDirectory = "analysis_figures";
};

struct SampleResult
{
  SampleSpec sample;
  std::string openedFileName;
  std::string error;
  std::int64_t totalEntries = 0;
  std::size_t selectedEventCount = 0;
  std::size_t invalidEnergyCount = 0;
  std::size_t unreadableEntryCount = 0;
  std::size_t emptyWaveformCount = 0;
  std::size_t incompleteWaveformCount = 0;
  std::size_t nonFiniteWaveformCount = 0;
  std::size_t invalidChargeConfigurationCount = 0;
  bool inputReadable = false;
  bool hasWaveformBranch = false;
  std::vector<double> bgoEnergyDepositions;
  std::vector<double> integratedCharges;
  std::vector<int> exampleWaveformEventIDs;
  std::vector<std::vector<double>> exampleWaveforms;
};

struct SampleHistograms
{
  std::size_t sampleIndex = 0;
  std::unique_ptr<TH1D> bgoEnergy;
  std::unique_ptr<TH1D> integratedCharge;
};

struct AnalysisOutput
{
  std::vector<SampleResult> samples;
  std::vector<SampleHistograms> histograms;
  std::vector<std::unique_ptr<TH1D>> exampleWaveformHistograms;
  std::unique_ptr<TLegend> bgoEnergyLegend;
  std::unique_ptr<TLegend> integratedChargeLegend;
  std::unique_ptr<TCanvas> bgoEnergyCanvas;
  std::unique_ptr<TCanvas> integratedChargeCanvas;
  std::vector<std::unique_ptr<TCanvas>> exampleWaveformCanvases;
  std::vector<std::string> exampleWaveformIdentifiers;
  std::vector<std::string> savedFigureFiles;
};

namespace
{
std::unique_ptr<AnalysisOutput> latestAnalysis;

void ConfigurePublicationStyle()
{
  gStyle->SetOptStat(0);
  gStyle->SetCanvasColor(kWhite);
  gStyle->SetPadColor(kWhite);
  gStyle->SetFrameFillColor(kWhite);
  gStyle->SetFrameLineWidth(2);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetTextFont(42);
  gStyle->SetLabelFont(42, "XYZ");
  gStyle->SetTitleFont(42, "XYZ");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFillColor(kWhite);
  gStyle->SetTitleFont(42, "");
  gStyle->SetTitleSize(0.05, "");
  TGaxis::SetMaxDigits(4);
}

void StyleHistogram(TH1D& histogram, bool startAtZero)
{
  histogram.SetStats(false);
  histogram.SetLineWidth(3);
  histogram.GetXaxis()->SetLabelFont(42);
  histogram.GetXaxis()->SetLabelSize(0.043);
  histogram.GetXaxis()->SetTitleFont(42);
  histogram.GetXaxis()->SetTitleSize(0.052);
  histogram.GetXaxis()->SetTitleOffset(1.08);
  histogram.GetXaxis()->SetTickLength(0.025);
  histogram.GetYaxis()->SetLabelFont(42);
  histogram.GetYaxis()->SetLabelSize(0.043);
  histogram.GetYaxis()->SetTitleFont(42);
  histogram.GetYaxis()->SetTitleSize(0.052);
  histogram.GetYaxis()->SetTitleOffset(1.25);
  histogram.GetYaxis()->SetTickLength(0.025);
  if (startAtZero) {
    histogram.SetMinimum(0.0);
  }
}

void StyleComparisonCanvas(TCanvas& canvas)
{
  canvas.SetFillColor(kWhite);
  canvas.SetBorderMode(0);
  canvas.SetLeftMargin(0.14);
  canvas.SetRightMargin(0.045);
  canvas.SetBottomMargin(0.13);
  canvas.SetTopMargin(0.05);
  canvas.SetTicks(1, 1);
}

std::string MakeIdentifier(const SampleSpec& sample)
{
  std::string identifier = sample.identifier;
  if (identifier.empty()) {
    identifier = sample.label.empty() ? sample.fileName : sample.label;
  }

  for (char& character : identifier) {
    const bool isAsciiLetter =
        (character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z');
    const bool isDigit = character >= '0' && character <= '9';
    if (!isAsciiLetter && !isDigit) {
      character = '_';
    }
  }

  return identifier.empty() ? "sample" : identifier;
}

std::unique_ptr<TFile> OpenInputFile(const std::string& fileName,
                                     std::string& openedFileName)
{
  auto inputFile = std::unique_ptr<TFile>(TFile::Open(fileName.c_str(), "READ"));
  if (inputFile && !inputFile->IsZombie()) {
    openedFileName = fileName;
    return inputFile;
  }

  inputFile.reset();
  if (fileName != "output.root") {
    return nullptr;
  }

  openedFileName = "../output.root";
  inputFile = std::unique_ptr<TFile>(TFile::Open(openedFileName.c_str(), "READ"));
  if (!inputFile || inputFile->IsZombie()) {
    inputFile.reset();
  }
  return inputFile;
}

bool IsValidHistogramConfig(const HistogramConfig& config)
{
  return config.binCount > 0 && std::isfinite(config.minimum) &&
      std::isfinite(config.maximum) && config.minimum < config.maximum;
}

bool NormalizeToUnitArea(TH1D& histogram)
{
  const double area = histogram.Integral(1, histogram.GetNbinsX(), "width");
  if (std::isfinite(area) && area > 0.0) {
    histogram.Scale(1.0 / area);
    return true;
  }
  return false;
}

std::unique_ptr<TH1D> MakeNormalizedHistogram(
    const std::string& name,
    const std::string& title,
    const HistogramConfig& config,
    const std::vector<double>& values,
    const SampleSpec& sample)
{
  if (values.empty()) {
    return nullptr;
  }

  auto histogram = std::make_unique<TH1D>(
      name.c_str(), title.c_str(), config.binCount, config.minimum, config.maximum);
  histogram->SetDirectory(nullptr);
  histogram->SetLineColor(sample.color);
  histogram->SetLineStyle(sample.lineStyle);
  StyleHistogram(*histogram, true);
  histogram->Sumw2();

  for (const double value : values) {
    histogram->Fill(value);
  }
  const double inRangeEntries =
      histogram->Integral(1, histogram->GetNbinsX());
  if (inRangeEntries < values.size()) {
    std::cout << "  " << sample.label << ": "
              << values.size() - static_cast<std::size_t>(inRangeEntries)
              << " entries fall outside the configured range ["
              << config.minimum << ", " << config.maximum << ")."
              << std::endl;
  }
  if (!NormalizeToUnitArea(*histogram)) {
    std::cerr << "No entries for sample '" << sample.label
              << "' fall inside the configured histogram range." << std::endl;
    return nullptr;
  }
  return histogram;
}

std::unique_ptr<TLegend> DrawComparisonHistogram(
    TCanvas& canvas,
    const std::vector<SampleResult>& samples,
    const std::vector<SampleHistograms>& histograms,
    bool drawCharge)
{
  double maximum = 0.0;
  for (const SampleHistograms& sampleHistograms : histograms) {
    const TH1D* histogram = drawCharge
        ? sampleHistograms.integratedCharge.get()
        : sampleHistograms.bgoEnergy.get();
    if (histogram) {
      maximum = std::max(maximum, histogram->GetMaximum());
    }
  }

  canvas.cd();
  auto legend = std::make_unique<TLegend>(0.62, 0.68, 0.94, 0.93);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  legend->SetTextFont(42);
  legend->SetTextSize(0.042);
  bool drewHistogram = false;
  for (const SampleHistograms& sampleHistograms : histograms) {
    TH1D* histogram = drawCharge
        ? sampleHistograms.integratedCharge.get()
        : sampleHistograms.bgoEnergy.get();
    if (!histogram) {
      continue;
    }

    if (!drewHistogram) {
      histogram->SetMaximum(maximum > 0.0 ? 1.1 * maximum : 1.0);
    }
    histogram->DrawCopy(drewHistogram ? "HIST SAME" : "HIST");
    legend->AddEntry(histogram,
                     samples[sampleHistograms.sampleIndex].sample.label.c_str(),
                     "l");
    drewHistogram = true;
  }

  if (drewHistogram) {
    legend->Draw();
    canvas.Update();
    return legend;
  }
  return nullptr;
}

void DrawExampleWaveforms(AnalysisOutput& output)
{
  for (std::size_t iSample = 0; iSample < output.samples.size(); ++iSample) {
    const SampleResult& sample = output.samples[iSample];
    if (sample.exampleWaveforms.empty()) {
      continue;
    }

    const std::string identifier = MakeIdentifier(sample.sample) +
        "_" + std::to_string(iSample);
    const std::string canvasName = "c_example_waveforms_" + identifier;
    const std::string canvasTitle = "Example waveforms: " + sample.sample.label;
    auto canvas =
        std::make_unique<TCanvas>(canvasName.c_str(), canvasTitle.c_str(), 1500, 460);
    canvas->SetFillColor(kWhite);
    canvas->SetBorderMode(0);
    canvas->Divide(static_cast<int>(sample.exampleWaveforms.size()), 1);

    for (std::size_t iWaveform = 0;
         iWaveform < sample.exampleWaveforms.size();
         ++iWaveform) {
      const std::vector<double>& waveform = sample.exampleWaveforms[iWaveform];
      const int eventID = sample.exampleWaveformEventIDs[iWaveform];
      const std::string histogramName = "example_waveform_" + identifier +
          "_" + std::to_string(iWaveform);
      const std::string histogramTitle = "Example waveform, event " +
          std::to_string(eventID) + ";Sample index;Amplitude [a.u.]";
      auto histogram = std::make_unique<TH1D>(histogramName.c_str(),
                                               histogramTitle.c_str(),
                                               waveform.size(),
                                               0.0,
                                               waveform.size());
      histogram->SetDirectory(nullptr);
      histogram->SetLineColor(sample.sample.color);
      StyleHistogram(*histogram, false);
      histogram->SetLineWidth(1);
      histogram->GetXaxis()->SetLabelSize(0.05);
      histogram->GetXaxis()->SetTitleSize(0.055);
      histogram->GetYaxis()->SetLabelSize(0.05);
      histogram->GetYaxis()->SetTitleSize(0.055);
      histogram->GetYaxis()->SetTitleOffset(1.35);
      for (std::size_t iSampleValue = 0;
           iSampleValue < waveform.size();
           ++iSampleValue) {
        histogram->SetBinContent(iSampleValue + 1, waveform[iSampleValue]);
      }

      TPad* pad = static_cast<TPad*>(canvas->cd(static_cast<int>(iWaveform + 1)));
      pad->SetFillColor(kWhite);
      pad->SetBorderMode(0);
      pad->SetLeftMargin(0.16);
      pad->SetRightMargin(0.035);
      pad->SetBottomMargin(0.15);
      pad->SetTopMargin(0.12);
      pad->SetTicks(1, 1);
      histogram->DrawCopy("HIST");
      output.exampleWaveformHistograms.push_back(std::move(histogram));
    }

    canvas->Update();
    output.exampleWaveformCanvases.push_back(std::move(canvas));
    output.exampleWaveformIdentifiers.push_back(identifier);
  }
}

std::string JoinPath(const std::string& directory, const std::string& fileName)
{
  if (directory.empty() || directory == ".") {
    return fileName;
  }
  return directory.back() == '/'
      ? directory + fileName
      : directory + "/" + fileName;
}

bool PrepareFigureOutputDirectory(const std::string& directory)
{
  if (directory.empty() || directory == ".") {
    return true;
  }
  if (!gSystem->AccessPathName(directory.c_str())) {
    return true;
  }
  if (gSystem->mkdir(directory.c_str(), true) == 0) {
    return true;
  }

  std::cerr << "Could not create figure output directory: " << directory
            << std::endl;
  return false;
}

void SaveCanvasAsPdf(TCanvas& canvas,
                     const std::string& path,
                     AnalysisOutput& output)
{
  canvas.Modified();
  canvas.Update();
  const double canvasWidth = canvas.GetWw();
  const double canvasHeight = canvas.GetWh();
  if (canvasWidth > 0.0 && canvasHeight > 0.0) {
    constexpr double paperWidth = 20.0;
    gStyle->SetPaperSize(paperWidth,
                         paperWidth * canvasHeight / canvasWidth);
  }
  canvas.Print(path.c_str(), "pdf");
  if (gSystem->AccessPathName(path.c_str())) {
    std::cerr << "Could not save figure: " << path << std::endl;
    return;
  }

  output.savedFigureFiles.push_back(path);
  std::cout << "Saved figure: " << path << std::endl;
}

void SaveFiguresAsPdf(AnalysisOutput& output, const AnalysisConfig& config)
{
  const std::string directory = config.figureOutputDirectory.empty()
      ? "."
      : config.figureOutputDirectory;
  if (!PrepareFigureOutputDirectory(directory)) {
    return;
  }

  if (output.bgoEnergyCanvas) {
    SaveCanvasAsPdf(*output.bgoEnergyCanvas,
                    JoinPath(directory, "bgo_energy_comparison.pdf"),
                    output);
  }
  if (output.integratedChargeCanvas) {
    SaveCanvasAsPdf(*output.integratedChargeCanvas,
                    JoinPath(directory, "integrated_charge_comparison.pdf"),
                    output);
  }
  for (std::size_t iCanvas = 0;
       iCanvas < output.exampleWaveformCanvases.size();
       ++iCanvas) {
    SaveCanvasAsPdf(
        *output.exampleWaveformCanvases[iCanvas],
        JoinPath(directory,
                 "example_waveforms_" +
                     output.exampleWaveformIdentifiers[iCanvas] + ".pdf"),
        output);
  }
}

void PrintSampleSummary(const SampleResult& result)
{
  const std::string label = result.sample.label.empty()
      ? result.sample.fileName
      : result.sample.label;
  std::cout << "Sample: " << label << std::endl;
  if (!result.inputReadable) {
    std::cout << "  error = " << result.error << std::endl;
    return;
  }

  std::cout << "  file = " << result.openedFileName << std::endl;
  std::cout << "  tree entries = " << result.totalEntries << std::endl;
  std::cout << "  events with positive BGO energy deposition = "
            << result.selectedEventCount << std::endl;
  std::cout << "  valid integrated charges = "
            << result.integratedCharges.size() << std::endl;
  if (!result.hasWaveformBranch) {
    std::cout << "  warning = no pmtWaveform branch; no charge distribution or example "
                 "waveforms were produced"
              << std::endl;
  }
  if (result.emptyWaveformCount > 0 ||
      result.incompleteWaveformCount > 0 ||
      result.nonFiniteWaveformCount > 0 ||
      result.invalidChargeConfigurationCount > 0) {
    std::cout << "  rejected waveforms: empty = " << result.emptyWaveformCount
              << ", incomplete = " << result.incompleteWaveformCount
              << ", non-finite = " << result.nonFiniteWaveformCount
              << ", invalid configuration = "
              << result.invalidChargeConfigurationCount << std::endl;
  }
  if (result.invalidEnergyCount > 0 || result.unreadableEntryCount > 0) {
    std::cout << "  rejected entries: invalid energy = "
              << result.invalidEnergyCount
              << ", unreadable = " << result.unreadableEntryCount << std::endl;
  }
}
}

SampleResult AnalyzeSample(const SampleSpec& sample,
                           const AnalysisConfig& config)
{
  SampleResult result;
  result.sample = sample;
  if (result.sample.label.empty()) {
    result.sample.label = sample.fileName;
  }

  auto inputFile = OpenInputFile(sample.fileName, result.openedFileName);
  if (!inputFile) {
    result.error = "could not open ROOT file: " + sample.fileName;
    return result;
  }

  TTree* events = nullptr;
  inputFile->GetObject("Events", events);
  if (!events) {
    result.error = "could not find TTree named 'Events'";
    return result;
  }

  if (!events->GetBranch("eventID") || !events->GetBranch("totalEdepBGO")) {
    result.error = "required eventID or totalEdepBGO branch is missing";
    return result;
  }

  Int_t eventID = 0;
  Double_t totalEdepBGO = 0.0;
  std::vector<double>* pmtWaveform = nullptr;
  if (events->SetBranchAddress("eventID", &eventID) < 0 ||
      events->SetBranchAddress("totalEdepBGO", &totalEdepBGO) < 0) {
    result.error = "required eventID or totalEdepBGO branch has an incompatible type";
    return result;
  }

  result.hasWaveformBranch = events->GetBranch("pmtWaveform") != nullptr;
  if (result.hasWaveformBranch &&
      events->SetBranchAddress("pmtWaveform", &pmtWaveform) < 0) {
    result.error = "pmtWaveform branch has an incompatible type";
    return result;
  }

  result.inputReadable = true;
  result.totalEntries = events->GetEntries();
  const std::vector<double> emptyWaveform;
  for (Long64_t iEntry = 0; iEntry < result.totalEntries; ++iEntry) {
    if (events->GetEntry(iEntry) <= 0) {
      ++result.unreadableEntryCount;
      continue;
    }
    if (!std::isfinite(totalEdepBGO)) {
      ++result.invalidEnergyCount;
      continue;
    }
    if (totalEdepBGO <= 0.0) {
      continue;
    }

    result.bgoEnergyDepositions.push_back(totalEdepBGO);
    if (!result.hasWaveformBranch) {
      continue;
    }

    const auto chargeResult = CalculateIntegratedCharge(
        pmtWaveform ? *pmtWaveform : emptyWaveform, config.charge);
    switch (chargeResult.status) {
      case ChargeStatus::Valid:
        result.integratedCharges.push_back(chargeResult.integratedCharge);
        if (result.exampleWaveforms.size() < config.exampleWaveformCount) {
          result.exampleWaveformEventIDs.push_back(eventID);
          result.exampleWaveforms.push_back(*pmtWaveform);
        }
        break;
      case ChargeStatus::EmptyWaveform:
        ++result.emptyWaveformCount;
        break;
      case ChargeStatus::IncompleteWaveform:
        ++result.incompleteWaveformCount;
        break;
      case ChargeStatus::NonFiniteSample:
        ++result.nonFiniteWaveformCount;
        break;
      case ChargeStatus::InvalidConfiguration:
        ++result.invalidChargeConfigurationCount;
        break;
    }
  }
  result.selectedEventCount = result.bgoEnergyDepositions.size();
  return result;
}

AnalysisOutput* AnalyzeSamples(const std::vector<SampleSpec>& sampleSpecs,
                               const AnalysisConfig& config)
{
  latestAnalysis.reset();
  ConfigurePublicationStyle();
  auto output = std::make_unique<AnalysisOutput>();

  if (sampleSpecs.empty()) {
    std::cerr << "No samples were provided for analysis." << std::endl;
    latestAnalysis = std::move(output);
    return latestAnalysis.get();
  }
  if (!IsValidHistogramConfig(config.bgoEnergyHistogram) ||
      !IsValidHistogramConfig(config.integratedChargeHistogram)) {
    std::cerr << "Histogram bin counts and ranges must be valid." << std::endl;
    latestAnalysis = std::move(output);
    return latestAnalysis.get();
  }

  output->samples.reserve(sampleSpecs.size());
  for (const SampleSpec& sample : sampleSpecs) {
    output->samples.push_back(AnalyzeSample(sample, config));
    PrintSampleSummary(output->samples.back());
  }

  output->histograms.reserve(output->samples.size());
  for (std::size_t iSample = 0; iSample < output->samples.size(); ++iSample) {
    const SampleResult& result = output->samples[iSample];
    const std::string identifier = MakeIdentifier(result.sample) +
        "_" + std::to_string(iSample);

    SampleHistograms sampleHistograms;
    sampleHistograms.sampleIndex = iSample;
    sampleHistograms.bgoEnergy = MakeNormalizedHistogram(
        "bgo_energy_" + identifier,
        ";Energy deposited in BGO [MeV];Probability density",
        config.bgoEnergyHistogram,
        result.bgoEnergyDepositions,
        result.sample);
    sampleHistograms.integratedCharge = MakeNormalizedHistogram(
        "integrated_charge_" + identifier,
        ";Integrated charge [a.u. ns];Probability density",
        config.integratedChargeHistogram,
        result.integratedCharges,
        result.sample);
    output->histograms.push_back(std::move(sampleHistograms));
  }

  const bool hasEnergyHistogram = std::any_of(
      output->histograms.begin(),
      output->histograms.end(),
      [](const SampleHistograms& histograms) { return histograms.bgoEnergy != nullptr; });
  if (hasEnergyHistogram) {
    output->bgoEnergyCanvas =
        std::make_unique<TCanvas>("c_bgo_energy_comparison",
                                  "BGO energy deposition comparison",
                                  800,
                                  650);
    StyleComparisonCanvas(*output->bgoEnergyCanvas);
    output->bgoEnergyLegend = DrawComparisonHistogram(*output->bgoEnergyCanvas,
                                                      output->samples,
                                                      output->histograms,
                                                      false);
  } else {
    std::cerr << "No selected BGO energy values were available for comparison."
              << std::endl;
  }

  const bool hasChargeHistogram = std::any_of(
      output->histograms.begin(),
      output->histograms.end(),
      [](const SampleHistograms& histograms) {
        return histograms.integratedCharge != nullptr;
      });
  if (hasChargeHistogram) {
    output->integratedChargeCanvas =
        std::make_unique<TCanvas>("c_integrated_charge_comparison",
                                  "Integrated charge comparison",
                                  800,
                                  650);
    StyleComparisonCanvas(*output->integratedChargeCanvas);
    output->integratedChargeLegend = DrawComparisonHistogram(
        *output->integratedChargeCanvas,
        output->samples,
        output->histograms,
        true);
  } else {
    std::cerr << "No valid integrated charges were available for comparison."
              << std::endl;
  }

  DrawExampleWaveforms(*output);
  SaveFiguresAsPdf(*output, config);
  latestAnalysis = std::move(output);
  return latestAnalysis.get();
}

const AnalysisOutput* LatestAnalysis()
{
  return latestAnalysis.get();
}
}
}

void analyze_events(
    const std::vector<AmBeTagger::Analysis::SampleSpec>& samples,
    const AmBeTagger::Analysis::AnalysisConfig& config = {})
{
  AmBeTagger::Analysis::AnalyzeSamples(samples, config);
}

void analyze_events(const char* fileName = "output.root")
{
  const AmBeTagger::Analysis::SampleSpec sample{
      fileName,
      fileName,
      "sample",
      kBlack,
      1,
  };
  AmBeTagger::Analysis::AnalyzeSamples(
      {sample}, AmBeTagger::Analysis::AnalysisConfig{});
}
