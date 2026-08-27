#include "AmBeTagger/TrackingAction.hh"

#include "FTFP_BERT.hh"
#include "G4DynamicParticle.hh"
#include "G4Gamma.hh"
#include "G4OpticalPhoton.hh"
#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"

#include <iostream>
#include <memory>
#include <string>

namespace
{
std::unique_ptr<G4Track> MakeTrack(const G4ParticleDefinition* particle,
                                   G4double energy)
{
  auto* dynamicParticle =
      new G4DynamicParticle(particle, G4ThreeVector(0.0, 0.0, 1.0), energy);
  return std::make_unique<G4Track>(
      dynamicParticle,
      0.0,
      G4ThreeVector());
}

bool RequireEqual(G4int actual, G4int expected, const std::string& message)
{
  if (actual != expected) {
    std::cerr << message << ": actual = " << actual
              << ", expected = " << expected << '\n';
    return false;
  }

  return true;
}
}

int main()
{
  std::unique_ptr<G4RunManager> runManager{
      G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial)};
  runManager->SetUserInitialization(new FTFP_BERT);

  G4TrackingManager trackingManager;
  AmBeTagger::TrackingAction action(2);
  action.SetTrackingManagerPointer(&trackingManager);

  bool ok = true;
  trackingManager.SetStoreTrajectory(1);

  std::unique_ptr<G4Track> firstOptical =
      MakeTrack(G4OpticalPhoton::Definition(), 2.76 * eV);
  action.PreUserTrackingAction(firstOptical.get());
  ok &= RequireEqual(trackingManager.GetStoreTrajectory(),
                     1,
                     "First optical photon should remain visible");
  action.PostUserTrackingAction(firstOptical.get());
  ok &= RequireEqual(trackingManager.GetStoreTrajectory(),
                     1,
                     "First optical photon post action should preserve state");

  std::unique_ptr<G4Track> secondOptical =
      MakeTrack(G4OpticalPhoton::Definition(), 2.76 * eV);
  action.PreUserTrackingAction(secondOptical.get());
  ok &= RequireEqual(trackingManager.GetStoreTrajectory(),
                     1,
                     "Second optical photon should remain visible");
  action.PostUserTrackingAction(secondOptical.get());
  ok &= RequireEqual(trackingManager.GetStoreTrajectory(),
                     1,
                     "Second optical photon post action should preserve state");

  std::unique_ptr<G4Track> thirdOptical =
      MakeTrack(G4OpticalPhoton::Definition(), 2.76 * eV);
  action.PreUserTrackingAction(thirdOptical.get());
  ok &= RequireEqual(trackingManager.GetStoreTrajectory(),
                     0,
                     "Third optical photon should be hidden by the quota");
  action.PostUserTrackingAction(thirdOptical.get());
  ok &= RequireEqual(trackingManager.GetStoreTrajectory(),
                     1,
                     "Suppressed optical photon should restore prior state");

  std::unique_ptr<G4Track> gammaTrack =
      MakeTrack(G4Gamma::Definition(), 1.0 * MeV);
  action.PreUserTrackingAction(gammaTrack.get());
  ok &= RequireEqual(trackingManager.GetStoreTrajectory(),
                     1,
                     "Non-optical tracks should not be suppressed");
  action.PostUserTrackingAction(gammaTrack.get());
  ok &= RequireEqual(trackingManager.GetStoreTrajectory(),
                     1,
                     "Non-optical post action should preserve state");

  AmBeTagger::TrackingAction zeroQuotaAction(0);
  zeroQuotaAction.SetTrackingManagerPointer(&trackingManager);
  trackingManager.SetStoreTrajectory(1);

  std::unique_ptr<G4Track> hiddenOptical =
      MakeTrack(G4OpticalPhoton::Definition(), 2.76 * eV);
  zeroQuotaAction.PreUserTrackingAction(hiddenOptical.get());
  ok &= RequireEqual(trackingManager.GetStoreTrajectory(),
                     0,
                     "Zero quota should hide the first optical photon");
  zeroQuotaAction.PostUserTrackingAction(hiddenOptical.get());
  ok &= RequireEqual(trackingManager.GetStoreTrajectory(),
                     1,
                     "Zero quota should restore prior trajectory state");

  if (!ok) {
    return 1;
  }

  std::cout << "TrackingAction trajectory quota checks passed\n";
  return 0;
}
