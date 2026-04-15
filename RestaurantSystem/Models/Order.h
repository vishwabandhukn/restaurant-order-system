#ifndef ORDER_H
#define ORDER_H

#include <vector>
#include <string>
#include "../Database/db.h"

class Order {
protected:
    int orderId;
    std::string status;
    std::vector<OrderItemData> items;

public:
    Order(int id, const std::string& status) : orderId(id), status(status) {}
    virtual ~Order() = default;

    int getOrderId() const { return orderId; }
    std::string getStatus() const { return status; }
    void setStatus(const std::string& newStatus) { status = newStatus; }
    
    void addItem(const OrderItemData& item) {
        items.push_back(item);
    }
    
    const std::vector<OrderItemData>& getItems() const {
        return items;
    }

    double getSubtotal() const {
        double total = 0;
        for (const auto& item : items) {
            total += item.price * item.quantity;
        }
        return total;
    }

    virtual std::string getOrderType() const = 0;
    
    // Polymorphic overrides for billing
    virtual double getExtraCharges(double subtotal, double serviceChargeRate, double deliveryFeeRate) const = 0;
    virtual std::string getExtraChargesDetail() const = 0;
};

#endif // ORDER_H
