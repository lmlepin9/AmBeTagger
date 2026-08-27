#include "AmBeTagger/PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

namespace AmBeTagger
{
PrimaryGeneratorAction::PrimaryGeneratorAction()
    : particleGun_(new G4ParticleGun(1))
{
  particleGun_->SetParticleDefinition(G4Gamma::Definition());
  particleGun_->SetParticleEnergy(1.0 * MeV);
  particleGun_->SetParticlePosition(G4ThreeVector(0.0 * cm, 0.5 * cm, -10.0 * cm));
  particleGun_->SetParticleMomentumDirection(G4ThreeVector(0.0, 0.0, 1.0));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete particleGun_;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  particleGun_->GeneratePrimaryVertex(event);
}
}