#include "AmBeTagger/DetectorConstruction.hh"
#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4Polycone.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

#include "G4MaterialPropertiesTable.hh"

#include "G4LogicalBorderSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4VisAttributes.hh"
#include <vector>

namespace
{
G4VisAttributes* MakeVisAttributes(const G4Colour& colour)
{
  auto* attributes = new G4VisAttributes(colour);
  attributes->SetVisibility(true);
  attributes->SetForceSolid(true);
  attributes->SetForceAuxEdgeVisible(true);
  return attributes;
}
}

namespace AmBeTagger
{
G4VPhysicalVolume* DetectorConstruction::Construct()
{
  G4NistManager* nist = G4NistManager::Instance();

  G4Material* air = nist->FindOrBuildMaterial("G4_AIR");
  G4Material* teflon = nist->FindOrBuildMaterial("G4_TEFLON");
  G4Material* nistStainlessSteel =
      nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  G4Material* nistPolyvinylChloride =
      nist->FindOrBuildMaterial("G4_POLYVINYL_CHLORIDE");
  G4Element* hydrogen = nist->FindOrBuildElement("H");
  G4Element* carbon = nist->FindOrBuildElement("C");
  G4Element* nitrogen = nist->FindOrBuildElement("N");
  G4Element* bismuth = nist->FindOrBuildElement("Bi");
  G4Element* germanium = nist->FindOrBuildElement("Ge");
  G4Element* oxygen = nist->FindOrBuildElement("O");
  G4Element* boron = nist->FindOrBuildElement("B");
  G4Element* sodium = nist->FindOrBuildElement("Na");
  G4Element* aluminum = nist->FindOrBuildElement("Al");

  constexpr G4double bgoDensity = 7.13 * g / cm3; 

  G4Material* bgo = new G4Material(
    "BGO",
    bgoDensity,
    3);

  bgo->AddElement(bismuth,4);
  bgo->AddElement(germanium,3);
  bgo->AddElement(oxygen,12);


  const std::vector<G4double> photonEnergy =  {
        1.771*eV, 1.784*eV, 1.797*eV, 1.810*eV, 1.823*eV, 1.837*eV, 1.850*eV, 1.864*eV, 1.878*eV, 1.893*eV,
        1.907*eV, 1.922*eV, 1.937*eV, 1.952*eV, 1.968*eV, 1.984*eV, 2.000*eV, 2.016*eV, 2.032*eV, 2.049*eV,
        2.066*eV, 2.084*eV, 2.101*eV, 2.119*eV, 2.138*eV, 2.156*eV, 2.175*eV, 2.194*eV, 2.214*eV, 2.234*eV,
        2.254*eV, 2.275*eV, 2.296*eV, 2.317*eV, 2.339*eV, 2.362*eV, 2.384*eV, 2.407*eV, 2.431*eV, 2.455*eV,
        2.480*eV, 2.505*eV, 2.530*eV, 2.556*eV, 2.583*eV, 2.610*eV, 2.638*eV, 2.666*eV, 2.695*eV, 2.725*eV,
        2.755*eV, 2.786*eV, 2.818*eV, 2.850*eV, 2.883*eV, 2.917*eV, 2.952*eV, 2.987*eV, 3.024*eV, 3.061*eV,
        3.100*eV, 3.139*eV, 3.179*eV, 3.220*eV, 3.263*eV, 3.306*eV, 3.351*eV, 3.397*eV, 3.444*eV, 3.492*eV,
        3.542*eV, 3.594*eV, 3.646*eV, 3.701*eV, 3.757*eV, 3.815*eV, 3.874*eV, 3.936*eV, 3.999*eV, 4.065*eV,
        4.133*eV, 4.203*eV, 4.275*eV, 4.350*eV, 4.428*eV, 4.508*eV, 4.592*eV, 4.678*eV, 4.768*eV, 4.862*eV,
        4.959*eV, 5.060*eV, 5.166*eV, 5.276*eV, 5.390*eV, 5.510*eV, 5.635*eV, 5.767*eV, 5.904*eV, 6.048*eV,
        6.199*eV, 6.358*eV, 6.525*eV, 6.702*eV, 6.888*eV, 7.085*eV};


  const std::vector<G4double> bgoRefractiveIndex = {
        2.0843, 2.0849, 2.0858, 2.0871, 2.0877, 2.0885, 2.0900, 2.0906, 2.0919, 2.0928,
        2.0938, 2.0949, 2.0956, 2.0969, 2.0984, 2.0990, 2.1003, 2.1018, 2.1032, 2.1045,
        2.1060, 2.1072, 2.1089, 2.1102, 2.1117, 2.1130, 2.1149, 2.1166, 2.1183, 2.1201,
        2.1218, 2.1235, 2.1257, 2.1276, 2.1296, 2.1320, 2.1339, 2.1362, 2.1385, 2.1410,
        2.1435, 2.1460, 2.1488, 2.1518, 2.1543, 2.1576, 2.1608, 2.1640, 2.1672, 2.1709,
        2.1744, 2.1783, 2.1823, 2.1865, 2.1912, 2.1957, 2.2001, 2.2051, 2.2103, 2.2156,
        2.2213, 2.2272, 2.2335, 2.2407, 2.2480, 2.2550, 2.2626, 2.2708, 2.2795, 2.2889,
        2.2984, 2.3089, 2.3196, 2.3311, 2.3437, 2.3570, 2.3715, 2.3867, 2.4032, 2.4159,
        2.4303, 2.4447, 2.4590, 2.4734, 2.4878, 2.5022, 2.5166, 2.5310, 2.5454, 2.5598,
        2.5742, 2.5885, 2.6029, 2.6173, 2.6317, 2.6461, 2.6605, 2.6749, 2.6893, 2.7037,
        2.7180, 2.7324, 2.7468, 2.7612, 2.7756, 2.7900};

  const std::vector<G4double> bgoScintillationFast =  {
        0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
        0.0000, 0.0826, 0.0943, 0.1107, 0.1316, 0.1504, 0.1712, 0.1901, 0.2109, 0.2307,
        0.2510, 0.2785, 0.3060, 0.3399, 0.3800, 0.4184, 0.4487, 0.4763, 0.5089, 0.5449,
        0.5810, 0.6244, 0.6682, 0.7102, 0.7504, 0.7905, 0.8301, 0.8709, 0.8979, 0.9094,
        0.9220, 0.9492, 0.9769, 0.9913, 0.9913, 0.9911, 0.9762, 0.9602, 0.9362, 0.9036,
        0.8704, 0.8142, 0.7525, 0.6948, 0.6422, 0.5885, 0.5065, 0.4223, 0.3542, 0.3019,
        0.2496, 0.1899, 0.1304, 0.0796, 0.0400, 0.0037, 0.0000, 0.0000, 0.0000, 0.0000,
        0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
        0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
        0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
        0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000};

  const std::vector<G4double> bgoScintillationSlow = {
        0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
        0.0000, 0.0826, 0.0943, 0.1107, 0.1316, 0.1504, 0.1712, 0.1901, 0.2109, 0.2307,
        0.2510, 0.2785, 0.3060, 0.3399, 0.3800, 0.4184, 0.4487, 0.4763, 0.5089, 0.5449,
        0.5810, 0.6244, 0.6682, 0.7102, 0.7504, 0.7905, 0.8301, 0.8709, 0.8979, 0.9094,
        0.9220, 0.9492, 0.9769, 0.9913, 0.9913, 0.9911, 0.9762, 0.9602, 0.9362, 0.9036,
        0.8704, 0.8142, 0.7525, 0.6948, 0.6422, 0.5885, 0.5065, 0.4223, 0.3542, 0.3019,
        0.2496, 0.1899, 0.1304, 0.0796, 0.0400, 0.0037, 0.0000, 0.0000, 0.0000, 0.0000,
        0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
        0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
        0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
        0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000};


  const std::vector<G4double> bgoAbsorptionLength = {
        10.05*cm, 10.03*cm, 10.01*cm, 9.99*cm, 10.04*cm, 10.04*cm, 9.93*cm, 9.93*cm, 9.92*cm, 9.85*cm,
         9.95*cm,  9.92*cm,  9.79*cm, 9.85*cm,  9.87*cm,  9.86*cm, 9.78*cm, 9.83*cm, 9.80*cm, 9.75*cm,
         9.70*cm,  9.72*cm,  9.65*cm, 9.76*cm,  9.73*cm,  9.70*cm, 9.68*cm, 9.65*cm, 9.70*cm, 9.70*cm,
         9.65*cm,  9.58*cm,  9.65*cm, 9.64*cm,  9.60*cm,  9.51*cm, 9.47*cm, 9.47*cm, 9.42*cm, 9.43*cm,
         9.40*cm,  9.39*cm,  9.33*cm, 9.39*cm,  9.33*cm,  9.26*cm, 9.25*cm, 9.17*cm, 9.13*cm, 9.08*cm,
         9.08*cm,  9.06*cm,  8.95*cm, 8.89*cm,  8.86*cm,  8.90*cm, 8.82*cm, 8.71*cm, 8.72*cm, 8.60*cm,
         8.53*cm,  8.40*cm,  8.45*cm, 8.25*cm,  8.22*cm,  7.97*cm, 7.70*cm, 7.76*cm, 7.54*cm, 7.30*cm,
         7.06*cm,  6.71*cm,  6.27*cm, 5.98*cm,  5.52*cm,  4.94*cm, 4.15*cm, 2.60*cm, 1.06*cm, 0.54*cm,
         1e-6*cm,  1e-6*cm,  1e-6*cm, 1e-6*cm,  1e-6*cm,  1e-6*cm, 1e-6*cm, 1e-6*cm, 1e-6*cm, 1e-6*cm,
         1e-6*cm,  1e-6*cm,  1e-6*cm, 1e-6*cm,  1e-6*cm,  1e-6*cm, 1e-6*cm, 1e-6*cm, 1e-6*cm, 1e-6*cm,
         1e-6*cm,  1e-6*cm,  1e-6*cm, 1e-6*cm,  1e-6*cm, 1e-6*cm};

  constexpr G4double bgoFastFraction = 0.0;
  constexpr G4double bgoSlowFraction = 1.0;

  G4MaterialPropertiesTable* bgoMpt = new G4MaterialPropertiesTable;


  bgoMpt->AddProperty("ABSLENGTH", photonEnergy, bgoAbsorptionLength);
  bgoMpt->AddProperty("RINDEX", photonEnergy, bgoRefractiveIndex);
  bgoMpt->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, bgoScintillationFast);
  bgoMpt->AddProperty("SCINTILLATIONCOMPONENT2", photonEnergy, bgoScintillationSlow);
  bgoMpt->AddConstProperty("SCINTILLATIONYIELD", 10000. / MeV);
  bgoMpt->AddConstProperty("RESOLUTIONSCALE", 2.0);
  bgoMpt->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 1.0 * ns);
  bgoMpt->AddConstProperty("SCINTILLATIONTIMECONSTANT2", 300.0 * ns);
  bgoMpt->AddConstProperty("SCINTILLATIONYIELD1", bgoFastFraction);
  bgoMpt->AddConstProperty("SCINTILLATIONYIELD2", bgoSlowFraction);

  bgo->SetMaterialPropertiesTable(bgoMpt);

  const auto n_entries = photonEnergy.size();
  const std::vector<G4double> airRefractiveIndex(n_entries,1.0);
  const std::vector<G4double> airAbsorptionLength(n_entries,100*m);


  G4MaterialPropertiesTable* airMpt = new G4MaterialPropertiesTable;
  airMpt->AddProperty("RINDEX", photonEnergy, airRefractiveIndex);
  airMpt->AddProperty("ABSLENGTH", photonEnergy, airAbsorptionLength);
  air->SetMaterialPropertiesTable(airMpt);

  G4Element* silicon = nist->FindOrBuildElement("Si");
  constexpr G4double ej550Density = 1.06 * g / cm3;

  G4Material* opticalCoupling = new G4Material(
      "EJ550OpticalGrease",
      ej550Density,
      1);
  opticalCoupling->AddElement(silicon, 1);

  const std::vector<G4double> couplingPhotonEnergy = {
      1.5 * eV,
      7.5 * eV};

  const std::vector<G4double> couplingRefractiveIndex = {
      1.46,
      1.46};

  const std::vector<G4double> couplingAbsorptionLength = {
      10.0 * m,
      10.0 * m};

  G4MaterialPropertiesTable* couplingMpt = new G4MaterialPropertiesTable;
  couplingMpt->AddProperty(
      "RINDEX",
      couplingPhotonEnergy,
      couplingRefractiveIndex);
  couplingMpt->AddProperty(
      "ABSLENGTH",
      couplingPhotonEnergy,
      couplingAbsorptionLength);
  opticalCoupling->SetMaterialPropertiesTable(couplingMpt);

  constexpr G4double uvGlassDensity = 2.2 * g / cm3;

  G4Material* pmtWindowMaterial = new G4Material(
      "UVGlass",
      uvGlassDensity,
      2);
  pmtWindowMaterial->AddElement(silicon, 1);
  pmtWindowMaterial->AddElement(oxygen, 2);

  const std::vector<G4double> pmtWindowRefractiveIndex = {
      1.49,
      1.49};

  const std::vector<G4double> pmtWindowAbsorptionLength = {
      10.0 * m,
      10.0 * m};

  G4MaterialPropertiesTable* pmtWindowMpt = new G4MaterialPropertiesTable;
  pmtWindowMpt->AddProperty(
      "RINDEX",
      couplingPhotonEnergy,
      pmtWindowRefractiveIndex);
  pmtWindowMpt->AddProperty(
      "ABSLENGTH",
      couplingPhotonEnergy,
      pmtWindowAbsorptionLength);
  pmtWindowMaterial->SetMaterialPropertiesTable(pmtWindowMpt);

  constexpr G4double borosilicateDensity = 2.23 * g / cm3;

  G4Material* pmtBodyMaterial = new G4Material(
      "BorosilicateGlass",
      borosilicateDensity,
      5);
  pmtBodyMaterial->AddElement(oxygen, 0.539);
  pmtBodyMaterial->AddElement(silicon, 0.377);
  pmtBodyMaterial->AddElement(boron, 0.040);
  pmtBodyMaterial->AddElement(sodium, 0.030);
  pmtBodyMaterial->AddElement(aluminum, 0.014);

  G4MaterialPropertiesTable* pmtBodyMpt = new G4MaterialPropertiesTable;
  pmtBodyMpt->AddProperty(
      "RINDEX",
      couplingPhotonEnergy,
      pmtWindowRefractiveIndex);
  pmtBodyMpt->AddProperty(
      "ABSLENGTH",
      couplingPhotonEnergy,
      pmtWindowAbsorptionLength);
  pmtBodyMaterial->SetMaterialPropertiesTable(pmtBodyMpt);

  constexpr G4double polyurethaneFoamDensity = 0.032 * g / cm3;

  G4Material* polyurethaneFoam = new G4Material(
      "PolyurethaneFoam",
      polyurethaneFoamDensity,
      4);
  polyurethaneFoam->AddElement(hydrogen, 0.060);
  polyurethaneFoam->AddElement(carbon, 0.630);
  polyurethaneFoam->AddElement(nitrogen, 0.060);
  polyurethaneFoam->AddElement(oxygen, 0.250);

  constexpr G4double stainlessSteelDensity = 8.0 * g / cm3;

  G4Material* stainlessSteel = new G4Material(
      "StainlessSteel",
      stainlessSteelDensity,
      1);
  stainlessSteel->AddMaterial(nistStainlessSteel, 1.0);

  constexpr G4double whitePvcDensity = 1.38 * g / cm3;

  G4Material* whitePvc = new G4Material(
      "WhitePVC",
      whitePvcDensity,
      1);
  whitePvc->AddMaterial(nistPolyvinylChloride, 1.0);


  constexpr G4double worldHalfLength = 50.0 * cm;

  G4Box* worldSolid = new G4Box(
      "WorldSolid",
      worldHalfLength,
      worldHalfLength,
      worldHalfLength);

  G4LogicalVolume* worldLogical = new G4LogicalVolume(
      worldSolid,
      air,
      "WorldLogical");
  worldLogical->SetVisAttributes(G4VisAttributes::GetInvisible());

  G4VPhysicalVolume* worldPhysical = new G4PVPlacement(
    nullptr,
    G4ThreeVector{},
    worldLogical,
    "WorldPhysical",
    nullptr,
    false,
    0,
    true);


   constexpr G4double bgoMaxRadius = 2.5 * cm;
   constexpr G4double bgoMinRadius = 1.0 * mm;
   constexpr G4double bgoHalfLengthZ = 2.5 * cm;


   G4Tubs* bgoSolid = new G4Tubs(
    "bgoSolid",
    bgoMinRadius,
    bgoMaxRadius,
    bgoHalfLengthZ,
    0.0 * deg,
    360.0 * deg);

   scoringVolume_ = new G4LogicalVolume(
        bgoSolid,
        bgo,
        "bgoLogical");
   scoringVolume_->SetVisAttributes(
       MakeVisAttributes(G4Colour(0.05, 0.25, 1.0, 0.38)));

  G4VPhysicalVolume* bgoPhysical = new G4PVPlacement(
    nullptr,
    G4ThreeVector{},
    scoringVolume_,
    "bgoPhysical",
    worldLogical,
    false,
    0,
    true);

  const G4Colour stainlessSteelColour(0.58, 0.60, 0.63, 0.76);

  constexpr G4double sourceShieldRadius = 26.025 * mm;
  constexpr G4double sourceShieldHalfThickness = 0.5 * 11.63 * mm;
  constexpr G4double sourceCapsuleOuterRadius = 7.8 * mm;
  constexpr G4double sourceCapsuleInnerRadius = 6.8 * mm;
  constexpr G4double sourceCapsuleHalfLength = 5.0 * mm;
  constexpr G4double sourceSurroundingShieldInnerRadius = 8.0 * mm;
  constexpr G4double sourceShieldFrontZ =
      -bgoHalfLengthZ - 0.25 * mm;
  constexpr G4double sourceShieldZ =
      sourceShieldFrontZ - sourceShieldHalfThickness;
  constexpr G4double sourceShieldBackZ =
      sourceShieldZ - sourceShieldHalfThickness;
  constexpr G4double sourceCapsuleZ =
      sourceShieldBackZ - sourceCapsuleHalfLength;

  G4Tubs* sourceShieldSolid = new G4Tubs(
      "SourceDownstreamShieldSolid",
      0.0 * cm,
      sourceShieldRadius,
      sourceShieldHalfThickness,
      0.0 * deg,
      360.0 * deg);

  G4LogicalVolume* sourceShieldLogical = new G4LogicalVolume(
      sourceShieldSolid,
      stainlessSteel,
      "SourceDownstreamShieldLogical");
  sourceShieldLogical->SetVisAttributes(
      MakeVisAttributes(stainlessSteelColour));

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, sourceShieldZ),
      sourceShieldLogical,
      "SourceDownstreamShieldPhysical",
      worldLogical,
      false,
      0,
      true);

  const std::vector<G4double> capsuleZPlanes = {
      -5.0 * mm,
      -4.0 * mm,
      -3.9999 * mm,
      3.9999 * mm,
      4.0 * mm,
      5.0 * mm};
  const std::vector<G4double> capsuleInnerRadii = {
      0.0 * mm,
      0.0 * mm,
      sourceCapsuleInnerRadius,
      sourceCapsuleInnerRadius,
      0.0 * mm,
      0.0 * mm};
  const std::vector<G4double> capsuleOuterRadii = {
      sourceCapsuleOuterRadius,
      sourceCapsuleOuterRadius,
      sourceCapsuleOuterRadius,
      sourceCapsuleOuterRadius,
      sourceCapsuleOuterRadius,
      sourceCapsuleOuterRadius};

  G4Polycone* sourceCapsuleSolid = new G4Polycone(
      "SourceCapsuleSolid",
      0.0 * deg,
      360.0 * deg,
      static_cast<G4int>(capsuleZPlanes.size()),
      capsuleZPlanes.data(),
      capsuleInnerRadii.data(),
      capsuleOuterRadii.data());

  G4LogicalVolume* sourceCapsuleLogical = new G4LogicalVolume(
      sourceCapsuleSolid,
      stainlessSteel,
      "SourceCapsuleLogical");
  sourceCapsuleLogical->SetVisAttributes(
      MakeVisAttributes(stainlessSteelColour));

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, sourceCapsuleZ),
      sourceCapsuleLogical,
      "SourceCapsulePhysical",
      worldLogical,
      false,
      0,
      true);

  G4Tubs* sourceSurroundingShieldSolid = new G4Tubs(
      "SourceSurroundingShieldSolid",
      sourceSurroundingShieldInnerRadius,
      sourceShieldRadius,
      sourceCapsuleHalfLength,
      0.0 * deg,
      360.0 * deg);

  G4LogicalVolume* sourceSurroundingShieldLogical = new G4LogicalVolume(
      sourceSurroundingShieldSolid,
      stainlessSteel,
      "SourceSurroundingShieldLogical");
  sourceSurroundingShieldLogical->SetVisAttributes(
      MakeVisAttributes(stainlessSteelColour));

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, sourceCapsuleZ),
      sourceSurroundingShieldLogical,
      "SourceSurroundingShieldPhysical",
      worldLogical,
      false,
      0,
      true);


  
  //const std::vector<G4double> tefRefractiveIndex(n_entries,1.35);
  //const std::vector<G4double> tefAbsorptionLength(n_entries,1e-6*m);
  const std::vector<G4double> tefReflectance(n_entries,0.9);
  const std::vector<G4double> tefEfficiency(n_entries,0.0);

  G4MaterialPropertiesTable* wrapMpt = new G4MaterialPropertiesTable;
  wrapMpt->AddProperty("REFLECTIVITY", photonEnergy, tefReflectance);
  //wrapMpt->AddProperty("ABSLENGTH", photonEnergy, tefAbsorptionLength);
  //wrapMpt->AddProperty("RINDEX",photonEnergy, tefRefractiveIndex);
  wrapMpt->AddProperty("EFFICIENCY", photonEnergy, tefEfficiency);

  G4OpticalSurface* wrapSurface = new G4OpticalSurface("BgoWrapSurface");
  wrapSurface->SetType(dielectric_metal);
  wrapSurface->SetFinish(ground);
  wrapSurface->SetModel(glisur);
  wrapSurface->SetMaterialPropertiesTable(wrapMpt);

  constexpr G4double wrapThickness = 0.25 * mm;
  constexpr G4double wrapOuterRadius = bgoMaxRadius + wrapThickness;
  constexpr G4double wrapHalfThickness = 0.5 * wrapThickness;
  constexpr G4double pmtWindowRadius = 14.25 * mm;

  G4Tubs* wrapBarrelSolid = new G4Tubs(
      "BgoTeflonBarrelWrapSolid",
      bgoMaxRadius,
      wrapOuterRadius,
      bgoHalfLengthZ,
      0.0 * deg,
      360.0 * deg);

  G4LogicalVolume* wrapBarrelLogical = new G4LogicalVolume(
      wrapBarrelSolid,
      teflon,
      "BgoTeflonBarrelWrapLogical");
  wrapBarrelLogical->SetVisAttributes(
      MakeVisAttributes(G4Colour(0.92, 0.92, 0.86, 0.72)));

  G4VPhysicalVolume* wrapBarrelPhysical = new G4PVPlacement(
      nullptr,
      G4ThreeVector{},
      wrapBarrelLogical,
      "BgoTeflonBarrelWrapPhysical",
      worldLogical,
      false,
      0,
      true);

  G4Tubs* wrapBackCapSolid = new G4Tubs(
      "BgoTeflonBackCapSolid",
      0.0 * cm,
      wrapOuterRadius,
      wrapHalfThickness,
      0.0 * deg,
      360.0 * deg);

  G4LogicalVolume* wrapBackCapLogical = new G4LogicalVolume(
      wrapBackCapSolid,
      teflon,
      "BgoTeflonBackCapLogical");
  wrapBackCapLogical->SetVisAttributes(
      MakeVisAttributes(G4Colour(0.92, 0.92, 0.86, 0.72)));

  G4VPhysicalVolume* wrapBackCapPhysical = new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm,
                    -bgoHalfLengthZ - wrapHalfThickness),
      wrapBackCapLogical,
      "BgoTeflonBackCapPhysical",
      worldLogical,
      false,
      0,
      true);

  G4Tubs* wrapFrontCapSolid = new G4Tubs(
      "BgoTeflonFrontCapSolid",
      pmtWindowRadius,
      wrapOuterRadius,
      wrapHalfThickness,
      0.0 * deg,
      360.0 * deg);

  G4LogicalVolume* wrapFrontCapLogical = new G4LogicalVolume(
      wrapFrontCapSolid,
      teflon,
      "BgoTeflonFrontCapLogical");
  wrapFrontCapLogical->SetVisAttributes(
      MakeVisAttributes(G4Colour(0.92, 0.92, 0.86, 0.72)));

  G4VPhysicalVolume* wrapFrontCapPhysical = new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm,
                    bgoHalfLengthZ + wrapHalfThickness),
      wrapFrontCapLogical,
      "BgoTeflonFrontCapPhysical",
      worldLogical,
      false,
      0,
      true);

  new G4LogicalBorderSurface(
      "BgoToTeflonBarrelWrapSurface",
      bgoPhysical,
      wrapBarrelPhysical,
      wrapSurface);

  new G4LogicalBorderSurface(
      "BgoToTeflonBackCapSurface",
      bgoPhysical,
      wrapBackCapPhysical,
      wrapSurface);

  new G4LogicalBorderSurface(
      "BgoToTeflonFrontCapSurface",
      bgoPhysical,
      wrapFrontCapPhysical,
      wrapSurface);


  constexpr G4double couplingRadius = pmtWindowRadius;
  constexpr G4double couplingHalfThickness = 0.05 * mm;
  constexpr G4double bgoFrontZ = bgoHalfLengthZ;
  constexpr G4double couplingZ = bgoFrontZ + couplingHalfThickness;

  G4Tubs* couplingSolid = new G4Tubs(
      "CouplingSolid",
      0.0 * cm,
      couplingRadius,
      couplingHalfThickness,
      0.0 * deg,
      360.0 * deg);

  G4LogicalVolume* couplingLogical = new G4LogicalVolume(
      couplingSolid,
      opticalCoupling,
      "CouplingLogical");
  couplingLogical->SetVisAttributes(
      MakeVisAttributes(G4Colour(1.0, 0.82, 0.05, 0.55)));

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, couplingZ),
      couplingLogical,
      "CouplingPhysical",
      worldLogical,
      false,
      0,
      true);

  constexpr G4double pmtWindowHalfThickness = 0.5 * mm;
  constexpr G4double pmtWindowZ =
      bgoFrontZ + 2.0 * couplingHalfThickness + pmtWindowHalfThickness;

  G4Tubs* pmtWindowSolid = new G4Tubs(
      "PmtWindowSolid",
      0.0 * cm,
      pmtWindowRadius,
      pmtWindowHalfThickness,
      0.0 * deg,
      360.0 * deg);

  G4LogicalVolume* pmtWindowLogical = new G4LogicalVolume(
      pmtWindowSolid,
      pmtWindowMaterial,
      "PmtWindowLogical");
  pmtWindowLogical->SetVisAttributes(
      MakeVisAttributes(G4Colour(0.0, 0.82, 0.95, 0.45)));

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, pmtWindowZ),
      pmtWindowLogical,
      "PmtWindowPhysical",
      worldLogical,
      false,
      0,
      true);

  constexpr G4double pmtPlaneRadius = 12.5 * mm;
  constexpr G4double pmtPlaneHalfThickness = 0.005 * mm;
  constexpr G4double pmtPlaneZ =
      bgoFrontZ + 2.0 * couplingHalfThickness
      + 2.0 * pmtWindowHalfThickness + pmtPlaneHalfThickness;

  G4Tubs* pmtPlaneSolid = new G4Tubs(
      "PmtPlaneSolid",
      0.0 * cm,
      pmtPlaneRadius,
      pmtPlaneHalfThickness,
      0.0 * deg,
      360.0 * deg);

  pmtPlaneVolume_ = new G4LogicalVolume(
      pmtPlaneSolid,
      air,
      "PmtPlaneLogical");
  pmtPlaneVolume_->SetVisAttributes(
      MakeVisAttributes(G4Colour(0.0, 0.95, 0.28, 0.9)));

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, pmtPlaneZ),
      pmtPlaneVolume_,
      "PmtPlanePhysical",
      worldLogical,
      false,
      0,
      true);

  constexpr G4double pmtBodyRadius = 12.25 * mm;
  constexpr G4double pmtBodyHalfLength = 50.0 * mm;
  constexpr G4double pmtBodyZ =
      pmtPlaneZ + pmtPlaneHalfThickness + pmtBodyHalfLength;

  G4Tubs* pmtBodySolid = new G4Tubs(
      "PmtBodySolid",
      0.0 * cm,
      pmtBodyRadius,
      pmtBodyHalfLength,
      0.0 * deg,
      360.0 * deg);

  G4LogicalVolume* pmtBodyLogical = new G4LogicalVolume(
      pmtBodySolid,
      pmtBodyMaterial,
      "PmtBodyLogical");
  pmtBodyLogical->SetVisAttributes(
      MakeVisAttributes(G4Colour(0.18, 0.22, 0.28, 0.42)));

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, pmtBodyZ),
      pmtBodyLogical,
      "PmtBodyPhysical",
      worldLogical,
      false,
      0,
      true);

  constexpr G4double pmtFoamInnerRadius = 14.29 * mm;
  constexpr G4double pmtFoamOuterRadius = 26.025 * mm;

  G4Tubs* pmtFoamSolid = new G4Tubs(
      "PmtFoamSupportSolid",
      pmtFoamInnerRadius,
      pmtFoamOuterRadius,
      pmtBodyHalfLength,
      0.0 * deg,
      360.0 * deg);

  G4LogicalVolume* pmtFoamLogical = new G4LogicalVolume(
      pmtFoamSolid,
      polyurethaneFoam,
      "PmtFoamSupportLogical");
  pmtFoamLogical->SetVisAttributes(
      MakeVisAttributes(G4Colour(0.04, 0.045, 0.055, 0.28)));

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, pmtBodyZ),
      pmtFoamLogical,
      "PmtFoamSupportPhysical",
      worldLogical,
      false,
      0,
      true);

  const G4Colour whitePvcColour(0.96, 0.96, 0.90, 0.30);

  constexpr G4double sourcePvcHolderHalfLength = 0.5 * 11.63 * mm;
  constexpr G4double sourcePvcHolderZ =
      sourceCapsuleZ - sourceCapsuleHalfLength - sourcePvcHolderHalfLength;
  const std::vector<G4double> sourcePvcHolderZPlanes = {
      -5.815 * mm,
      -4.1921 * mm,
      -4.185 * mm,
      5.815 * mm};
  const std::vector<G4double> sourcePvcHolderInnerRadii = {
      0.0 * mm,
      0.0 * mm,
      7.9 * mm,
      7.9 * mm};
  const std::vector<G4double> sourcePvcHolderOuterRadii = {
      sourceShieldRadius,
      sourceShieldRadius,
      sourceShieldRadius,
      sourceShieldRadius};

  G4Polycone* sourcePvcHolderSolid = new G4Polycone(
      "SourcePvcHolderSolid",
      0.0 * deg,
      360.0 * deg,
      static_cast<G4int>(sourcePvcHolderZPlanes.size()),
      sourcePvcHolderZPlanes.data(),
      sourcePvcHolderInnerRadii.data(),
      sourcePvcHolderOuterRadii.data());

  G4LogicalVolume* sourcePvcHolderLogical = new G4LogicalVolume(
      sourcePvcHolderSolid,
      whitePvc,
      "SourcePvcHolderLogical");
  sourcePvcHolderLogical->SetVisAttributes(
      MakeVisAttributes(whitePvcColour));

  G4VPhysicalVolume* sourcePvcHolderPhysical = new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, sourcePvcHolderZ),
      sourcePvcHolderLogical,
      "SourcePvcHolderPhysical",
      worldLogical,
      false,
      0,
      true);

  constexpr G4double readoutPvcHolderHalfLength = 60.0 * mm;
  constexpr G4double readoutPvcHolderZ =
      pmtBodyZ + pmtBodyHalfLength + readoutPvcHolderHalfLength;
  const std::vector<G4double> readoutPvcHolderZPlanes = {
      -60.0 * mm,
      -40.001 * mm,
      -40.0 * mm,
      -39.0 * mm,
      -38.999 * mm,
      60.0 * mm};
  const std::vector<G4double> readoutPvcHolderInnerRadii = {
      25.0 * mm,
      25.0 * mm,
      12.79 * mm,
      12.79 * mm,
      25.0 * mm,
      25.0 * mm};
  const std::vector<G4double> readoutPvcHolderOuterRadii = {
      sourceShieldRadius,
      sourceShieldRadius,
      sourceShieldRadius,
      sourceShieldRadius,
      sourceShieldRadius,
      sourceShieldRadius};

  G4Polycone* readoutPvcHolderSolid = new G4Polycone(
      "ReadoutPvcHolderSolid",
      0.0 * deg,
      360.0 * deg,
      static_cast<G4int>(readoutPvcHolderZPlanes.size()),
      readoutPvcHolderZPlanes.data(),
      readoutPvcHolderInnerRadii.data(),
      readoutPvcHolderOuterRadii.data());

  G4LogicalVolume* readoutPvcHolderLogical = new G4LogicalVolume(
      readoutPvcHolderSolid,
      whitePvc,
      "ReadoutPvcHolderLogical");
  readoutPvcHolderLogical->SetVisAttributes(
      MakeVisAttributes(whitePvcColour));

  G4VPhysicalVolume* readoutPvcHolderPhysical = new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, readoutPvcHolderZ),
      readoutPvcHolderLogical,
      "ReadoutPvcHolderPhysical",
      worldLogical,
      false,
      0,
      true);

  constexpr G4double outerPvcInnerRadius = sourceShieldRadius;
  constexpr G4double outerPvcOuterRadius = 35.235 * mm;
  constexpr G4double outerPvcBarrelHalfLength = 190.0 * mm;
  constexpr G4double rearEndCapHalfLength = 17.365 * mm;
  constexpr G4double rearEndCapInsertionStartZ = 2.636 * mm;
  constexpr G4double rearEndCapFrontZ = sourcePvcHolderZ
      - sourcePvcHolderHalfLength;
  constexpr G4double rearEndCapZ = rearEndCapFrontZ - rearEndCapHalfLength;
  constexpr G4double outerPvcBarrelBackZ =
      rearEndCapZ + rearEndCapInsertionStartZ;
  constexpr G4double outerPvcBarrelZ =
      outerPvcBarrelBackZ + outerPvcBarrelHalfLength;
  constexpr G4double outerPvcBarrelFrontZ =
      outerPvcBarrelZ + outerPvcBarrelHalfLength;

  G4Tubs* outerPvcBarrelSolid = new G4Tubs(
      "OuterPvcBarrelSolid",
      outerPvcInnerRadius,
      outerPvcOuterRadius,
      outerPvcBarrelHalfLength,
      0.0 * deg,
      360.0 * deg);

  G4LogicalVolume* outerPvcBarrelLogical = new G4LogicalVolume(
      outerPvcBarrelSolid,
      whitePvc,
      "OuterPvcBarrelLogical");
  outerPvcBarrelLogical->SetVisAttributes(
      MakeVisAttributes(whitePvcColour));

  G4VPhysicalVolume* outerPvcBarrelPhysical = new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, outerPvcBarrelZ),
      outerPvcBarrelLogical,
      "OuterPvcBarrelPhysical",
      worldLogical,
      false,
      0,
      true);

  const std::vector<G4double> rearEndCapZPlanes = {
      -17.365 * mm,
      2.635 * mm,
      2.636 * mm,
      17.365 * mm};
  const std::vector<G4double> rearEndCapInnerRadii(
      rearEndCapZPlanes.size(),
      0.0 * mm);
  const std::vector<G4double> rearEndCapOuterRadii = {
      outerPvcOuterRadius,
      outerPvcOuterRadius,
      outerPvcInnerRadius - 0.025 * mm,
      outerPvcInnerRadius - 0.025 * mm};

  G4Polycone* rearEndCapSolid = new G4Polycone(
      "OuterPvcRearEndCapSolid",
      0.0 * deg,
      360.0 * deg,
      static_cast<G4int>(rearEndCapZPlanes.size()),
      rearEndCapZPlanes.data(),
      rearEndCapInnerRadii.data(),
      rearEndCapOuterRadii.data());

  G4LogicalVolume* rearEndCapLogical = new G4LogicalVolume(
      rearEndCapSolid,
      whitePvc,
      "OuterPvcRearEndCapLogical");
  rearEndCapLogical->SetVisAttributes(
      MakeVisAttributes(whitePvcColour));

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, rearEndCapZ),
      rearEndCapLogical,
      "OuterPvcRearEndCapPhysical",
      worldLogical,
      false,
      0,
      true);

  const std::vector<G4double> frontEndCapZPlanes = {
      -15.725 * mm,
      -9.085 * mm,
      -9.075 * mm,
      15.725 * mm};
  const std::vector<G4double> frontEndCapInnerRadii(
      frontEndCapZPlanes.size(),
      0.0 * mm);
  const std::vector<G4double> frontEndCapOuterRadii = {
      outerPvcOuterRadius,
      outerPvcOuterRadius,
      outerPvcOuterRadius,
      19.575 * mm};
  constexpr G4double frontEndCapZ =
      outerPvcBarrelFrontZ + 15.725 * mm;

  G4Polycone* frontEndCapSolid = new G4Polycone(
      "OuterPvcFrontEndCapSolid",
      0.0 * deg,
      360.0 * deg,
      static_cast<G4int>(frontEndCapZPlanes.size()),
      frontEndCapZPlanes.data(),
      frontEndCapInnerRadii.data(),
      frontEndCapOuterRadii.data());

  G4LogicalVolume* frontEndCapLogical = new G4LogicalVolume(
      frontEndCapSolid,
      whitePvc,
      "OuterPvcFrontEndCapLogical");
  frontEndCapLogical->SetVisAttributes(
      MakeVisAttributes(whitePvcColour));

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, frontEndCapZ),
      frontEndCapLogical,
      "OuterPvcFrontEndCapPhysical",
      worldLogical,
      false,
      0,
      true);

  return worldPhysical;
}

G4LogicalVolume* DetectorConstruction::GetScoringVolume() const
{
    return scoringVolume_;
}

G4LogicalVolume* DetectorConstruction::GetPmtPlaneVolume() const
{
  return pmtPlaneVolume_;
}



}
