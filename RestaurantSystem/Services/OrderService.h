#ifndef ORDER_SERVICE_H
#define ORDER_SERVICE_H

#include <vector>
#include <memory>
#include "../Database/db.h"
#include "../Models/Order.h"
#include "../Models/DineInOrder.h"
#include "../Models/TakeawayOrder.h"
#include "../Models/DeliveryOrder.h"
#include "../Models/Appetizer.h"
#include "../Models/Beverage.h"
#include "../Models/MainCourse.h"

class OrderService {
private:
    Database* db;
    std::vector<std::unique_ptr<Order>> activeOrders;

public:
    OrderService(Database* database);

    Order* createOrder(int typeChoice);
    // FIX #7: added name parameter
    bool addItemToOrder(Order* order, int menuId, int quantity,
                        double price, const std::string& name);
    bool advanceOrderStatus(Order* order);

    const std::vector<std::unique_ptr<Order>>& getActiveOrders() const;
    void showMenu(const std::vector<MenuItemData>& menu);
};

#endif // ORDER_SERVICE_H
