cmake_minimum_required(VERSION 3.20)
project(AmBeTagger LANGUAGES CXX)
find_package(Geant4 REQUIRED ui_all vis_all)
add_executable(ambe_tagger
  src/main.cc
  src/DetectorConstruction.cc
  src/ActionInitialization.cc
  src/PrimaryGeneratorAction.cc
  src/EventAction.cc
  src/SteppingAction.cc
  src/RunAction.cc
)
target_compile_features(ambe_tagger PRIVATE cxx_std_17)
target_include_directories(ambe_tagger PRIVATE include)
target_link_libraries(ambe_tagger PRIVATE ${Geant4_LIBRARIES})

set(AMBETAGGER_MACROS
  macros/run_one.mac
  macros/init_vis.mac
  macros/run_10_gamma.mac
)

foreach(_macro ${AMBETAGGER_MACROS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_macro}
    ${PROJECT_BINARY_DIR}/${_macro}
    COPYONLY
  )
endforeach()