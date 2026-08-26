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


    /*
   G4Box* testSolid = new G4Box(
    "TestSolid",
    testHalfLength,
    testHalfLength,
    testHalfLength);
    */ 

   scoringVolume_ = new G4LogicalVolume(
        bgoSolid,
        bgo,
        "bgoLogical");

   // Physical placement of the water cube 
   new G4PVPlacement(
    nullptr,          // no rotation
    G4ThreeVector{},  // position: (0, 0, 0)
    scoringVolume_,   // logical volume being placed
    "bgoPhysical",   // name of this physical placement
    worldLogical,     // mother logical volume
    false,            // not using boolean-volume placement mode
    0,                // copy number
    true);            // check for overlaps

  G4VPhysicalVolume* worldPhysical = new G4PVPlacement(
      nullptr,
      G4ThreeVector{},
      worldLogical,
      "WorldPhysical",
      nullptr,
      false,
      0,
      true);

  return worldPhysical;
}

G4LogicalVolume* DetectorConstruction::GetScoringVolume() const
{
    return scoringVolume_;
}


}