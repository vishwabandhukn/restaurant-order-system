#ifndef STUDENT_DISCOUNT_H
#define STUDENT_DISCOUNT_H

#include "Discount.h"

class StudentDiscount : public Discount {
public:
    double calculateDiscount(double subtotal) const override {
        return subtotal * 0.15; // 15% discount
    }
    
    std::string getDiscountName() const override {
        return "Student Discount (15%)";
    }
};

#endif // STUDENT_DISCOUNT_H
