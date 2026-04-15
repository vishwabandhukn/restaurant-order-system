#ifndef DELIVERY_ORDER_H
#define DELIVERY_ORDER_H

#include "Order.h"

class DeliveryOrder : public Order {
public:
    DeliveryOrder(int id, const std::string& status) : Order(id, status) {}

    std::string getOrderType() const override {
        return "Delivery";
    }

    double getExtraCharges(double subtotal, double serviceChargeRate, double deliveryFeeRate) const override {
        // Flat delivery fee
        return deliveryFeeRate;
    }

    std::string getExtraChargesDetail() const override {
        return "Delivery Fee";
    }
};

#endif // DELIVERY_ORDER_H
