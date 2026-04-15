#ifndef APPETIZER_H
#define APPETIZER_H

#include "MenuItem.h"

class Appetizer : public MenuItem {
public:
    Appetizer(int id, const std::string& name, double price) 
        : MenuItem(id, name, price) {}

    std::string getDetails() const override {
        return "[Appetizer] " + getName();
    }
};

#endif // APPETIZER_H
