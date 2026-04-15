#include <iostream>
#include <limits>
#include "Database/db.h"
#include "Services/OrderService.h"
#include "Services/BillingService.h"
#include "Models/StudentDiscount.h"
#include "Models/HappyHourDiscount.h"

using namespace std;

void clearScreen() {
    // Only clear if running in an interactive terminal (not piped)
    if (cin.rdbuf() == nullptr) return;
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void waitForEnter() {
    if (!cin.good()) return;
    // Flush any leftover newlines before waiting
    if (cin.peek() == '\n') cin.ignore();
}

int main() {
    Database db("database.db");
    if (!db.open()) {
        cerr << "Failed to initialize database." << endl;
        return 1;
    }

    db.initialize();
    db.seedData();

    OrderService orderService(&db);
    BillingService billingService;
    
    // Billing rules can be dynamically loaded, but for now we hardcode rates
    // matching seeded database BillingRules (tax 10%, service 5%, fee 5.00)
    billingService.setRates(0.10, 0.05, 5.00);

    bool running = true;
    while (running) {
        cout << "\n";
        cout << "========================================\n";
        cout << "  RESTAURANT ORDER & MENU SYSTEM\n";
        cout << "========================================\n";
        cout << "1. View Menu\n";
        cout << "2. Place New Order\n";
        cout << "3. Manage Active Orders\n";
        cout << "4. Exit\n";
        cout << "========================================\n";
        cout << "Enter choice: ";
        
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            clearScreen();
            auto menu = db.loadMenu();
            orderService.showMenu(menu);
            waitForEnter();
        }
        else if (choice == 2) {
            clearScreen();
            cout << "--- PLACE NEW ORDER ---\n";
            cout << "1. Dine-In\n";
            cout << "2. Takeaway\n";
            cout << "3. Delivery\n";
            cout << "Select Order Type: ";
            int typeChoice;
            cin >> typeChoice;

            Order* newOrder = orderService.createOrder(typeChoice);
            if (!newOrder) {
                cout << "Invalid choice. Order creation failed.\n";
                waitForEnter();
                continue;
            }

            cout << "Order created! ID: " << newOrder->getOrderId() << "\n\n";
            
            auto menu = db.loadMenu();
            orderService.showMenu(menu);

            while (true) {
                cout << "\nEnter Menu ID to add to order (0 to finish): ";
                int menuId;
                cin >> menuId;
                if (menuId == 0) break;

                // Find menu item to get price
                double price = -1;
                for (const auto& item : menu) {
                    if (item.id == menuId) {
                        price = item.price;
                        break;
                    }
                }

                if (price == -1) {
                    cout << "Invalid Menu ID.\n";
                    continue;
                }

                cout << "Enter Quantity: ";
                int qty;
                cin >> qty;

                if (qty > 0 && orderService.addItemToOrder(newOrder, menuId, qty, price)) {
                    cout << "Item added successfully.\n";
                } else {
                    cout << "Failed to add item.\n";
                }
            }
            waitForEnter();
        }
        else if (choice == 3) {
            clearScreen();
            const auto& activeOrders = orderService.getActiveOrders();
            if (activeOrders.empty()) {
                cout << "No active orders in this session.\n";
                waitForEnter();
                continue;
            }

            cout << "--- ACTIVE ORDERS ---\n";
            for (const auto& ord : activeOrders) {
                cout << "Order ID: " << ord->getOrderId() 
                     << " | Type: " << ord->getOrderType() 
                     << " | Status: " << ord->getStatus() << "\n";
            }

            cout << "\nEnter Order ID to manage (0 to go back): ";
            int manageId;
            if (!(cin >> manageId)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            if (manageId == 0) continue;

            Order* targetOrder = nullptr;
            for (const auto& ord : activeOrders) {
                if (ord->getOrderId() == manageId) {
                    targetOrder = ord.get();
                    break;
                }
            }

            if (!targetOrder) {
                cout << "Order not found in current session.\n";
                waitForEnter();
                continue;
            }

            cout << "\n1. Advance Status (Placed -> Preparing -> Served -> Paid)\n";
            cout << "2. Generate Final Bill\n";
            cout << "Choice: ";
            int subChoice;
            cin >> subChoice;

            if (subChoice == 1) {
                if (orderService.advanceOrderStatus(targetOrder)) {
                    cout << "Order status updated to: " << targetOrder->getStatus() << "\n";
                } else {
                    cout << "Cannot advance status further.\n";
                }
                waitForEnter();
            } else if (subChoice == 2) {
                cout << "\nApply Discount?\n";
                cout << "1. No Discount\n";
                cout << "2. Student Discount (15%)\n";
                cout << "3. Happy Hour Discount (20%)\n";
                cout << "Choice: ";
                int discChoice;
                cin >> discChoice;

                std::unique_ptr<Discount> appliedDiscount = nullptr;
                if (discChoice == 2) appliedDiscount = make_unique<StudentDiscount>();
                else if (discChoice == 3) appliedDiscount = make_unique<HappyHourDiscount>();

                billingService.generateBill(targetOrder, appliedDiscount.get());
                waitForEnter();
            }
        }
        else if (choice == 4) {
            running = false;
        }
    }

    db.close();
    return 0;
}
