#ifndef BEVERAGE_H
#define BEVERAGE_H

#include "MenuItem.h"

class Beverage : public MenuItem {
public:
    Beverage(int id, const std::string& name, double price) 
        : MenuItem(id, name, price) {}

    std::string getDetails() const override {
        return "[Beverage] " + getName();
    }
};

#endif // BEVERAGE_H
