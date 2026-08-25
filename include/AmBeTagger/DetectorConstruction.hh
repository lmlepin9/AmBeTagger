#pragma once

#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;
class G4LogicalVolume; 

namespace AmBeTagger
{
class DetectorConstruction final : public G4VUserDetectorConstruction
{
 public:
  G4LogicalVolume* GetScoringVolume() const; 
    DetectorConstruction() = default;
    ~DetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;

  private: 
    G4LogicalVolume* scoringVolume_ = nullptr; 
};
}