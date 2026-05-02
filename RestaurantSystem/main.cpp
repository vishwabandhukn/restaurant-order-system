#include <iostream>
#include <limits>
#include <iomanip>
#ifdef _WIN32
    #include <io.h>      // FIX #4: for _isatty / _fileno
#else
    #include <unistd.h>
#endif
#include "Database/db.h"
#include "Services/OrderService.h"
#include "Services/BillingService.h"
#include "Models/StudentDiscount.h"
#include "Models/HappyHourDiscount.h"

using namespace std;

// FIX #4: use _isatty to properly detect interactive terminal
void clearScreen() {
#ifdef _WIN32
    if (!_isatty(_fileno(stdin))) return;
    system("cls");
#else
    if (!isatty(STDIN_FILENO)) return;
    system("clear");
#endif
}

// FIX #5: now prints a prompt and actually blocks for Enter
void waitForEnter() {
    cout << "\nPress Enter to continue...";
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

int main() {
    Database db("database.db");
    if (!db.open()) {
        cerr << "Failed to initialize database." << endl;
        return 1;
    }
    db.initialize();
    db.seedData();

    // FIX #14: load billing rates from DB instead of hardcoding
    BillingRulesData rules = db.loadBillingRules();

    OrderService  orderService(&db);
    BillingService billingService;
    billingService.setRates(rules.taxRate, rules.serviceCharge, rules.deliveryFee);

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

                // Find menu item by ID
                double price = -1;
                string itemName;
                for (const auto& item : menu) {
                    if (item.id == menuId) {
                        price    = item.price;
                        itemName = item.name;
                        break;
                    }
                }

                if (price < 0) {
                    cout << "Invalid Menu ID. Please choose from the menu above.\n";
                    continue;
                }

                cout << "Enter Quantity: ";
                int qty;
                cin >> qty;

                // FIX #15: clear quantity validation message
                if (qty <= 0) {
                    cout << "Quantity must be a positive number.\n";
                    continue;
                }

                // FIX #7: pass item name to addItemToOrder
                if (orderService.addItemToOrder(newOrder, menuId, qty, price, itemName)) {
                    cout << "Added: " << itemName << " x" << qty
                         << " = $" << fixed << setprecision(2) << (price * qty) << "\n";
                } else {
                    cout << "Failed to add item. Please try again.\n";
                }
            }
            // FIX #10: show order summary before returning
            cout << "\n--- Order Summary (ID: " << newOrder->getOrderId() << ") ---\n";
            for (const auto& it : newOrder->getItems()) {
                cout << "  " << it.name << " x" << it.quantity
                     << " = $" << fixed << setprecision(2) << (it.price * it.quantity) << "\n";
            }
            cout << "  Subtotal: $" << fixed << setprecision(2) << newOrder->getSubtotal() << "\n";
            waitForEnter();
        }
        else if (choice == 3) {
            clearScreen();
            const auto& activeOrders = orderService.getActiveOrders();
            if (activeOrders.empty()) {
                cout << "No active orders.\n";
                waitForEnter();
                continue;
            }

            cout << "--- ACTIVE ORDERS ---\n";
            for (const auto& ord : activeOrders) {
                cout << "Order ID: " << ord->getOrderId()
                     << " | Type: "   << ord->getOrderType()
                     << " | Status: " << ord->getStatus()
                     << " | Items: "  << ord->getItems().size() << "\n";
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
                cout << "Order not found.\n";
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
                    cout << "Status updated to: " << targetOrder->getStatus() << "\n";
                } else {
                    cout << "Cannot advance status further (already Paid or no items).\n";
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

                unique_ptr<Discount> appliedDiscount;
                if      (discChoice == 2) appliedDiscount = make_unique<StudentDiscount>();
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
