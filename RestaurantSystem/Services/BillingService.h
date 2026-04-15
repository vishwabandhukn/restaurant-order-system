#ifndef BILLING_SERVICE_H
#define BILLING_SERVICE_H

#include "../Models/Order.h"
#include "../Models/Discount.h"
#include <memory>

class BillingService {
private:
    double taxRate;
    double serviceChargeRate;
    double deliveryFee;

public:
    BillingService();
    void setRates(double tax, double service, double delivery);
    void generateBill(const Order* order, const Discount* discount = nullptr) const;
};

#endif // BILLING_SERVICE_H
