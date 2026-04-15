#include "OrderService.h"
#include <iostream>
#include <iomanip>

OrderService::OrderService(Database* database) : db(database) {}

Order* OrderService::createOrder(int typeChoice) {
    std::string typeStr;
    if (typeChoice == 1) typeStr = "DineIn";
    else if (typeChoice == 2) typeStr = "Takeaway";
    else if (typeChoice == 3) typeStr = "Delivery";
    else return nullptr;

    std::string status = "Placed";
    int orderId = db->insertOrder(typeStr, status);
    
    if (orderId == -1) {
        std::cerr << "Failed to create order in database.\n";
        return nullptr;
    }

    std::unique_ptr<Order> newOrder;
    if (typeChoice == 1) newOrder = std::make_unique<DineInOrder>(orderId, status);
    else if (typeChoice == 2) newOrder = std::make_unique<TakeawayOrder>(orderId, status);
    else if (typeChoice == 3) newOrder = std::make_unique<DeliveryOrder>(orderId, status);

    Order* orderPtr = newOrder.get();
    activeOrders.push_back(std::move(newOrder));
    return orderPtr;
}

bool OrderService::addItemToOrder(Order* order, int menuId, int quantity, double price) {
    if (order && db->insertOrderItem(order->getOrderId(), menuId, quantity)) {
        OrderItemData item;
        item.id = 0; // DB generates this
        item.menuId = menuId;
        item.quantity = quantity;
        item.price = price;
        item.orderId = order->getOrderId();
        order->addItem(item);
        return true;
    }
    return false;
}

bool OrderService::advanceOrderStatus(Order* order) {
    if (!order) return false;
    
    std::string currentStatus = order->getStatus();
    std::string nextStatus;
    
    if (currentStatus == "Placed") nextStatus = "Preparing";
    else if (currentStatus == "Preparing") nextStatus = "Served";
    else if (currentStatus == "Served") nextStatus = "Paid";
    else return false;

    if (db->updateOrderStatus(order->getOrderId(), nextStatus)) {
        order->setStatus(nextStatus);
        return true;
    }
    return false;
}

const std::vector<std::unique_ptr<Order>>& OrderService::getActiveOrders() const {
    return activeOrders;
}

void OrderService::showMenu(const std::vector<MenuItemData>& menu) {
    std::cout << "\n--- RESTAURANT MENU ---\n";
    std::cout << std::left << std::setw(5) << "ID" 
              << std::setw(20) << "Name" 
              << std::setw(15) << "Category" 
              << "Price\n";
    std::cout << "------------------------------------------------\n";
    for (const auto& item : menu) {
        std::cout << std::left << std::setw(5) << item.id 
                  << std::setw(20) << item.name 
                  << std::setw(15) << item.category 
                  << "$" << std::fixed << std::setprecision(2) << item.price << "\n";
    }
    std::cout << "------------------------------------------------\n";
}
