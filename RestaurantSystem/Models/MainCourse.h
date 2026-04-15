#ifndef MAIN_COURSE_H
#define MAIN_COURSE_H

#include "MenuItem.h"

class MainCourse : public MenuItem {
public:
    MainCourse(int id, const std::string& name, double price) 
        : MenuItem(id, name, price) {}

    std::string getDetails() const override {
        return "[Main Course] " + getName();
    }
};

#endif // MAIN_COURSE_H
