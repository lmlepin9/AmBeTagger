#pragma once

#include "G4VUserPrimaryGeneratorAction.hh"

class G4Event;
class G4GeneralParticleSource;

namespace AmBeTagger
{
class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction
{
 public:
  PrimaryGeneratorAction();
  ~PrimaryGeneratorAction() override;

  void GeneratePrimaries(G4Event* event) override;

 private:
  G4GeneralParticleSource* particleSource_;
};
}
