#include "AmBeTagger/DetectorConstruction.hh"

#include "G4Colour.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalSurface.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Polycone.hh"
#include "G4PolyconeHistorical.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "geomdefs.hh"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace
{
bool CloseEnough(double actual, double expected, double tolerance)
{
  return std::abs(actual - expected) < tolerance;
}

G4VPhysicalVolume* PhysicalVolume(const G4String& name)
{
  return G4PhysicalVolumeStore::GetInstance()->GetVolume(name, false);
}

G4LogicalVolume* LogicalVolume(const G4String& name)
{
  return G4LogicalVolumeStore::GetInstance()->GetVolume(name, false);
}

bool Require(bool condition, const std::string& message)
{
  if (!condition) {
    std::cerr << message << '\n';
    return false;
  }

  return true;
}

bool CheckClose(const std::string& label,
                double actual,
                double expected,
                double tolerance)
{
  if (!CloseEnough(actual, expected, tolerance)) {
    std::cerr << label << " failed: actual = " << actual
              << ", expected = " << expected << '\n';
    return false;
  }

  return true;
}

const G4Tubs* RequireTubs(G4LogicalVolume* logical,
                          const std::string& label)
{
  if (logical == nullptr) {
    std::cerr << label << " logical volume is null\n";
    return nullptr;
  }

  const auto* tubs = dynamic_cast<const G4Tubs*>(logical->GetSolid());
  if (tubs == nullptr) {
    std::cerr << label << " is not backed by a G4Tubs solid\n";
  }

  return tubs;
}

const G4Polycone* RequirePolycone(G4LogicalVolume* logical,
                                  const std::string& label)
{
  if (logical == nullptr) {
    std::cerr << label << " logical volume is null\n";
    return nullptr;
  }

  const auto* polycone = dynamic_cast<const G4Polycone*>(logical->GetSolid());
  if (polycone == nullptr) {
    std::cerr << label << " is not backed by a G4Polycone solid\n";
  }

  return polycone;
}

bool CheckMaterial(G4LogicalVolume* logical,
                   const std::string& label,
                   const G4String& expectedName)
{
  if (logical == nullptr || logical->GetMaterial() == nullptr) {
    std::cerr << label << " material is missing\n";
    return false;
  }

  if (logical->GetMaterial()->GetName() != expectedName) {
    std::cerr << label << " material failed: actual = "
              << logical->GetMaterial()->GetName()
              << ", expected = " << expectedName << '\n';
    return false;
  }

  return true;
}

bool CheckOpticalProperty(const G4Material* material,
                          const char* propertyName,
                          double probeEnergy,
                          double expectedValue,
                          double tolerance)
{
  if (material == nullptr ||
      material->GetMaterialPropertiesTable() == nullptr) {
    std::cerr << "Missing material properties table for "
              << (material == nullptr ? "<null>" : material->GetName())
              << '\n';
    return false;
  }

  auto* property =
      material->GetMaterialPropertiesTable()->GetProperty(propertyName);
  if (property == nullptr) {
    std::cerr << "Missing " << propertyName << " for "
              << material->GetName() << '\n';
    return false;
  }

  if (!CloseEnough(property->Value(probeEnergy),
                   expectedValue,
                   tolerance)) {
    std::cerr << material->GetName() << ' ' << propertyName
              << " failed at " << probeEnergy / eV
              << " eV: actual = " << property->Value(probeEnergy)
              << ", expected = " << expectedValue << '\n';
    return false;
  }

  return true;
}

bool CheckSurfaceProperty(const G4LogicalBorderSurface* borderSurface,
                          const std::string& label,
                          const char* propertyName,
                          double probeEnergy,
                          double expectedValue,
                          double tolerance)
{
  if (borderSurface == nullptr ||
      borderSurface->GetSurfaceProperty() == nullptr) {
    std::cerr << label << " optical surface property is missing\n";
    return false;
  }

  const auto* opticalSurface =
      dynamic_cast<const G4OpticalSurface*>(
          borderSurface->GetSurfaceProperty());
  if (opticalSurface == nullptr ||
      opticalSurface->GetMaterialPropertiesTable() == nullptr) {
    std::cerr << label << " optical material properties are missing\n";
    return false;
  }

  auto* property =
      opticalSurface->GetMaterialPropertiesTable()->GetProperty(propertyName);
  if (property == nullptr) {
    std::cerr << label << " is missing " << propertyName << '\n';
    return false;
  }

  if (!CloseEnough(property->Value(probeEnergy),
                   expectedValue,
                   tolerance)) {
    std::cerr << label << ' ' << propertyName
              << " failed: actual = " << property->Value(probeEnergy)
              << ", expected = " << expectedValue << '\n';
    return false;
  }

  return true;
}

bool CheckVisAttributes(G4LogicalVolume* logical,
                        const std::string& label,
                        const G4Colour& expectedColour)
{
  if (logical == nullptr || logical->GetVisAttributes() == nullptr) {
    std::cerr << label << " visualization attributes are missing\n";
    return false;
  }

  const G4VisAttributes* attributes = logical->GetVisAttributes();
  const G4Colour& colour = attributes->GetColour();

  return Require(attributes->IsVisible(), label + " should be visible") &&
      Require(attributes->IsForceAuxEdgeVisible(),
              label + " should force auxiliary edges") &&
      Require(attributes->IsForceDrawingStyle(),
              label + " should force a drawing style") &&
      Require(attributes->GetForcedDrawingStyle() == G4VisAttributes::solid,
              label + " should force solid drawing") &&
      CheckClose(label + " red", colour.GetRed(), expectedColour.GetRed(),
                 1.0e-12) &&
      CheckClose(label + " green", colour.GetGreen(),
                 expectedColour.GetGreen(), 1.0e-12) &&
      CheckClose(label + " blue", colour.GetBlue(),
                 expectedColour.GetBlue(), 1.0e-12) &&
      CheckClose(label + " alpha", colour.GetAlpha(),
                 expectedColour.GetAlpha(), 1.0e-12);
}

double BackFaceZ(const G4VPhysicalVolume* physical, const G4Tubs* solid)
{
  return physical->GetTranslation().z() - solid->GetZHalfLength();
}

double FrontFaceZ(const G4VPhysicalVolume* physical, const G4Tubs* solid)
{
  return physical->GetTranslation().z() + solid->GetZHalfLength();
}

double PolyconeBackFaceZ(const G4VPhysicalVolume* physical,
                         const G4Polycone* solid)
{
  return physical->GetTranslation().z() +
      solid->GetOriginalParameters()->Z_values[0];
}

double PolyconeFrontFaceZ(const G4VPhysicalVolume* physical,
                          const G4Polycone* solid)
{
  const auto* parameters = solid->GetOriginalParameters();
  return physical->GetTranslation().z() +
      parameters->Z_values[parameters->Num_z_planes - 1];
}

double PolyconePlaneZ(const G4VPhysicalVolume* physical,
                      const G4Polycone* solid,
                      G4int index)
{
  return physical->GetTranslation().z() +
      solid->GetOriginalParameters()->Z_values[index];
}

bool CheckPolyconeProfile(const G4Polycone* solid,
                          const std::string& label,
                          const std::vector<double>& expectedZ,
                          const std::vector<double>& expectedRmin,
                          const std::vector<double>& expectedRmax,
                          double tolerance)
{
  const auto* parameters = solid->GetOriginalParameters();
  const auto expectedSize = static_cast<G4int>(expectedZ.size());

  bool ok = Require(parameters->Num_z_planes == expectedSize,
                    label + " should have expected z-plane count");
  if (!ok) {
    return false;
  }

  for (G4int index = 0; index < expectedSize; ++index) {
    ok &= CheckClose(label + " z plane",
                     parameters->Z_values[index],
                     expectedZ[index],
                     tolerance);
    ok &= CheckClose(label + " inner radius",
                     parameters->Rmin[index],
                     expectedRmin[index],
                     tolerance);
    ok &= CheckClose(label + " outer radius",
                     parameters->Rmax[index],
                     expectedRmax[index],
                     tolerance);
  }

  return ok;
}
}

