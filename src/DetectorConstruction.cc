#include "AmBeTagger/DetectorConstruction.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

namespace AmBeTagger
{
G4VPhysicalVolume* DetectorConstruction::Construct()
{
  G4NistManager* nist = G4NistManager::Instance();

  G4Material* air = nist->FindOrBuildMaterial("G4_AIR");

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


   G4Material* water = nist->FindOrBuildMaterial("G4_WATER");
   constexpr G4double testHalfLength = 5.0 * cm;

   G4Box* testSolid = new G4Box(
    "TestSolid",
    testHalfLength,
    testHalfLength,
    testHalfLength);

   scoringVolume_ = new G4LogicalVolume(
        testSolid,
        water,
        "TestLogical");

   // Physical placement of the water cube 
   new G4PVPlacement(
    nullptr,          // no rotation
    G4ThreeVector{},  // position: (0, 0, 0)
    scoringVolume_,   // logical volume being placed
    "TestPhysical",   // name of this physical placement
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