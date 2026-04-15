#ifndef DINE_IN_ORDER_H
#define DINE_IN_ORDER_H

#include "Order.h"

class DineInOrder : public Order {
public:
    DineInOrder(int id, const std::string& status) : Order(id, status) {}

    std::string getOrderType() const override {
        return "DineIn";
    }

    double getExtraCharges(double subtotal, double serviceChargeRate, double deliveryFeeRate) const override {
        return subtotal * serviceChargeRate;
    }

    std::string getExtraChargesDetail() const override {
        return "Service Charge";
    }
};

#endif // DINE_IN_ORDER_H
