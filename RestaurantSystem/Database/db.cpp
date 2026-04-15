#include "db.h"
#include <stdexcept>

Database::Database(const std::string& dbname) : dbName(dbname), db(nullptr) {
}

Database::~Database() {
    close();
}

bool Database::open() {
    int rc = sqlite3_open(dbName.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

void Database::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool Database::executeQuery(const std::string& sql) {
    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    return true;
}

void Database::initialize() {
    const std::string sql = 
        "CREATE TABLE IF NOT EXISTS Menu ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "category TEXT NOT NULL, "
        "price REAL NOT NULL);"
        
        "CREATE TABLE IF NOT EXISTS Orders ("
        "orderId INTEGER PRIMARY KEY AUTOINCREMENT, "
        "orderType TEXT NOT NULL, "
        "status TEXT NOT NULL);"
        
        "CREATE TABLE IF NOT EXISTS OrderItems ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "orderId INTEGER NOT NULL, "
        "menuId INTEGER NOT NULL, "
        "quantity INTEGER NOT NULL, "
        "FOREIGN KEY(orderId) REFERENCES Orders(orderId), "
        "FOREIGN KEY(menuId) REFERENCES Menu(id));"
        
        "CREATE TABLE IF NOT EXISTS BillingRules ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "taxRate REAL, "
        "serviceCharge REAL, "
        "deliveryFee REAL);";

    executeQuery(sql);
}

void Database::seedData() {
    const std::string checkSql = "SELECT COUNT(*) FROM Menu;";
    sqlite3_stmt* stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, checkSql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (count == 0) {
        const std::string seedSql = 
            "INSERT INTO Menu (name, category, price) VALUES ('Garlic Bread', 'Appetizer', 4.50);"
            "INSERT INTO Menu (name, category, price) VALUES ('Spring Rolls', 'Appetizer', 5.00);"
            "INSERT INTO Menu (name, category, price) VALUES ('Steak', 'MainCourse', 25.00);"
            "INSERT INTO Menu (name, category, price) VALUES ('Pasta', 'MainCourse', 15.00);"
            "INSERT INTO Menu (name, category, price) VALUES ('Cola', 'Beverage', 2.00);"
            "INSERT INTO Menu (name, category, price) VALUES ('Coffee', 'Beverage', 3.00);"
            "INSERT INTO BillingRules (taxRate, serviceCharge, deliveryFee) VALUES (0.10, 0.05, 5.00);";
            
        executeQuery(seedSql);
    }
}

int Database::menuCallback(void* data, int argc, char** argv, char** azColName) {
    auto* menuList = static_cast<std::vector<MenuItemData>*>(data);
    MenuItemData item;
    for (int i = 0; i < argc; i++) {
        std::string colName = azColName[i];
        if (argv[i]) {
            if (colName == "id") item.id = std::stoi(argv[i]);
            else if (colName == "name") item.name = argv[i];
            else if (colName == "category") item.category = argv[i];
            else if (colName == "price") item.price = std::stod(argv[i]);
        }
    }
    menuList->push_back(item);
    return 0;
}

std::vector<MenuItemData> Database::loadMenu() {
    std::vector<MenuItemData> menuItems;
    const std::string sql = "SELECT * FROM Menu;";
    char* zErrMsg = nullptr;
    
    int rc = sqlite3_exec(db, sql.c_str(), menuCallback, &menuItems, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error in loadMenu: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    return menuItems;
}

int Database::insertOrder(const std::string& orderType, const std::string& status) {
    const std::string sql = "INSERT INTO Orders (orderType, status) VALUES ('" + orderType + "', '" + status + "');";
    if (executeQuery(sql)) {
        return static_cast<int>(sqlite3_last_insert_rowid(db));
    }
    return -1;
}

bool Database::insertOrderItem(int orderId, int menuId, int quantity) {
    const std::string sql = "INSERT INTO OrderItems (orderId, menuId, quantity) VALUES (" +
                            std::to_string(orderId) + ", " +
                            std::to_string(menuId) + ", " +
                            std::to_string(quantity) + ");";
    return executeQuery(sql);
}

bool Database::updateOrderStatus(int orderId, const std::string& status) {
    const std::string sql = "UPDATE Orders SET status = '" + status + "' WHERE orderId = " + std::to_string(orderId) + ";";
    return executeQuery(sql);
}
