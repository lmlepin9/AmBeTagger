#include "AmBeTagger/PmtResponse.hh"

#include "G4SystemOfUnits.hh"

#include <cmath>
#include <iostream>

namespace
{
bool CloseEnough(double actual, double expected, double tolerance)
{
  return std::abs(actual - expected) < tolerance;
}
}

int main()
{
  const AmBeTagger::PmtResponse pmt;

  const double qeAtPeak = pmt.QuantumEfficiency(3.139 * eV);
  const double qeOutOfRangeLow = pmt.QuantumEfficiency(1.0 * eV);
  const double qeOutOfRangeHigh = pmt.QuantumEfficiency(8.0 * eV);

  if (!CloseEnough(qeAtPeak, 0.24408, 1.0e-8)) {
    std::cerr << "QE check failed at 3.139 eV: " << qeAtPeak << '\n';
    return 1;
  }

  if (!CloseEnough(qeOutOfRangeLow, 0.0, 1.0e-12)) {
    std::cerr << "QE low out-of-range check failed: "
              << qeOutOfRangeLow << '\n';
    return 1;
  }

  if (!CloseEnough(qeOutOfRangeHigh, 0.0, 1.0e-12)) {
    std::cerr << "QE high out-of-range check failed: "
              << qeOutOfRangeHigh << '\n';
    return 1;
  }

  std::cout << "PmtResponse QE checks passed\n";
  return 0;
}