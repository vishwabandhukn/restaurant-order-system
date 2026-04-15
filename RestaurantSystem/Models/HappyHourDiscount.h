#ifndef HAPPY_HOUR_DISCOUNT_H
#define HAPPY_HOUR_DISCOUNT_H

#include "Discount.h"

class HappyHourDiscount : public Discount {
public:
    double calculateDiscount(double subtotal) const override {
        return subtotal * 0.20; // 20% discount
    }
    
    std::string getDiscountName() const override {
        return "Happy Hour Discount (20%)";
    }
};

#endif // HAPPY_HOUR_DISCOUNT_H
