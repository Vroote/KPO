#pragma once

class Vehicle;

class BeginnerPolicy {
public:
    static constexpr int kMinSimplicity = 6;

    bool isSuitable(const Vehicle &vehicle) const;
};
