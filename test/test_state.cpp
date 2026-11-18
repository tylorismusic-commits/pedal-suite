// Pedal Suite — state tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>
#include <cstdio>
#include <vector>

#include "module_ids.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "state.hpp"

using namespace pedal;

namespace {
int failures = 0;
void check(bool ok, const char* name) {
  std::printf("%s  %s\n", ok ? "PASS" : "FAIL", name);
  if (!ok) ++failures;
}
} // namespace

int main() {
  Shared a;
  initializeRack(a);
  a.set(slotParam(0, kSlotOffsetC0), 0.42);
  a.set(slotParam(7, kSlotOffsetType), static_cast<double>(ModuleId::Boost));
  a.set(slotParam(7, kSlotOffsetMix), 73.0);
  a.set(P_OUTPUT, -3.5);

  const auto blob = serializeState(a, 1.25);
  Shared b;
  double scale = 0.0;
  check(deserializeState(blob.data(), blob.size(), b, &scale),
        "state round-trip decodes");
  check(std::fabs(scale - 1.25) < 1e-9, "state restores UI scale");
  check(std::fabs(b.get(slotParam(0, kSlotOffsetC0)) - 0.42) < 1e-9,
        "state restores pedal control");
  check(std::fabs(b.get(P_OUTPUT) + 3.5) < 1e-9,
        "state restores global output");
  check(static_cast<int>(b.get(slotParam(7, kSlotOffsetType)) + 0.5) ==
            static_cast<int>(ModuleId::Boost),
        "state restores page-2 pedal type");

  check(!deserializeState(nullptr, 0, b), "null state is rejected");
  unsigned char bad[8] = {0};
  check(!deserializeState(bad, sizeof(bad), b), "short state is rejected");

  std::printf("%d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
