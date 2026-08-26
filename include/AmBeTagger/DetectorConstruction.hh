#pragma once

#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;
class G4LogicalVolume; 

namespace AmBeTagger
{
class DetectorConstruction final : public G4VUserDetectorConstruction
{
 public:
    DetectorConstruction() = default;
    ~DetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;
    G4LogicalVolume* GetScoringVolume() const; 
    G4LogicalVolume* GetPmtPlaneVolume() const;

  private: 
    G4LogicalVolume* scoringVolume_ = nullptr; 
    G4LogicalVolume* pmtPlaneVolume_ = nullptr;

};
}