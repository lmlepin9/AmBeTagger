#pragma once

#include "G4VUserPrimaryGeneratorAction.hh"

class G4Event;
class G4ParticleGun;

namespace AmBeTagger
{
class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction
{
 public:
  PrimaryGeneratorAction();
  ~PrimaryGeneratorAction() override;

  void GeneratePrimaries(G4Event* event) override;

 private:
  G4ParticleGun* particleGun_;
};
}