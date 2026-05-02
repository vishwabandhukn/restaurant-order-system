#include "OrderService.h"
#include <iostream>
#include <iomanip>

// FIX #11: constructor loads persisted active (non-Paid) orders from DB
OrderService::OrderService(Database* database) : db(database) {
    auto savedOrders = db->loadActiveOrders();
    for (const auto& data : savedOrders) {
        std::unique_ptr<Order> order;
        if      (data.orderType == "DineIn")   order = std::make_unique<DineInOrder>  (data.orderId, data.status);
        else if (data.orderType == "Takeaway") order = std::make_unique<TakeawayOrder>(data.orderId, data.status);
        else if (data.orderType == "Delivery") order = std::make_unique<DeliveryOrder>(data.orderId, data.status);

        if (order) {
            for (const auto& item : data.items) order->addItem(item);
            activeOrders.push_back(std::move(order));
        }
    }
    if (!activeOrders.empty()) {
        std::cout << "[INFO] Loaded " << activeOrders.size()
                  << " active order(s) from previous session.\n";
    }
}

Order* OrderService::createOrder(int typeChoice) {
    std::string typeStr;
    if      (typeChoice == 1) typeStr = "DineIn";
    else if (typeChoice == 2) typeStr = "Takeaway";
    else if (typeChoice == 3) typeStr = "Delivery";
    else return nullptr;

    int orderId = db->insertOrder(typeStr, "Placed");
    if (orderId == -1) {
        std::cerr << "Failed to create order in database.\n";
        return nullptr;
    }

    std::unique_ptr<Order> newOrder;
    if      (typeChoice == 1) newOrder = std::make_unique<DineInOrder>  (orderId, "Placed");
    else if (typeChoice == 2) newOrder = std::make_unique<TakeawayOrder>(orderId, "Placed");
    else if (typeChoice == 3) newOrder = std::make_unique<DeliveryOrder>(orderId, "Placed");

    Order* ptr = newOrder.get();
    activeOrders.push_back(std::move(newOrder));
    return ptr;
}

// FIX #7: name is now passed through and stored in DB + memory
bool OrderService::addItemToOrder(Order* order, int menuId, int quantity,
                                  double price, const std::string& name) {
    if (!order) return false;
    if (db->insertOrderItem(order->getOrderId(), menuId, quantity, price, name)) {
        OrderItemData item;
        item.id       = 0;
        item.menuId   = menuId;
        item.name     = name;
        item.quantity = quantity;
        item.price    = price;
        item.orderId  = order->getOrderId();
        order->addItem(item);
        return true;
    }
    return false;
}

bool OrderService::advanceOrderStatus(Order* order) {
    if (!order) return false;
    std::string next;
    const std::string& cur = order->getStatus();
    if      (cur == "Placed")   next = "Preparing";
    else if (cur == "Preparing") next = "Served";
    else if (cur == "Served")   next = "Paid";
    else return false;

    if (db->updateOrderStatus(order->getOrderId(), next)) {
        order->setStatus(next);
        return true;
    }
    return false;
}

const std::vector<std::unique_ptr<Order>>& OrderService::getActiveOrders() const {
    return activeOrders;
}

// FIX #13: showMenu now instantiates MenuItem subclasses for polymorphic display
void OrderService::showMenu(const std::vector<MenuItemData>& menu) {
    std::cout << "\n--- RESTAURANT MENU ---\n";
    std::cout << std::left << std::setw(5)  << "ID"
                           << std::setw(22) << "Name"
                           << std::setw(18) << "Category"
                           << "Price\n";
    std::cout << "--------------------------------------------------\n";
    for (const auto& data : menu) {
        // Create typed MenuItem object for polymorphic getDetails()
        std::unique_ptr<MenuItem> item;
        if      (data.category == "Appetizer")  item = std::make_unique<Appetizer> (data.id, data.name, data.price);
        else if (data.category == "MainCourse") item = std::make_unique<MainCourse>(data.id, data.name, data.price);
        else if (data.category == "Beverage")   item = std::make_unique<Beverage>  (data.id, data.name, data.price);

        std::cout << std::left << std::setw(5) << data.id
                  << std::setw(22) << data.name
                  << std::setw(18) << (item ? item->getDetails().substr(0, item->getDetails().find(']') + 1) : data.category)
                  << "$" << std::fixed << std::setprecision(2) << data.price << "\n";
    }
    std::cout << "--------------------------------------------------\n";
}
