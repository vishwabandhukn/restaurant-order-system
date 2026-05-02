#include "BillingService.h"
#include <iostream>
#include <iomanip>

BillingService::BillingService() : taxRate(0.0), serviceChargeRate(0.0), deliveryFee(0.0) {}

void BillingService::setRates(double tax, double service, double delivery) {
    taxRate           = tax;
    serviceChargeRate = service;
    deliveryFee       = delivery;
}

void BillingService::generateBill(const Order* order, const Discount* discount) const {
    if (!order) return;

    std::cout << "\n============================================\n";
    std::cout << "               FINAL BILL                   \n";
    std::cout << "============================================\n";
    std::cout << "Order ID : " << order->getOrderId()
              << "  |  Type: " << order->getOrderType() << "\n";
    std::cout << "Status   : " << order->getStatus() << "\n";
    std::cout << "--------------------------------------------\n";

    double subtotal = order->getSubtotal();

    // FIX #6: display item name instead of "Item ID X"
    for (const auto& item : order->getItems()) {
        std::string label = item.name.empty()
                            ? ("Item #" + std::to_string(item.menuId))
                            : item.name;
        std::cout << std::left  << std::setw(22) << label
                  << std::setw(6) << ("x" + std::to_string(item.quantity))
                  << "$" << std::fixed << std::setprecision(2)
                  << (item.price * item.quantity) << "\n";
    }

    std::cout << "--------------------------------------------\n";
    std::cout << std::left << std::setw(30) << "Subtotal:" << "$" << subtotal << "\n";

    double discountAmt = 0;
    if (discount) {
        discountAmt = discount->calculateDiscount(subtotal);
        std::cout << std::left << std::setw(30) << discount->getDiscountName()
                  << "-$" << discountAmt << "\n";
    }

    double afterDiscount = subtotal - discountAmt;
    if (afterDiscount < 0) afterDiscount = 0;

    double taxAmt = afterDiscount * taxRate;
    std::cout << std::left << std::setw(30)
              << ("Tax (" + std::to_string(int(taxRate * 100)) + "%):")
              << "$" << taxAmt << "\n";

    // FIX #8: pass original subtotal so service charge isn't on discounted amount
    double extra = order->getExtraCharges(subtotal, serviceChargeRate, deliveryFee);
    if (extra > 0) {
        std::cout << std::left << std::setw(30) << order->getExtraChargesDetail()
                  << "$" << extra << "\n";
    }

    double total = afterDiscount + taxAmt + extra;
    std::cout << "============================================\n";
    std::cout << std::left << std::setw(30) << "GRAND TOTAL:" << "$" << total << "\n";
    std::cout << "============================================\n";
}
