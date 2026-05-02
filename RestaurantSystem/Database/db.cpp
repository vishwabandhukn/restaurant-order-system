#include "db.h"
#include <stdexcept>

Database::Database(const std::string& dbname) : dbName(dbname), db(nullptr) {}

Database::~Database() { close(); }

bool Database::open() {
    int rc = sqlite3_open(dbName.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

void Database::close() {
    if (db) { sqlite3_close(db); db = nullptr; }
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

// FIX #1: helper — checks whether a column exists in a table
bool Database::columnExists(const std::string& table, const std::string& column) {
    std::string sql = "PRAGMA table_info(" + table + ");";
    sqlite3_stmt* stmt;
    bool found = false;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* col = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            if (col && column == col) { found = true; break; }
        }
        sqlite3_finalize(stmt);
    }
    return found;
}

// FIX #1: each CREATE TABLE is a separate executeQuery call
void Database::initialize() {
    executeQuery(
        "CREATE TABLE IF NOT EXISTS Menu ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "category TEXT NOT NULL, "
        "price REAL NOT NULL);"
    );
    executeQuery(
        "CREATE TABLE IF NOT EXISTS Orders ("
        "orderId INTEGER PRIMARY KEY AUTOINCREMENT, "
        "orderType TEXT NOT NULL, "
        "status TEXT NOT NULL);"
    );

    // FIX #2: migrate OrderItems table if price or name columns are missing
    if (!columnExists("OrderItems", "price") || !columnExists("OrderItems", "name")) {
        executeQuery("DROP TABLE IF EXISTS OrderItems;");
    }
    executeQuery(
        "CREATE TABLE IF NOT EXISTS OrderItems ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "orderId INTEGER NOT NULL, "
        "menuId INTEGER NOT NULL, "
        "name TEXT NOT NULL, "
        "quantity INTEGER NOT NULL, "
        "price REAL NOT NULL, "
        "FOREIGN KEY(orderId) REFERENCES Orders(orderId), "
        "FOREIGN KEY(menuId) REFERENCES Menu(id));"
    );
    executeQuery(
        "CREATE TABLE IF NOT EXISTS BillingRules ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "taxRate REAL, "
        "serviceCharge REAL, "
        "deliveryFee REAL);"
    );
}

void Database::seedData() {
    // Check Menu
    sqlite3_stmt* stmt;
    int count = 0;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Menu;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }
    if (count == 0) {
        executeQuery("INSERT INTO Menu (name, category, price) VALUES ('Garlic Bread', 'Appetizer', 4.50);");
        executeQuery("INSERT INTO Menu (name, category, price) VALUES ('Spring Rolls', 'Appetizer', 5.00);");
        executeQuery("INSERT INTO Menu (name, category, price) VALUES ('Steak', 'MainCourse', 25.00);");
        executeQuery("INSERT INTO Menu (name, category, price) VALUES ('Pasta', 'MainCourse', 15.00);");
        executeQuery("INSERT INTO Menu (name, category, price) VALUES ('Cola', 'Beverage', 2.00);");
        executeQuery("INSERT INTO Menu (name, category, price) VALUES ('Coffee', 'Beverage', 3.00);");
    }

    // Check BillingRules
    int ruleCount = 0;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM BillingRules;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) ruleCount = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }
    if (ruleCount == 0) {
        executeQuery("INSERT INTO BillingRules (taxRate, serviceCharge, deliveryFee) VALUES (0.10, 0.05, 5.00);");
    }
}

// FIX #12: rewritten using sqlite3_prepare_v2 (no more SELECT * or callback)
std::vector<MenuItemData> Database::loadMenu() {
    std::vector<MenuItemData> menuItems;
    const std::string sql = "SELECT id, name, category, price FROM Menu;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            MenuItemData item;
            item.id       = sqlite3_column_int(stmt, 0);
            item.name     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            item.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            item.price    = sqlite3_column_double(stmt, 3);
            menuItems.push_back(item);
        }
        sqlite3_finalize(stmt);
    }
    return menuItems;
}

// FIX #11: loads all non-Paid orders + their items from DB
std::vector<OrderData> Database::loadActiveOrders() {
    std::vector<OrderData> orders;
    const std::string sql =
        "SELECT orderId, orderType, status FROM Orders WHERE status != 'Paid';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            OrderData ord;
            ord.orderId   = sqlite3_column_int(stmt, 0);
            ord.orderType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            ord.status    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            orders.push_back(ord);
        }
        sqlite3_finalize(stmt);
    }
    // Load items for each order
    for (auto& ord : orders) {
        const std::string itemSql =
            "SELECT id, orderId, menuId, name, quantity, price "
            "FROM OrderItems WHERE orderId = ?;";
        sqlite3_stmt* itemStmt;
        if (sqlite3_prepare_v2(db, itemSql.c_str(), -1, &itemStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(itemStmt, 1, ord.orderId);
            while (sqlite3_step(itemStmt) == SQLITE_ROW) {
                OrderItemData item;
                item.id       = sqlite3_column_int(itemStmt, 0);
                item.orderId  = sqlite3_column_int(itemStmt, 1);
                item.menuId   = sqlite3_column_int(itemStmt, 2);
                item.name     = reinterpret_cast<const char*>(sqlite3_column_text(itemStmt, 3));
                item.quantity = sqlite3_column_int(itemStmt, 4);
                item.price    = sqlite3_column_double(itemStmt, 5);
                ord.items.push_back(item);
            }
            sqlite3_finalize(itemStmt);
        }
    }
    return orders;
}

// FIX #14: read billing rates from DB instead of hardcoding
BillingRulesData Database::loadBillingRules() {
    BillingRulesData rules{0.10, 0.05, 5.00}; // safe defaults
    const std::string sql =
        "SELECT taxRate, serviceCharge, deliveryFee FROM BillingRules LIMIT 1;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            rules.taxRate       = sqlite3_column_double(stmt, 0);
            rules.serviceCharge = sqlite3_column_double(stmt, 1);
            rules.deliveryFee   = sqlite3_column_double(stmt, 2);
        }
        sqlite3_finalize(stmt);
    }
    return rules;
}

// FIX #3: use prepared statement to prevent SQL injection
int Database::insertOrder(const std::string& orderType, const std::string& status) {
    const std::string sql = "INSERT INTO Orders (orderType, status) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    sqlite3_bind_text(stmt, 1, orderType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, status.c_str(),    -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) return static_cast<int>(sqlite3_last_insert_rowid(db));
    return -1;
}

// FIX #2 + #3 + #7: store price & name; use prepared statement
bool Database::insertOrderItem(int orderId, int menuId, int quantity,
                               double price, const std::string& name) {
    const std::string sql =
        "INSERT INTO OrderItems (orderId, menuId, name, quantity, price) "
        "VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_int   (stmt, 1, orderId);
    sqlite3_bind_int   (stmt, 2, menuId);
    sqlite3_bind_text  (stmt, 3, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int   (stmt, 4, quantity);
    sqlite3_bind_double(stmt, 5, price);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

// FIX #3: use prepared statement
bool Database::updateOrderStatus(int orderId, const std::string& status) {
    const std::string sql = "UPDATE Orders SET status = ? WHERE orderId = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 2, orderId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}
