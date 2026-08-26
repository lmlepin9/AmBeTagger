#include "AmBeTagger/DetectorConstruction.hh"
#include "G4Box.hh"
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

namespace AmBeTagger
{
G4VPhysicalVolume* DetectorConstruction::Construct()
{
  G4NistManager* nist = G4NistManager::Instance();

  G4Material* air = nist->FindOrBuildMaterial("G4_AIR");
  G4Element* bismuth = nist->FindOrBuildElement("Bi");
  G4Element* germanium = nist->FindOrBuildElement("Ge");
  G4Element* oxygen = nist->FindOrBuildElement("O");

  constexpr G4double bgoDensity = 7.13 * g / cm3; 

  G4Material* bgo = new G4Material(
    "BGO",
    bgoDensity,
    3);

  bgo->AddElement(bismuth,4);
  bgo->AddElement(germanium,3);
  bgo->AddElement(oxygen,12);


  // Configure optical properties here. For now use a simplified version! 
  const std::vector<G4double> photonEnergy = {
    2.48 * eV,
    2.76 * eV,
    3.10 * eV};

  const std::vector<G4double> bgoRefractiveIndex = {
      2.15,
      2.15,
      2.15};

  const std::vector<G4double> bgoScintillation = {
      0.2,
      1.0,
      0.2};

  const std::vector<G4double> bgoAbsorptionLength = {
    20.0 * cm,
    20.0 * cm,
    20.0 * cm};


  G4MaterialPropertiesTable* bgoMpt = new G4MaterialPropertiesTable;


  bgoMpt->AddProperty("ABSLENGTH", photonEnergy, bgoAbsorptionLength);
  bgoMpt->AddProperty("RINDEX", photonEnergy, bgoRefractiveIndex);
  bgoMpt->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, bgoScintillation);

  bgoMpt->AddConstProperty("SCINTILLATIONYIELD", 1000. / MeV);
  bgoMpt->AddConstProperty("RESOLUTIONSCALE", 2.0);
  bgoMpt->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 300.0 * ns);
  bgoMpt->AddConstProperty("SCINTILLATIONYIELD1", 1.0);

  bgo->SetMaterialPropertiesTable(bgoMpt);



  const std::vector<G4double> airRefractiveIndex = {
      1.0,
      1.0,
      1.0};

  G4MaterialPropertiesTable* airMpt = new G4MaterialPropertiesTable;
  airMpt->AddProperty("RINDEX", photonEnergy, airRefractiveIndex);
  air->SetMaterialPropertiesTable(airMpt);

  G4Material* opticalCoupling = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
  const std::vector<G4double> couplingRefractiveIndex = {
      1.50,
      1.50,
      1.50};

  G4MaterialPropertiesTable* couplingMpt = new G4MaterialPropertiesTable;
  couplingMpt->AddProperty("RINDEX", photonEnergy, couplingRefractiveIndex);
  opticalCoupling->SetMaterialPropertiesTable(couplingMpt);


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


   constexpr G4double bgoMaxRadius = 2.5 * cm;
   constexpr G4double bgoMinRadius = 0.0 * cm; // I need to double check the size of the extrusion, for testing I removed this. 
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

  G4VPhysicalVolume* bgoPhysical = new G4PVPlacement(
    nullptr,
    G4ThreeVector{},
    scoringVolume_,
    "bgoPhysical",
    worldLogical,
    false,
    0,
    true);

  G4VPhysicalVolume* worldPhysical = new G4PVPlacement(
    nullptr,
    G4ThreeVector{},
    worldLogical,
    "WorldPhysical",
    nullptr,
    false,
    0,
    true);


  const std::vector<G4double> wrapReflectivity = {
    0.98,
    0.98,
    0.98};

  const std::vector<G4double> wrapEfficiency = {
    0.0,
    0.0,
    0.0};

  G4MaterialPropertiesTable* wrapMpt = new G4MaterialPropertiesTable;
  wrapMpt->AddProperty("REFLECTIVITY", photonEnergy, wrapReflectivity);
  wrapMpt->AddProperty("EFFICIENCY", photonEnergy, wrapEfficiency);

  G4OpticalSurface* wrapSurface = new G4OpticalSurface("BgoWrapSurface");
  wrapSurface->SetType(dielectric_metal);
  wrapSurface->SetFinish(ground);
  wrapSurface->SetModel(unified);
  wrapSurface->SetMaterialPropertiesTable(wrapMpt);

  new G4LogicalBorderSurface(
      "BgoToAirWrapSurface",
      bgoPhysical,
      worldPhysical,
      wrapSurface);


  constexpr G4double couplingRadius = 2.5 * cm;
  constexpr G4double couplingHalfThickness = 0.5 * mm;
  constexpr G4double couplingZ = 2.55 * cm;

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

  new G4PVPlacement(
      nullptr,
      G4ThreeVector(0.0 * cm, 0.0 * cm, couplingZ),
      couplingLogical,
      "CouplingPhysical",
      worldLogical,
      false,
      0,
      true);

constexpr G4double pmtPlaneRadius = 2.5 * cm;
constexpr G4double pmtPlaneHalfThickness = 0.05 * mm;
constexpr G4double pmtPlaneZ = 2.61 * cm;

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

new G4PVPlacement(
    nullptr,
    G4ThreeVector(0.0 * cm, 0.0 * cm, pmtPlaneZ),
    pmtPlaneVolume_,
    "PmtPlanePhysical",
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