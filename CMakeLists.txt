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
  src/PmtResponse.cc
  src/WaveformBuilder.cc
  src/SinglePhotoelectronPulse.cc
  src/SinglePhotoelectronGain.cc
  src/WaveformObservables.cc
)

target_compile_features(ambe_tagger PRIVATE cxx_std_17)
target_include_directories(ambe_tagger PRIVATE include)
target_link_libraries(ambe_tagger PRIVATE ${Geant4_LIBRARIES})

add_executable(check_pmt_response
  tests/check_pmt_response.cc
  src/PmtResponse.cc
)

target_compile_features(check_pmt_response PRIVATE cxx_std_17)

target_include_directories(check_pmt_response PRIVATE include)

target_link_libraries(check_pmt_response PRIVATE ${Geant4_LIBRARIES})

add_executable(check_waveform_builder
  tests/check_waveform_builder.cc
  src/WaveformBuilder.cc
  src/SinglePhotoelectronPulse.cc
  src/SinglePhotoelectronGain.cc
)

target_compile_features(check_waveform_builder PRIVATE cxx_std_17)

target_include_directories(check_waveform_builder PRIVATE include)

target_link_libraries(check_waveform_builder PRIVATE ${Geant4_LIBRARIES})

add_executable(check_single_photoelectron_pulse
  tests/check_single_photoelectron_pulse.cc
  src/SinglePhotoelectronPulse.cc
)

target_compile_features(check_single_photoelectron_pulse PRIVATE cxx_std_17)

target_include_directories(check_single_photoelectron_pulse PRIVATE include)

target_link_libraries(check_single_photoelectron_pulse PRIVATE ${Geant4_LIBRARIES})

add_executable(check_single_photoelectron_gain
  tests/check_single_photoelectron_gain.cc
  src/SinglePhotoelectronGain.cc
)

target_compile_features(check_single_photoelectron_gain PRIVATE cxx_std_17)

target_include_directories(check_single_photoelectron_gain PRIVATE include)

target_link_libraries(check_single_photoelectron_gain PRIVATE ${Geant4_LIBRARIES})

add_executable(check_waveform_observables
  tests/check_waveform_observables.cc
  src/WaveformObservables.cc
)

target_compile_features(check_waveform_observables PRIVATE cxx_std_17)

target_include_directories(check_waveform_observables PRIVATE include)

target_link_libraries(check_waveform_observables PRIVATE ${Geant4_LIBRARIES})

set(AMBETAGGER_MACROS
  macros/run_one.mac
  macros/init_vis.mac
  macros/run_10_gamma.mac
  macros/run_1000_gamma.mac
  macros/run_100_optical_photons.mac
  macros/optical_backward.mac
  macros/optical_forward.mac
  macros/optical_sideways.mac
)

foreach(_macro ${AMBETAGGER_MACROS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_macro}
    ${PROJECT_BINARY_DIR}/${_macro}
    COPYONLY
  )
endforeach()
