#ifndef TAKEAWAY_ORDER_H
#define TAKEAWAY_ORDER_H

#include "Order.h"

class TakeawayOrder : public Order {
public:
    TakeawayOrder(int id, const std::string& status) : Order(id, status) {}

    std::string getOrderType() const override {
        return "Takeaway";
    }

    double getExtraCharges(double subtotal, double serviceChargeRate, double deliveryFeeRate) const override {
        // No extra charges for takeaway
        return 0;
    }

    std::string getExtraChargesDetail() const override {
        return ""; // No extra charges for takeaway
    }
};

#endif // TAKEAWAY_ORDER_H
