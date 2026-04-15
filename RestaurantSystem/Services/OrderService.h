#ifndef ORDER_SERVICE_H
#define ORDER_SERVICE_H

#include <vector>
#include <memory>
#include "../Database/db.h"
#include "../Models/Order.h"
#include "../Models/DineInOrder.h"
#include "../Models/TakeawayOrder.h"
#include "../Models/DeliveryOrder.h"

class OrderService {
private:
    Database* db;
    std::vector<std::unique_ptr<Order>> activeOrders;

public:
    OrderService(Database* database);
    
    Order* createOrder(int typeChoice);
    bool addItemToOrder(Order* order, int menuId, int quantity, double price);
    bool advanceOrderStatus(Order* order);
    
    const std::vector<std::unique_ptr<Order>>& getActiveOrders() const;
    void showMenu(const std::vector<MenuItemData>& menu);
};

#endif // ORDER_SERVICE_H
