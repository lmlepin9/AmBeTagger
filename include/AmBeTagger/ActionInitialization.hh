#pragma once

#include "G4VUserActionInitialization.hh"

namespace AmBeTagger
{
class DetectorConstruction;

class ActionInitialization final : public G4VUserActionInitialization
{
 public:
  explicit ActionInitialization(const DetectorConstruction* detectorConstruction);
  ~ActionInitialization() override = default;

  void Build() const override;

 private:
  const DetectorConstruction* detectorConstruction_;
};
}