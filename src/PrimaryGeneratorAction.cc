#include "AmBeTagger/PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleTable.hh"
#include "G4SingleParticleSource.hh"
#include "G4SPSAngDistribution.hh"
#include "G4SPSEneDistribution.hh"
#include "G4SPSPosDistribution.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

namespace AmBeTagger
{
PrimaryGeneratorAction::PrimaryGeneratorAction()
    : particleSource_(new G4GeneralParticleSource)
{
  G4SingleParticleSource* currentSource = particleSource_->GetCurrentSource();
  particleSource_->SetParticleDefinition(
      G4ParticleTable::GetParticleTable()->FindParticle("gamma"));
  currentSource->GetPosDist()->SetPosDisType("Point");
  currentSource->GetPosDist()->SetCentreCoords(
      G4ThreeVector(0.0 * cm, 0.5 * cm, -10.0 * cm));
  currentSource->GetAngDist()->SetAngDistType("planar");
  currentSource->GetAngDist()->SetParticleMomentumDirection(
      G4ThreeVector(0.0, 0.0, 1.0));
  currentSource->GetEneDist()->SetEnergyDisType("Mono");
  currentSource->GetEneDist()->SetMonoEnergy(4.4 * MeV);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete particleSource_;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  particleSource_->GeneratePrimaryVertex(event);
}
}