int main()
{
  AmBeTagger::DetectorConstruction detector;
  G4VPhysicalVolume* worldPhysical = detector.Construct();

  bool ok = true;
  ok &= Require(worldPhysical != nullptr, "World physical volume is null");

  G4LogicalVolume* worldLogical = LogicalVolume("WorldLogical");
  G4LogicalVolume* bgoLogical = LogicalVolume("bgoLogical");
  G4LogicalVolume* couplingLogical = LogicalVolume("CouplingLogical");
  G4LogicalVolume* wrapBarrelLogical =
      LogicalVolume("BgoTeflonBarrelWrapLogical");
  G4LogicalVolume* wrapBackCapLogical =
      LogicalVolume("BgoTeflonBackCapLogical");
  G4LogicalVolume* wrapFrontCapLogical =
      LogicalVolume("BgoTeflonFrontCapLogical");
  G4LogicalVolume* pmtWindowLogical = LogicalVolume("PmtWindowLogical");
  G4LogicalVolume* pmtPlaneLogical = LogicalVolume("PmtPlaneLogical");
  G4LogicalVolume* pmtBodyLogical = LogicalVolume("PmtBodyLogical");
  G4LogicalVolume* pmtFoamLogical = LogicalVolume("PmtFoamSupportLogical");
  G4LogicalVolume* sourceShieldLogical =
      LogicalVolume("SourceDownstreamShieldLogical");
  G4LogicalVolume* sourceCapsuleLogical =
      LogicalVolume("SourceCapsuleLogical");
  G4LogicalVolume* sourceSurroundingShieldLogical =
      LogicalVolume("SourceSurroundingShieldLogical");
  G4LogicalVolume* sourcePvcHolderLogical =
      LogicalVolume("SourcePvcHolderLogical");
  G4LogicalVolume* readoutPvcHolderLogical =
      LogicalVolume("ReadoutPvcHolderLogical");
  G4LogicalVolume* outerPvcBarrelLogical =
      LogicalVolume("OuterPvcBarrelLogical");
  G4LogicalVolume* outerPvcRearEndCapLogical =
      LogicalVolume("OuterPvcRearEndCapLogical");
  G4LogicalVolume* outerPvcFrontEndCapLogical =
      LogicalVolume("OuterPvcFrontEndCapLogical");

  G4VPhysicalVolume* bgoPhysical = PhysicalVolume("bgoPhysical");
  G4VPhysicalVolume* couplingPhysical = PhysicalVolume("CouplingPhysical");
  G4VPhysicalVolume* wrapBarrelPhysical =
      PhysicalVolume("BgoTeflonBarrelWrapPhysical");
  G4VPhysicalVolume* wrapBackCapPhysical =
      PhysicalVolume("BgoTeflonBackCapPhysical");
  G4VPhysicalVolume* wrapFrontCapPhysical =
      PhysicalVolume("BgoTeflonFrontCapPhysical");
  G4VPhysicalVolume* pmtWindowPhysical = PhysicalVolume("PmtWindowPhysical");
  G4VPhysicalVolume* pmtPlanePhysical = PhysicalVolume("PmtPlanePhysical");
  G4VPhysicalVolume* pmtBodyPhysical = PhysicalVolume("PmtBodyPhysical");
  G4VPhysicalVolume* pmtFoamPhysical = PhysicalVolume("PmtFoamSupportPhysical");
  G4VPhysicalVolume* sourceShieldPhysical =
      PhysicalVolume("SourceDownstreamShieldPhysical");
  G4VPhysicalVolume* sourceCapsulePhysical =
      PhysicalVolume("SourceCapsulePhysical");
  G4VPhysicalVolume* sourceSurroundingShieldPhysical =
      PhysicalVolume("SourceSurroundingShieldPhysical");
  G4VPhysicalVolume* sourcePvcHolderPhysical =
      PhysicalVolume("SourcePvcHolderPhysical");
  G4VPhysicalVolume* readoutPvcHolderPhysical =
      PhysicalVolume("ReadoutPvcHolderPhysical");
  G4VPhysicalVolume* outerPvcBarrelPhysical =
      PhysicalVolume("OuterPvcBarrelPhysical");
  G4VPhysicalVolume* outerPvcRearEndCapPhysical =
      PhysicalVolume("OuterPvcRearEndCapPhysical");
  G4VPhysicalVolume* outerPvcFrontEndCapPhysical =
      PhysicalVolume("OuterPvcFrontEndCapPhysical");

  ok &= Require(worldLogical != nullptr, "World logical volume is missing");
  ok &= Require(bgoPhysical != nullptr, "BGO physical volume is missing");
  ok &= Require(couplingPhysical != nullptr,
                "EJ-550 physical volume is missing");
  ok &= Require(wrapBarrelPhysical != nullptr,
                "PTFE barrel wrap physical volume is missing");
  ok &= Require(wrapBackCapPhysical != nullptr,
                "PTFE back cap physical volume is missing");
  ok &= Require(wrapFrontCapPhysical != nullptr,
                "PTFE front cap physical volume is missing");
  ok &= Require(pmtWindowPhysical != nullptr,
                "PMT window physical volume is missing");
  ok &= Require(pmtPlanePhysical != nullptr,
                "PMT plane physical volume is missing");
  ok &= Require(pmtBodyPhysical != nullptr,
                "PMT body physical volume is missing");
  ok &= Require(pmtFoamPhysical != nullptr,
                "PMT foam support physical volume is missing");
  ok &= Require(sourceShieldPhysical != nullptr,
                "Source downstream shield physical volume is missing");
  ok &= Require(sourceCapsulePhysical != nullptr,
                "Source capsule physical volume is missing");
  ok &= Require(sourceSurroundingShieldPhysical != nullptr,
                "Source surrounding shield physical volume is missing");
  ok &= Require(sourcePvcHolderPhysical != nullptr,
                "Source PVC holder physical volume is missing");
  ok &= Require(readoutPvcHolderPhysical != nullptr,
                "Readout PVC holder physical volume is missing");
  ok &= Require(outerPvcBarrelPhysical != nullptr,
                "Outer PVC barrel physical volume is missing");
  ok &= Require(outerPvcRearEndCapPhysical != nullptr,
                "Outer PVC rear end cap physical volume is missing");
  ok &= Require(outerPvcFrontEndCapPhysical != nullptr,
                "Outer PVC front end cap physical volume is missing");

  if (!ok) {
    return 1;
  }

  ok &= !bgoPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !couplingPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !wrapBarrelPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !wrapBackCapPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !wrapFrontCapPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !pmtWindowPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !pmtPlanePhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !pmtBodyPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !pmtFoamPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !sourceShieldPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !sourceCapsulePhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !sourceSurroundingShieldPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !sourcePvcHolderPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !readoutPvcHolderPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !outerPvcBarrelPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !outerPvcRearEndCapPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !outerPvcFrontEndCapPhysical->CheckOverlaps(1000, 0.0, false);

  const G4Tubs* bgoSolid = RequireTubs(bgoLogical, "BGO");
  const G4Tubs* couplingSolid = RequireTubs(couplingLogical, "EJ-550");
  const G4Tubs* wrapBarrelSolid =
      RequireTubs(wrapBarrelLogical, "PTFE barrel wrap");
  const G4Tubs* wrapBackCapSolid =
      RequireTubs(wrapBackCapLogical, "PTFE back cap");
  const G4Tubs* wrapFrontCapSolid =
      RequireTubs(wrapFrontCapLogical, "PTFE front cap");
  const G4Tubs* pmtWindowSolid = RequireTubs(pmtWindowLogical, "PMT window");
  const G4Tubs* pmtPlaneSolid = RequireTubs(pmtPlaneLogical, "PMT plane");
  const G4Tubs* pmtBodySolid = RequireTubs(pmtBodyLogical, "PMT body");
  const G4Tubs* pmtFoamSolid =
      RequireTubs(pmtFoamLogical, "PMT foam support");
  const G4Tubs* sourceShieldSolid =
      RequireTubs(sourceShieldLogical, "Source downstream shield");
  const G4Polycone* sourceCapsuleSolid =
      RequirePolycone(sourceCapsuleLogical, "Source capsule");
  const G4Tubs* sourceSurroundingShieldSolid =
      RequireTubs(sourceSurroundingShieldLogical,
                  "Source surrounding shield");
  const G4Polycone* sourcePvcHolderSolid =
      RequirePolycone(sourcePvcHolderLogical, "Source PVC holder");
  const G4Polycone* readoutPvcHolderSolid =
      RequirePolycone(readoutPvcHolderLogical, "Readout PVC holder");
  const G4Tubs* outerPvcBarrelSolid =
      RequireTubs(outerPvcBarrelLogical, "Outer PVC barrel");
  const G4Polycone* outerPvcRearEndCapSolid =
      RequirePolycone(outerPvcRearEndCapLogical, "Outer PVC rear end cap");
  const G4Polycone* outerPvcFrontEndCapSolid =
      RequirePolycone(outerPvcFrontEndCapLogical, "Outer PVC front end cap");

  if (bgoSolid == nullptr || couplingSolid == nullptr ||
      wrapBarrelSolid == nullptr || wrapBackCapSolid == nullptr ||
      wrapFrontCapSolid == nullptr ||
      pmtWindowSolid == nullptr || pmtPlaneSolid == nullptr ||
      pmtBodySolid == nullptr || pmtFoamSolid == nullptr ||
      sourceShieldSolid == nullptr || sourceCapsuleSolid == nullptr ||
      sourceSurroundingShieldSolid == nullptr ||
      sourcePvcHolderSolid == nullptr ||
      readoutPvcHolderSolid == nullptr ||
      outerPvcBarrelSolid == nullptr ||
      outerPvcRearEndCapSolid == nullptr ||
      outerPvcFrontEndCapSolid == nullptr) {
    return 1;
  }

  constexpr double kTolerance = 1.0e-9 * mm;

  ok &= CheckClose("BGO inner bore radius",
                   bgoSolid->GetInnerRadius(),
                   1.0 * mm,
                   kTolerance);
  ok &= CheckClose("BGO radius",
                   bgoSolid->GetOuterRadius(),
                   25.0 * mm,
                   kTolerance);
  ok &= CheckClose("BGO full length",
                   2.0 * bgoSolid->GetZHalfLength(),
                   50.0 * mm,
                   kTolerance);
  ok &= CheckClose("EJ-550 radius",
                   couplingSolid->GetOuterRadius(),
                   14.25 * mm,
                   kTolerance);
  ok &= CheckClose("EJ-550 full thickness",
                   2.0 * couplingSolid->GetZHalfLength(),
                   0.1 * mm,
                   kTolerance);
  ok &= CheckClose("PTFE barrel wrap inner radius",
                   wrapBarrelSolid->GetInnerRadius(),
                   25.0 * mm,
                   kTolerance);
  ok &= CheckClose("PTFE barrel wrap outer radius",
                   wrapBarrelSolid->GetOuterRadius(),
                   25.25 * mm,
                   kTolerance);
  ok &= CheckClose("PTFE barrel wrap full length",
                   2.0 * wrapBarrelSolid->GetZHalfLength(),
                   50.0 * mm,
                   kTolerance);
  ok &= CheckClose("PTFE back cap outer radius",
                   wrapBackCapSolid->GetOuterRadius(),
                   25.25 * mm,
                   kTolerance);
  ok &= CheckClose("PTFE back cap full thickness",
                   2.0 * wrapBackCapSolid->GetZHalfLength(),
                   0.25 * mm,
                   kTolerance);
  ok &= CheckClose("PTFE front cap inner radius",
                   wrapFrontCapSolid->GetInnerRadius(),
                   14.25 * mm,
                   kTolerance);
  ok &= CheckClose("PTFE front cap outer radius",
                   wrapFrontCapSolid->GetOuterRadius(),
                   25.25 * mm,
                   kTolerance);
  ok &= CheckClose("PTFE front cap full thickness",
                   2.0 * wrapFrontCapSolid->GetZHalfLength(),
                   0.25 * mm,
                   kTolerance);
  ok &= CheckClose("PMT window radius",
                   pmtWindowSolid->GetOuterRadius(),
                   14.25 * mm,
                   kTolerance);
  ok &= CheckClose("PMT window full thickness",
                   2.0 * pmtWindowSolid->GetZHalfLength(),
                   1.0 * mm,
                   kTolerance);
  ok &= CheckClose("PMT plane radius",
                   pmtPlaneSolid->GetOuterRadius(),
                   12.5 * mm,
                   kTolerance);
  ok &= CheckClose("PMT plane full thickness",
                   2.0 * pmtPlaneSolid->GetZHalfLength(),
                   0.01 * mm,
                   kTolerance);
  ok &= CheckClose("PMT body radius",
                   pmtBodySolid->GetOuterRadius(),
                   12.25 * mm,
                   kTolerance);
  ok &= CheckClose("PMT body full length",
                   2.0 * pmtBodySolid->GetZHalfLength(),
                   100.0 * mm,
                   kTolerance);
  ok &= CheckClose("PMT foam support inner radius",
                   pmtFoamSolid->GetInnerRadius(),
                   14.29 * mm,
                   kTolerance);
  ok &= CheckClose("PMT foam support outer radius",
                   pmtFoamSolid->GetOuterRadius(),
                   26.025 * mm,
                   kTolerance);
  ok &= CheckClose("PMT foam support full length",
                   2.0 * pmtFoamSolid->GetZHalfLength(),
                   100.0 * mm,
                   kTolerance);
  ok &= CheckClose("Source downstream shield radius",
                   sourceShieldSolid->GetOuterRadius(),
                   26.025 * mm,
                   kTolerance);
  ok &= CheckClose("Source downstream shield full thickness",
                   2.0 * sourceShieldSolid->GetZHalfLength(),
                   11.63 * mm,
                   kTolerance);
  ok &= CheckClose("Source surrounding shield inner radius",
                   sourceSurroundingShieldSolid->GetInnerRadius(),
                   8.0 * mm,
                   kTolerance);
  ok &= CheckClose("Source surrounding shield outer radius",
                   sourceSurroundingShieldSolid->GetOuterRadius(),
                   sourceShieldSolid->GetOuterRadius(),
                   kTolerance);
  ok &= CheckClose("Source surrounding shield full length",
                   2.0 * sourceSurroundingShieldSolid->GetZHalfLength(),
                   10.0 * mm,
                   kTolerance);

  const auto* capsuleParameters = sourceCapsuleSolid->GetOriginalParameters();
  ok &= Require(capsuleParameters->Num_z_planes == 6,
                "Source capsule should have six z planes");
  if (capsuleParameters->Num_z_planes == 6) {
    const double expectedCapsuleZ[] = {
        -5.0 * mm, -4.0 * mm, -3.9999 * mm,
        3.9999 * mm, 4.0 * mm, 5.0 * mm};
    const double expectedCapsuleRmin[] = {
        0.0 * mm, 0.0 * mm, 6.8 * mm,
        6.8 * mm, 0.0 * mm, 0.0 * mm};

    for (G4int index = 0; index < 6; ++index) {
      ok &= CheckClose("Source capsule z plane",
                       capsuleParameters->Z_values[index],
                       expectedCapsuleZ[index],
                       kTolerance);
      ok &= CheckClose("Source capsule inner radius",
                       capsuleParameters->Rmin[index],
                       expectedCapsuleRmin[index],
                       kTolerance);
      ok &= CheckClose("Source capsule outer radius",
                       capsuleParameters->Rmax[index],
                       7.8 * mm,
                       kTolerance);
    }
  }

  ok &= CheckPolyconeProfile(
      sourcePvcHolderSolid,
      "Source PVC holder",
      {-5.815 * mm, -4.1921 * mm, -4.185 * mm, 5.815 * mm},
      {0.0 * mm, 0.0 * mm, 7.9 * mm, 7.9 * mm},
      {26.025 * mm, 26.025 * mm, 26.025 * mm, 26.025 * mm},
      kTolerance);
  ok &= CheckPolyconeProfile(
      readoutPvcHolderSolid,
      "Readout PVC holder",
      {-60.0 * mm, -40.001 * mm, -40.0 * mm,
       -39.0 * mm, -38.999 * mm, 60.0 * mm},
      {25.0 * mm, 25.0 * mm, 12.79 * mm,
       12.79 * mm, 25.0 * mm, 25.0 * mm},
      {26.025 * mm, 26.025 * mm, 26.025 * mm,
       26.025 * mm, 26.025 * mm, 26.025 * mm},
      kTolerance);
  ok &= CheckClose("Outer PVC barrel inner radius",
                   outerPvcBarrelSolid->GetInnerRadius(),
                   26.025 * mm,
                   kTolerance);
  ok &= CheckClose("Outer PVC barrel outer radius",
                   outerPvcBarrelSolid->GetOuterRadius(),
                   35.235 * mm,
                   kTolerance);
  ok &= CheckClose("Outer PVC barrel full length",
                   2.0 * outerPvcBarrelSolid->GetZHalfLength(),
                   380.0 * mm,
                   kTolerance);
  ok &= CheckPolyconeProfile(
      outerPvcRearEndCapSolid,
      "Outer PVC rear end cap",
      {-17.365 * mm, 2.635 * mm, 2.636 * mm, 17.365 * mm},
      {0.0 * mm, 0.0 * mm, 0.0 * mm, 0.0 * mm},
      {35.235 * mm, 35.235 * mm, 26.0 * mm, 26.0 * mm},
      kTolerance);
  ok &= CheckPolyconeProfile(
      outerPvcFrontEndCapSolid,
      "Outer PVC front end cap",
      {-15.725 * mm, -9.085 * mm, -9.075 * mm, 15.725 * mm},
      {0.0 * mm, 0.0 * mm, 0.0 * mm, 0.0 * mm},
      {35.235 * mm, 35.235 * mm, 35.235 * mm, 19.575 * mm},
      kTolerance);

  ok &= CheckClose("BGO to EJ-550 contact",
                   BackFaceZ(couplingPhysical, couplingSolid),
                   FrontFaceZ(bgoPhysical, bgoSolid),
                   kTolerance);
  ok &= CheckClose("BGO to PTFE back cap contact",
                   FrontFaceZ(wrapBackCapPhysical, wrapBackCapSolid),
                   BackFaceZ(bgoPhysical, bgoSolid),
                   kTolerance);
  ok &= CheckClose("BGO to PTFE front cap contact",
                   BackFaceZ(wrapFrontCapPhysical, wrapFrontCapSolid),
                   FrontFaceZ(bgoPhysical, bgoSolid),
                   kTolerance);
  ok &= CheckClose("EJ-550 to PMT window contact",
                   BackFaceZ(pmtWindowPhysical, pmtWindowSolid),
                   FrontFaceZ(couplingPhysical, couplingSolid),
                   kTolerance);
  ok &= CheckClose("PMT window to PMT plane contact",
                   BackFaceZ(pmtPlanePhysical, pmtPlaneSolid),
                   FrontFaceZ(pmtWindowPhysical, pmtWindowSolid),
                   kTolerance);
  ok &= CheckClose("PMT plane to PMT body contact",
                   BackFaceZ(pmtBodyPhysical, pmtBodySolid),
                   FrontFaceZ(pmtPlanePhysical, pmtPlaneSolid),
                   kTolerance);
  ok &= CheckClose("PMT body and foam support back face alignment",
                   BackFaceZ(pmtFoamPhysical, pmtFoamSolid),
                   BackFaceZ(pmtBodyPhysical, pmtBodySolid),
                   kTolerance);
  ok &= CheckClose("PMT body and foam support front face alignment",
                   FrontFaceZ(pmtFoamPhysical, pmtFoamSolid),
                   FrontFaceZ(pmtBodyPhysical, pmtBodySolid),
                   kTolerance);
  ok &= CheckClose("Source downstream shield to PTFE back cap contact",
                   FrontFaceZ(sourceShieldPhysical, sourceShieldSolid),
                   BackFaceZ(wrapBackCapPhysical, wrapBackCapSolid),
                   kTolerance);
  ok &= CheckClose("Source capsule to downstream shield contact",
                   PolyconeFrontFaceZ(sourceCapsulePhysical,
                                      sourceCapsuleSolid),
                   BackFaceZ(sourceShieldPhysical, sourceShieldSolid),
                   kTolerance);
  ok &= CheckClose("Source surrounding shield to downstream shield contact",
                   FrontFaceZ(sourceSurroundingShieldPhysical,
                              sourceSurroundingShieldSolid),
                   BackFaceZ(sourceShieldPhysical, sourceShieldSolid),
                   kTolerance);
  ok &= CheckClose("Source capsule and surrounding shield back face alignment",
                   PolyconeBackFaceZ(sourceCapsulePhysical,
                                     sourceCapsuleSolid),
                   BackFaceZ(sourceSurroundingShieldPhysical,
                             sourceSurroundingShieldSolid),
                   kTolerance);
  ok &= CheckClose("Source PVC holder to source capsule contact",
                   PolyconeFrontFaceZ(sourcePvcHolderPhysical,
                                      sourcePvcHolderSolid),
                   PolyconeBackFaceZ(sourceCapsulePhysical,
                                     sourceCapsuleSolid),
                   kTolerance);
  ok &= CheckClose("Readout PVC holder to PMT body contact",
                   PolyconeBackFaceZ(readoutPvcHolderPhysical,
                                     readoutPvcHolderSolid),
                   FrontFaceZ(pmtBodyPhysical, pmtBodySolid),
                   kTolerance);
  ok &= CheckClose("Outer PVC rear end cap insertion to barrel contact",
                   PolyconePlaneZ(outerPvcRearEndCapPhysical,
                                  outerPvcRearEndCapSolid,
                                  2),
                   BackFaceZ(outerPvcBarrelPhysical, outerPvcBarrelSolid),
                   kTolerance);
  ok &= CheckClose("Outer PVC rear end cap to source PVC holder contact",
                   PolyconeFrontFaceZ(outerPvcRearEndCapPhysical,
                                      outerPvcRearEndCapSolid),
                   PolyconeBackFaceZ(sourcePvcHolderPhysical,
                                     sourcePvcHolderSolid),
                   kTolerance);
  ok &= Require(FrontFaceZ(outerPvcBarrelPhysical, outerPvcBarrelSolid) >
                    PolyconeFrontFaceZ(readoutPvcHolderPhysical,
                                       readoutPvcHolderSolid),
                "Outer PVC barrel should extend past the readout PVC holder");
  ok &= Require(BackFaceZ(outerPvcBarrelPhysical, outerPvcBarrelSolid) <
                    PolyconeBackFaceZ(sourcePvcHolderPhysical,
                                      sourcePvcHolderSolid),
                "Outer PVC barrel should start before the source PVC holder");
  ok &= CheckClose("Outer PVC front end cap to barrel contact",
                   PolyconeBackFaceZ(outerPvcFrontEndCapPhysical,
                                     outerPvcFrontEndCapSolid),
                   FrontFaceZ(outerPvcBarrelPhysical, outerPvcBarrelSolid),
                   kTolerance);

  ok &= CheckMaterial(bgoLogical, "BGO", "BGO");
  ok &= CheckMaterial(couplingLogical, "EJ-550", "EJ550OpticalGrease");
  ok &= CheckMaterial(wrapBarrelLogical, "PTFE barrel wrap", "G4_TEFLON");
  ok &= CheckMaterial(wrapBackCapLogical, "PTFE back cap", "G4_TEFLON");
  ok &= CheckMaterial(wrapFrontCapLogical, "PTFE front cap", "G4_TEFLON");
  ok &= CheckMaterial(pmtWindowLogical, "PMT window", "UVGlass");
  ok &= CheckMaterial(pmtPlaneLogical, "PMT plane", "G4_AIR");
  ok &= CheckMaterial(pmtBodyLogical, "PMT body", "BorosilicateGlass");
  ok &= CheckMaterial(pmtFoamLogical, "PMT foam support", "PolyurethaneFoam");
  ok &= CheckMaterial(sourceShieldLogical,
                      "Source downstream shield",
                      "StainlessSteel");
  ok &= CheckMaterial(sourceCapsuleLogical,
                      "Source capsule",
                      "StainlessSteel");
  ok &= CheckMaterial(sourceSurroundingShieldLogical,
                      "Source surrounding shield",
                      "StainlessSteel");
  ok &= CheckClose("Stainless steel density",
                   sourceShieldLogical->GetMaterial()->GetDensity(),
                   8.0 * g / cm3,
                   1.0e-12 * g / cm3);
  ok &= CheckMaterial(sourcePvcHolderLogical, "Source PVC holder", "WhitePVC");
  ok &= CheckMaterial(readoutPvcHolderLogical,
                      "Readout PVC holder",
                      "WhitePVC");
  ok &= CheckMaterial(outerPvcBarrelLogical,
                      "Outer PVC barrel",
                      "WhitePVC");
  ok &= CheckMaterial(outerPvcRearEndCapLogical,
                      "Outer PVC rear end cap",
                      "WhitePVC");
  ok &= CheckMaterial(outerPvcFrontEndCapLogical,
                      "Outer PVC front end cap",
                      "WhitePVC");
  ok &= CheckClose("White PVC density",
                   sourcePvcHolderLogical->GetMaterial()->GetDensity(),
                   1.38 * g / cm3,
                   1.0e-12 * g / cm3);

  ok &= CheckOpticalProperty(couplingLogical->GetMaterial(),
                             "RINDEX",
                             3.0 * eV,
                             1.46,
                             1.0e-12);
  ok &= CheckOpticalProperty(couplingLogical->GetMaterial(),
                             "ABSLENGTH",
                             3.0 * eV,
                             10.0 * m,
                             1.0e-9 * m);
  ok &= CheckOpticalProperty(pmtWindowLogical->GetMaterial(),
                             "RINDEX",
                             3.0 * eV,
                             1.49,
                             1.0e-12);
  ok &= CheckOpticalProperty(pmtWindowLogical->GetMaterial(),
                             "ABSLENGTH",
                             3.0 * eV,
                             10.0 * m,
                             1.0e-9 * m);
  ok &= CheckOpticalProperty(pmtBodyLogical->GetMaterial(),
                             "RINDEX",
                             3.0 * eV,
                             1.49,
                             1.0e-12);
  ok &= CheckOpticalProperty(pmtBodyLogical->GetMaterial(),
                             "ABSLENGTH",
                             3.0 * eV,
                             10.0 * m,
                             1.0e-9 * m);

  ok &= Require(worldLogical->GetVisAttributes() != nullptr &&
                    !worldLogical->GetVisAttributes()->IsVisible(),
                "World volume should be invisible");
  ok &= CheckVisAttributes(bgoLogical,
                           "BGO",
                           G4Colour(0.05, 0.25, 1.0, 0.38));
  ok &= CheckVisAttributes(couplingLogical,
                           "EJ-550",
                           G4Colour(1.0, 0.82, 0.05, 0.55));
  ok &= CheckVisAttributes(wrapBarrelLogical,
                           "PTFE barrel wrap",
                           G4Colour(0.92, 0.92, 0.86, 0.72));
  ok &= CheckVisAttributes(wrapBackCapLogical,
                           "PTFE back cap",
                           G4Colour(0.92, 0.92, 0.86, 0.72));
  ok &= CheckVisAttributes(wrapFrontCapLogical,
                           "PTFE front cap",
                           G4Colour(0.92, 0.92, 0.86, 0.72));
  ok &= CheckVisAttributes(pmtWindowLogical,
                           "PMT window",
                           G4Colour(0.0, 0.82, 0.95, 0.45));
  ok &= CheckVisAttributes(pmtPlaneLogical,
                           "PMT plane",
                           G4Colour(0.0, 0.95, 0.28, 0.9));
  ok &= CheckVisAttributes(pmtBodyLogical,
                           "PMT body",
                           G4Colour(0.18, 0.22, 0.28, 0.42));
  ok &= CheckVisAttributes(pmtFoamLogical,
                           "PMT foam support",
                           G4Colour(0.04, 0.045, 0.055, 0.28));
  ok &= CheckVisAttributes(sourceShieldLogical,
                           "Source downstream shield",
                           G4Colour(0.58, 0.60, 0.63, 0.76));
  ok &= CheckVisAttributes(sourceCapsuleLogical,
                           "Source capsule",
                           G4Colour(0.58, 0.60, 0.63, 0.76));
  ok &= CheckVisAttributes(sourceSurroundingShieldLogical,
                           "Source surrounding shield",
                           G4Colour(0.58, 0.60, 0.63, 0.76));
  ok &= CheckVisAttributes(sourcePvcHolderLogical,
                           "Source PVC holder",
                           G4Colour(0.96, 0.96, 0.90, 0.30));
  ok &= CheckVisAttributes(readoutPvcHolderLogical,
                           "Readout PVC holder",
                           G4Colour(0.96, 0.96, 0.90, 0.30));
  ok &= CheckVisAttributes(outerPvcBarrelLogical,
                           "Outer PVC barrel",
                           G4Colour(0.96, 0.96, 0.90, 0.30));
  ok &= CheckVisAttributes(outerPvcRearEndCapLogical,
                           "Outer PVC rear end cap",
                           G4Colour(0.96, 0.96, 0.90, 0.30));
  ok &= CheckVisAttributes(outerPvcFrontEndCapLogical,
                           "Outer PVC front end cap",
                           G4Colour(0.96, 0.96, 0.90, 0.30));

  ok &= Require(detector.GetPmtPlaneVolume() == pmtPlaneLogical,
                "Detector should still expose the PMT plane as the readout volume");

  ok &= Require(bgoSolid->Inside(G4ThreeVector(0.5 * mm, 0.0, 0.0))
                    == kOutside,
                "Point inside the central BGO bore was accepted");
  ok &= Require(bgoSolid->Inside(G4ThreeVector(1.5 * mm, 0.0, 0.0))
                    != kOutside,
                "Point inside the BGO annulus was rejected");
  ok &= Require(bgoSolid->Inside(G4ThreeVector(25.5 * mm, 0.0, 0.0))
                    == kOutside,
                "Point outside the BGO outer radius was accepted");

  ok &= Require(pmtPlaneSolid->Inside(G4ThreeVector(12.49 * mm, 0.0, 0.0))
                    != kOutside,
                "Point inside the PMT aperture was rejected");
  ok &= Require(pmtPlaneSolid->Inside(G4ThreeVector(12.51 * mm, 0.0, 0.0))
                    == kOutside,
                "Point outside the PMT aperture was accepted");
  ok &= Require(pmtFoamSolid->Inside(G4ThreeVector(14.0 * mm, 0.0, 0.0))
                    == kOutside,
                "Point inside the PMT foam support bore was accepted");
  ok &= Require(pmtFoamSolid->Inside(G4ThreeVector(20.0 * mm, 0.0, 0.0))
                    != kOutside,
                "Point inside the PMT foam support annulus was rejected");
  ok &= Require(pmtFoamSolid->Inside(G4ThreeVector(26.5 * mm, 0.0, 0.0))
                    == kOutside,
                "Point outside the PMT foam support was accepted");
  ok &= Require(sourceCapsuleSolid->Inside(G4ThreeVector(7.7 * mm, 0.0,
                                                         0.0))
                    != kOutside,
                "Point inside the source capsule end cap was rejected");
  ok &= Require(sourceCapsuleSolid->Inside(G4ThreeVector(6.7 * mm, 0.0,
                                                         -3.0 * mm))
                    == kOutside,
                "Point inside the source capsule hollow region was accepted");
  ok &= Require(sourceCapsuleSolid->Inside(G4ThreeVector(7.2 * mm, 0.0,
                                                         -3.0 * mm))
                    != kOutside,
                "Point inside the source capsule shell was rejected");
  ok &= Require(sourceSurroundingShieldSolid->Inside(
                    G4ThreeVector(7.9 * mm, 0.0, 0.0))
                    == kOutside,
                "Point inside the source surrounding shield bore was accepted");
  ok &= Require(sourceSurroundingShieldSolid->Inside(
                    G4ThreeVector(20.0 * mm, 0.0, 0.0))
                    != kOutside,
                "Point inside the source surrounding shield annulus was rejected");
  ok &= Require(sourcePvcHolderSolid->Inside(G4ThreeVector(4.0 * mm, 0.0,
                                                           -5.0 * mm))
                    != kOutside,
                "Point inside the source PVC holder closed disk was rejected");
  ok &= Require(sourcePvcHolderSolid->Inside(G4ThreeVector(7.8 * mm, 0.0,
                                                           0.0))
                    == kOutside,
                "Point inside the source PVC holder opening was accepted");
  ok &= Require(readoutPvcHolderSolid->Inside(G4ThreeVector(20.0 * mm, 0.0,
                                                            -39.5 * mm))
                    != kOutside,
                "Point inside the readout PVC holder support lip was rejected");
  ok &= Require(readoutPvcHolderSolid->Inside(G4ThreeVector(24.0 * mm, 0.0,
                                                            0.0))
                    == kOutside,
                "Point inside the readout PVC holder main bore was accepted");

  const auto* barrelWrapSurface =
      G4LogicalBorderSurface::GetSurface(bgoPhysical, wrapBarrelPhysical);
  const auto* backCapSurface =
      G4LogicalBorderSurface::GetSurface(bgoPhysical, wrapBackCapPhysical);
  const auto* frontCapSurface =
      G4LogicalBorderSurface::GetSurface(bgoPhysical, wrapFrontCapPhysical);
  ok &= Require(barrelWrapSurface != nullptr,
                "BGO-to-PTFE barrel optical surface is missing");
  ok &= Require(backCapSurface != nullptr,
                "BGO-to-PTFE back cap optical surface is missing");
  ok &= Require(frontCapSurface != nullptr,
                "BGO-to-PTFE front cap optical surface is missing");
  ok &= CheckSurfaceProperty(barrelWrapSurface,
                             "BGO-to-PTFE barrel optical surface",
                             "REFLECTIVITY",
                             3.0 * eV,
                             0.9,
                             1.0e-12);
  ok &= CheckSurfaceProperty(backCapSurface,
                             "BGO-to-PTFE back cap optical surface",
                             "REFLECTIVITY",
                             3.0 * eV,
                             0.9,
                             1.0e-12);
  ok &= CheckSurfaceProperty(frontCapSurface,
                             "BGO-to-PTFE front cap optical surface",
                             "REFLECTIVITY",
                             3.0 * eV,
                             0.9,
                             1.0e-12);

  if (!ok) {
    return 1;
  }

  std::cout << "Detector geometry checks passed\n";
  return 0;
}
