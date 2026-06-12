#pragma once
#include <string>
#include <vector>
#include "model/Sample.h"

enum class SampleMenuChoice { REGISTER = 1, LIST, SEARCH, BACK = 0 };

class SampleView {
public:
    void displayMenu() const;
    SampleMenuChoice getChoice() const;
    Sample inputSampleData() const;
    std::string inputSearchName() const;
    void displaySamples(const std::vector<Sample>& samples) const;
    void displayResult(bool success, const std::string& msg) const;
};
