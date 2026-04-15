#ifndef DISCOUNT_H
#define DISCOUNT_H

#include <string>

class Discount {
public:
    virtual ~Discount() = default;
    
    // Returns the discount amount to be subtracted from the total
    virtual double calculateDiscount(double subtotal) const = 0;
    virtual std::string getDiscountName() const = 0;
};

#endif // DISCOUNT_H
