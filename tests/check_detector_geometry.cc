#include "AmBeTagger/DetectorConstruction.hh"

#include "G4Colour.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
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
  G4LogicalVolume* pmtWindowLogical = LogicalVolume("PmtWindowLogical");
  G4LogicalVolume* pmtPlaneLogical = LogicalVolume("PmtPlaneLogical");

  G4VPhysicalVolume* bgoPhysical = PhysicalVolume("bgoPhysical");
  G4VPhysicalVolume* couplingPhysical = PhysicalVolume("CouplingPhysical");
  G4VPhysicalVolume* pmtWindowPhysical = PhysicalVolume("PmtWindowPhysical");
  G4VPhysicalVolume* pmtPlanePhysical = PhysicalVolume("PmtPlanePhysical");

  ok &= Require(worldLogical != nullptr, "World logical volume is missing");
  ok &= Require(bgoPhysical != nullptr, "BGO physical volume is missing");
  ok &= Require(couplingPhysical != nullptr,
                "EJ-550 physical volume is missing");
  ok &= Require(pmtWindowPhysical != nullptr,
                "PMT window physical volume is missing");
  ok &= Require(pmtPlanePhysical != nullptr,
                "PMT plane physical volume is missing");

  if (!ok) {
    return 1;
  }

  ok &= !bgoPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !couplingPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !pmtWindowPhysical->CheckOverlaps(1000, 0.0, false);
  ok &= !pmtPlanePhysical->CheckOverlaps(1000, 0.0, false);

  const G4Tubs* bgoSolid = RequireTubs(bgoLogical, "BGO");
  const G4Tubs* couplingSolid = RequireTubs(couplingLogical, "EJ-550");
  const G4Tubs* pmtWindowSolid = RequireTubs(pmtWindowLogical, "PMT window");
  const G4Tubs* pmtPlaneSolid = RequireTubs(pmtPlaneLogical, "PMT plane");

  if (bgoSolid == nullptr || couplingSolid == nullptr ||
      pmtWindowSolid == nullptr || pmtPlaneSolid == nullptr) {
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
                   25.1 * mm,
                   kTolerance);
  ok &= CheckClose("EJ-550 full thickness",
                   2.0 * couplingSolid->GetZHalfLength(),
                   0.1 * mm,
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

  ok &= CheckClose("BGO to EJ-550 contact",
                   BackFaceZ(couplingPhysical, couplingSolid),
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

  ok &= CheckMaterial(bgoLogical, "BGO", "BGO");
  ok &= CheckMaterial(couplingLogical, "EJ-550", "EJ550OpticalGrease");
  ok &= CheckMaterial(pmtWindowLogical, "PMT window", "UVGlass");
  ok &= CheckMaterial(pmtPlaneLogical, "PMT plane", "G4_AIR");

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

  ok &= Require(worldLogical->GetVisAttributes() != nullptr &&
                    !worldLogical->GetVisAttributes()->IsVisible(),
                "World volume should be invisible");
  ok &= CheckVisAttributes(bgoLogical,
                           "BGO",
                           G4Colour(0.05, 0.25, 1.0, 0.38));
  ok &= CheckVisAttributes(couplingLogical,
                           "EJ-550",
                           G4Colour(1.0, 0.82, 0.05, 0.55));
  ok &= CheckVisAttributes(pmtWindowLogical,
                           "PMT window",
                           G4Colour(0.0, 0.82, 0.95, 0.45));
  ok &= CheckVisAttributes(pmtPlaneLogical,
                           "PMT plane",
                           G4Colour(0.0, 0.95, 0.28, 0.9));

  ok &= Require(pmtPlaneSolid->Inside(G4ThreeVector(12.49 * mm, 0.0, 0.0))
                    != kOutside,
                "Point inside the PMT aperture was rejected");
  ok &= Require(pmtPlaneSolid->Inside(G4ThreeVector(12.51 * mm, 0.0, 0.0))
                    == kOutside,
                "Point outside the PMT aperture was accepted");

  const auto* wrapSurface =
      G4LogicalBorderSurface::GetSurface(bgoPhysical, worldPhysical);
  ok &= Require(wrapSurface != nullptr,
                "BGO-to-world optical wrap border surface is missing");

  if (!ok) {
    return 1;
  }

  std::cout << "Detector geometry checks passed\n";
  return 0;
}
