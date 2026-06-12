#pragma once
#include <string>

enum class MainMenuChoice {
    SAMPLE_MANAGEMENT = 1,
    ORDER_MANAGEMENT,
    MONITORING,
    SHIPMENT,
    PRODUCTION_LINE,
    EXIT = 0
};

class MainView {
public:
    void displayMenu() const;
    MainMenuChoice getChoice() const;
    void displayMessage(const std::string& msg) const;
};
