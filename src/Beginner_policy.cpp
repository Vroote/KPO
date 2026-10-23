#include "Beginner_policy.h"

#include "equipment.h"

bool BeginnerPolicy::isSuitable(const Vehicle &vehicle) const { return vehicle.simplicity() >= kMinSimplicity; }
