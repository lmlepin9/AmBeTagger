#include "AmBeTagger/DetectorConstruction.hh"

#include "G4Colour.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalSurface.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "geomdefs.hh"

#include <cmath>
#include <iostream>
#include <string>

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

  if (bgoSolid == nullptr || couplingSolid == nullptr ||
      wrapBarrelSolid == nullptr || wrapBackCapSolid == nullptr ||
      wrapFrontCapSolid == nullptr ||
      pmtWindowSolid == nullptr || pmtPlaneSolid == nullptr ||
      pmtBodySolid == nullptr) {
    return 1;
  }

  constexpr double kTolerance = 1.0e-9 * mm;

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

  ok &= CheckMaterial(bgoLogical, "BGO", "BGO");
  ok &= CheckMaterial(couplingLogical, "EJ-550", "EJ550OpticalGrease");
  ok &= CheckMaterial(wrapBarrelLogical, "PTFE barrel wrap", "G4_TEFLON");
  ok &= CheckMaterial(wrapBackCapLogical, "PTFE back cap", "G4_TEFLON");
  ok &= CheckMaterial(wrapFrontCapLogical, "PTFE front cap", "G4_TEFLON");
  ok &= CheckMaterial(pmtWindowLogical, "PMT window", "UVGlass");
  ok &= CheckMaterial(pmtPlaneLogical, "PMT plane", "G4_AIR");
  ok &= CheckMaterial(pmtBodyLogical, "PMT body", "BorosilicateGlass");

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

  ok &= Require(detector.GetPmtPlaneVolume() == pmtPlaneLogical,
                "Detector should still expose the PMT plane as the readout volume");

  ok &= Require(pmtPlaneSolid->Inside(G4ThreeVector(12.49 * mm, 0.0, 0.0))
                    != kOutside,
                "Point inside the PMT aperture was rejected");
  ok &= Require(pmtPlaneSolid->Inside(G4ThreeVector(12.51 * mm, 0.0, 0.0))
                    == kOutside,
                "Point outside the PMT aperture was accepted");

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
