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
    int quantity;
    double price; 
};

class Database {
private:
    sqlite3* db;
    std::string dbName;

    static int menuCallback(void* data, int argc, char** argv, char** azColName);

public:
    Database(const std::string& dbname);
    ~Database();

    bool open();
    void close();
    bool executeQuery(const std::string& sql);
    
    void initialize();
    void seedData();

    std::vector<MenuItemData> loadMenu();
    int insertOrder(const std::string& orderType, const std::string& status);
    bool insertOrderItem(int orderId, int menuId, int quantity);
    bool updateOrderStatus(int orderId, const std::string& status);
};

#endif // DB_H
