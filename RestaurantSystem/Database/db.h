#ifndef DB_H
#define DB_H

#include <string>
#include <vector>
#include <iostream>
#include <sqlite3.h>

struct MenuItemData {
    int id;
    std::string name;
    std::string category;
    double price;
};

struct OrderItemData {
    int id;
    int orderId;
    int menuId;
    std::string name;   // FIX #7: added item name
    int quantity;
    double price;       // FIX #2: added price for persistence
};

// FIX #11: struct to hold a full order loaded from DB
struct OrderData {
    int orderId;
    std::string orderType;
    std::string status;
    std::vector<OrderItemData> items;
};

// FIX #14: struct for billing rules loaded from DB
struct BillingRulesData {
    double taxRate;
    double serviceCharge;
    double deliveryFee;
};

class Database {
private:
    sqlite3* db;
    std::string dbName;

    // FIX #1: helper for schema migration
    bool columnExists(const std::string& table, const std::string& column);

public:
    Database(const std::string& dbname);
    ~Database();

    bool open();
    void close();
    bool executeQuery(const std::string& sql);

    void initialize();
    void seedData();

    std::vector<MenuItemData> loadMenu();
    std::vector<OrderData>    loadActiveOrders();   // FIX #11
    BillingRulesData          loadBillingRules();   // FIX #14

    int  insertOrder(const std::string& orderType, const std::string& status);
    bool insertOrderItem(int orderId, int menuId, int quantity,
                         double price, const std::string& name); // FIX #2,#7
    bool updateOrderStatus(int orderId, const std::string& status);
};

#endif // DB_H
