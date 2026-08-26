#include "AmBeTagger/ActionInitialization.hh"
#include "AmBeTagger/DetectorConstruction.hh"

#include "FTFP_BERT.hh"
#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4Version.hh"
#include "G4VisExecutive.hh"
#include "G4ios.hh"
#include "G4OpticalPhysics.hh"

#include <memory>

int main(int argc, char** argv)
{
    G4UIExecutive* ui = nullptr;
    if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
    }

    std::unique_ptr<G4RunManager> runManager{
    G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial)};
    // Use G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default) for Multi-thread mode! 


    AmBeTagger::DetectorConstruction* detectorConstruction =
    new AmBeTagger::DetectorConstruction;

    runManager->SetUserInitialization(detectorConstruction);

    // We configure optical properties here
    FTFP_BERT* physicsList = new FTFP_BERT;
    physicsList->RegisterPhysics(new G4OpticalPhysics);
    runManager->SetUserInitialization(physicsList);


    runManager->SetUserInitialization(
        new AmBeTagger::ActionInitialization(detectorConstruction));

    G4cout << "AmBeTagger minimal Geant4 application" << G4endl;
    G4cout << "Geant4 version: " << G4Version << G4endl;

    runManager->Initialize();

    G4VisExecutive visManager;
    visManager.Initialize();

    G4UImanager* uiManager = G4UImanager::GetUIpointer();

    if (ui != nullptr) {
        uiManager->ApplyCommand("/control/execute macros/init_vis.mac");
        ui->SessionStart();
        delete ui;
    } else {
        G4String command = "/control/execute ";
        G4String macroFile = argv[1];
        uiManager->ApplyCommand(command + macroFile);
    }

    return 0;
}