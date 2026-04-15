#ifndef MENU_ITEM_H
#define MENU_ITEM_H

#include <string>

class MenuItem {
protected:
    int id;
    std::string name;
    double price;

public:
    MenuItem(int id, const std::string& name, double price) 
        : id(id), name(name), price(price) {}

    virtual ~MenuItem() = default;

    int getId() const { return id; }
    std::string getName() const { return name; }
    double getPrice() const { return price; }

    // Pure virtual method for polymorphic behavior
    virtual std::string getDetails() const = 0; 
};

#endif // MENU_ITEM_H
