#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>
#include <fstream>
#include "bank_customer.h"
#include "buyer.h"
#include "seller.h"
#include "transaction.h"

using namespace std;

void saveBankAccounts(const std::vector<BankCustomer *> &accounts, const std::string &filename)
{
    std::ofstream ofs(filename);
    for (auto &b : accounts)
    {
        ofs << b->getId() << "|" << b->getName() << "|" << b->getBalance() << "\n";
    }
}

void loadBankAccounts(std::vector<BankCustomer *> &accounts, const std::string &filename)
{
    std::ifstream ifs(filename);
    accounts.clear();
    std::string line;
    while (std::getline(ifs, line))
    {
        int id;
        std::string name;
        double balance;

        size_t pos1 = line.find('|');
        size_t pos2 = line.rfind('|');

        id = stoi(line.substr(0, pos1));
        name = line.substr(pos1 + 1, pos2 - pos1 - 1);
        balance = stod(line.substr(pos2 + 1));

        accounts.push_back(new BankCustomer(id, name, balance));
    }
}

void displayBuyer(const Buyer &b)
{
    cout << "ID: " << b.getId() << " | Name: " << b.getName();
    if (b.getAccount())
        cout << " | Balance: $" << b.getAccount()->getBalance();
    cout << endl;
}

void displaySeller(const Seller &s)
{
    cout << "ID: " << s.getId() << " | Name: " << s.getName();
    if (s.getAccount())
        cout << " | Balance: $" << s.getAccount()->getBalance();
    cout << endl;
}

int main()
{
    vector<Buyer> buyers;
    vector<Seller> sellers;
    vector<BankCustomer *> bankAccounts; 
    vector<Transaction> transactions;

    loadBankAccounts(bankAccounts, "bank.txt");

    int transactionCount = 1;

    int nextBankId = 1001;
    int nextUserId = 1;

    while (true)
    {
        cout << "ONLINE STORE\n";
        cout << "1. Login\n2. Register\n3. Admin Login\n4. Exit\nChoose: ";
        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 4)
        {
            cout << "Exiting program\n";
            break;
        }

        // ===== REGISTER =====
        if (choice == 2)
        {
            cout << "\nRegister\n1. Buyer\n2. Seller\nChoose: ";
            int type;
            cin >> type;
            cin.ignore();

            string name, pass;
            cout << "Enter Name: ";
            getline(cin, name);
            cout << "Enter Password: ";
            getline(cin, pass);

            char createAcc;
            cout << "Create Bank Account? (y/n): ";
            cin >> createAcc;
            cin.ignore();

            BankCustomer *accPtr = nullptr;
            if (createAcc == 'y' || createAcc == 'Y')
            {
                double balance;
                cout << "Enter Initial Balance: ";
                cin >> balance;
                cin.ignore();

                BankCustomer *newAcc = new BankCustomer(nextBankId, name, balance);
                bankAccounts.push_back(newAcc);
                accPtr = newAcc;
                cout << "Bank Account created! ID: " << nextBankId << endl;
                nextBankId++;
            }

            saveBankAccounts(bankAccounts, "bank.txt");

            if (type == 1)
            {
                buyers.emplace_back(nextUserId++, name, pass, accPtr);
                cout << "Buyer registered successfully!\n";
            }
            else
            {
                sellers.emplace_back(nextUserId++, name, pass, accPtr);
                cout << "Seller registered successfully!\n";
            }
        }

        // ===== LOGIN =====
        else if (choice == 1)
        {
            string name, pass;
            cout << "\nEnter Name: ";
            getline(cin, name);
            cout << "Enter Password: ";
            getline(cin, pass);

            bool loggedIn = false;

        
            for (auto &b : buyers)
            {
                if (b.getName() == name && b.getPassword() == pass)
                {
                    cout << "\nLogged in as Buyer!\n";
                    bool active = true;
                    while (active)
                    {
                        cout << "\n--- Buyer Menu ---\n";
                        cout << "1. View Items\n2. Purchase Item\n3. View Orders\n4. Bank Account\n5. Logout\nChoose: ";
                        int opt;
                        cin >> opt;
                        cin.ignore();

                        if (opt == 1)
                        {
                            cout << "\nAvailable Items:\n";
                            for (auto &s : sellers)
                                for (auto &item : s.getItems())
                                    cout << "[ID: " << s.getId() * 1000 + item.getId() << "] "
                                         << item.getName() << " | Price: " << item.getPrice()
                                         << " | Stock: " << item.getQuantity()
                                         << " | Seller: " << s.getName() << endl;
                        }
                        else if (opt == 2)
                        {
                            int itemCode, qty;
                            cout << "\nEnter Item ID: ";
                            cin >> itemCode;
                            cout << "Enter Quantity: ";
                            cin >> qty;
                            cin.ignore();

                            bool found = false;
                            for (auto &s : sellers)
                            {
                                for (auto &item : s.getItems())
                                {
                                    int code = s.getId() * 1000 + item.getId();
                                    if (code == itemCode)
                                    {
                                        found = true;
                                        double total = item.getPrice() * qty;
                                        BankCustomer *buyerAcc = b.getAccount();
                                        BankCustomer *sellerAcc = s.getAccount();

                                        if (!buyerAcc || !sellerAcc)
                                        {
                                            cout << "One or both users don't have bank account.\n";
                                        }
                                        else if (buyerAcc->withdrawBalance(total))
                                        {
                                            sellerAcc->addBalance(total);
                                            item.setQuantity(item.getQuantity() - qty);

                                            string tid = "T-" + to_string(transactionCount++);
                                            transactions.emplace_back(tid, b.getName(), b.getId(),
                                                                      s.getName(), s.getId(),
                                                                      item.getName(), qty, total, "PAID", "Debit");

                                            cout << "Payment successful! Total: " << total << endl;
                                        }
                                        else
                                            cout << "Not enough balance.\n";

                                        break;
                                    }
                                }
                                if (found)
                                    break;
                            }
                            if (!found)
                                cout << "Item not found.\n";
                        }
                        else if (opt == 3)
                        {
                            cout << "\n=== Your Orders ===\n";
                            bool found = false;
                            vector<int> orderIndexes; 

                            for (int i = 0; i < transactions.size(); ++i)
                            {
                                auto &t = transactions[i];
                                if (t.getBuyerName() == b.getName())
                                {
                                    cout << i + 1 << ". " << t.getId()
                                         << " | Seller: " << t.getSellerName()
                                         << " | Item: " << t.getItemName()
                                         << " | Total: " << t.getTotalPrice()
                                         << " | Status: " << t.getStatus() << endl;
                                    orderIndexes.push_back(i);
                                    found = true;
                                }
                            }

                            if (!found)
                            {
                                cout << "You have no orders yet.\n";
                            }
                            else
                            {
                                cout << "\nEnter order number to cancel (0 to back): ";
                                int choice;
                                cin >> choice;
                                cin.ignore();

                                if (choice > 0 && choice <= orderIndexes.size())
                                {
                                    Transaction &t = transactions[orderIndexes[choice - 1]];

                                    if (t.getStatus() == "PAID")
                                    {
                                        cout << "Are you sure you want to cancel this order? (y/n): ";
                                        char confirm;
                                        cin >> confirm;
                                        cin.ignore();

                                        if (confirm == 'y' || confirm == 'Y')
                                        {
                                            t.setStatus("CANCELLED");

        
                                            for (auto &buyer : buyers)
                                            {
                                                if (buyer.getName() == t.getBuyerName())
                                                {
                                                    buyer.getAccount()->addBalance(t.getTotalPrice(), "Refund");
                                                    break;
                                                }
                                            }

                                            for (auto &seller : sellers)
                                            {
                                                if (seller.getName() == t.getSellerName())
                                                {
                                                    seller.getAccount()->withdrawBalance(t.getTotalPrice(), "Refund Deduct");
                                                    break;
                                                }
                                            }

                                            cout << "Order cancelled and refund processed!\n";
                                        }
                                    }
                                    else if (t.getStatus() == "COMPLETE")
                                    {
                                        cout << "Cannot cancel completed orders.\n";
                                    }
                                    else if (t.getStatus() == "CANCELLED")
                                    {
                                        cout << "Order already cancelled.\n";
                                    }
                                }
                            }
                        }
                        else if (opt == 4)
                        { 
                            BankCustomer *acc = b.getAccount();
                            if (!acc)
                            {
                                cout << "No bank account!\n";
                                continue;
                            }

                            bool bankActive = true;
                            while (bankActive)
                            {
                                cout << "\n=== BANK ACCOUNT ===\n";
                                cout << "Account ID: " << acc->getId() << endl;
                                cout << "Balance: " << acc->getBalance() << endl;

                                cout << "1. Top Up\n2. Withdraw\n3. View Cash Flow\n4. Back\nChoose: ";
                                int choice2;
                                cin >> choice2;
                                cin.ignore();

                                if (choice2 == 1)
                                {
                                    double amt;
                                    cout << "Amount to Top Up: ";
                                    cin >> amt;
                                    cin.ignore();
                                    acc->addBalance(amt, "Top Up");
                                    cout << "Top Up Successful!\n";
                                }
                                else if (choice2 == 2)
                                {
                                    double amt;
                                    cout << "Amount to Withdraw: ";
                                    cin >> amt;
                                    cin.ignore();
                                    if (acc->withdrawBalance(amt, "Withdraw"))
                                        cout << "Withdraw Successful!\n";
                                    else
                                        cout << "Not enough balance!\n";
                                }
                                else if (choice2 == 3)
                                {
                                    acc->printCashFlow();
                                }
                                else if (choice2 == 4)
                                {
                                    bankActive = false;
                                }
                                else
                                {
                                    cout << "Invalid choice!\n";
                                }
                            }
                        }
                        else if (opt == 5)
                            active = false;
                        else
                            cout << "Invalid choice!\n";
                    }
                    loggedIn = true;
                    break;
                }
            }

            // Seller login
            if (!loggedIn)
            {
                for (auto &s : sellers)
                {
                    if (s.getName() == name && s.getPassword() == pass)
                    {
                        cout << "\nLogged in as Seller!\n";
                        bool active = true;
                        while (active)
                        {
                            cout << "\n--- Seller Menu ---\n";
                            cout << "1. Manage Items (Add/Replenish/Discard)\n";
                            cout << "2. View Transactions (Your Sales)\n";
                            cout << "3. View Items & Purchase\n";
                            cout << "4. View Your Orders (As Buyer)\n";
                            cout << "5. Bank Account\n";
                            cout << "6. Logout\n";
                            cout << "Choose: ";
                            int opt;
                            cin >> opt;
                            cin.ignore();

                            if (opt == 1)
                            { 
                                bool manageActive = true;
                                while (manageActive)
                                {
                                    cout << "\n1. Add Item\n2. Replenish Stock\n3. Discard Item\n4. Back\nChoose: ";
                                    int mOpt;
                                    cin >> mOpt;
                                    cin.ignore();
                                    if (mOpt == 1)
                                    {
                                        string itemName;
                                        double price;
                                        int stock;
                                        cout << "Item Name: ";
                                        getline(cin, itemName);
                                        cout << "Price: ";
                                        cin >> price;
                                        cout << "Stock: ";
                                        cin >> stock;
                                        cin.ignore();
                                        int itemId = 2000 + s.getItems().size() + 1;
                                        Item newItem(itemId, itemName, stock, price);
                                        s.addItem(newItem);
                                        cout << "Item added.\n";
                                    }
                                    else if (mOpt == 2)
                                    {
                                        int id, qty;
                                        cout << "Item ID to replenish: ";
                                        cin >> id;
                                        cout << "Quantity: ";
                                        cin >> qty;
                                        cin.ignore();
                                        for (auto &it : s.getItems())
                                        {
                                            if (it.getId() + s.getId() * 1000 == id)
                                            {
                                                it.setQuantity(it.getQuantity() + qty);
                                                cout << "Stock updated.\n";
                                                break;
                                            }
                                        }
                                    }
                                    else if (mOpt == 3)
                                    {
                                        int id;
                                        cout << "Item ID to discard: ";
                                        cin >> id;
                                        cin.ignore();
                                        auto &items = s.getItems();
                                        items.erase(remove_if(items.begin(), items.end(),
                                                              [&](Item &it)
                                                              { return it.getId() + s.getId() * 1000 == id; }),
                                                    items.end());
                                        cout << "Item discarded.\n";
                                    }
                                    else if (mOpt == 4)
                                        manageActive = false;
                                    else
                                        cout << "Invalid choice!\n";
                                }
                            }
                            else if (opt == 2)
                            { 
                                cout << "\n=== TRANSACTIONS ===\n";
                                vector<int> sellerTrans; 
                                int index = 1;

                                for (int i = 0; i < transactions.size(); ++i)
                                {
                                    auto &t = transactions[i];
                                    if (t.getSellerName() == s.getName())
                                    {
                                        cout << index++ << ". " << t.getId()
                                             << " | Buyer: " << t.getBuyerName()
                                             << " | Item: " << t.getItemName()
                                             << " | Total: " << t.getTotalPrice()
                                             << " | Status: " << t.getStatus() << endl;
                                        sellerTrans.push_back(i);
                                    }
                                }

                                if (sellerTrans.empty())
                                {
                                    cout << "No transactions found.\n";
                                    continue;
                                }

                                cout << "\nEnter transaction number to mark as COMPLETE (0 to back): ";
                                int choice;
                                cin >> choice;
                                cin.ignore();

                                if (choice > 0 && choice <= sellerTrans.size())
                                {
                                    Transaction &t = transactions[sellerTrans[choice - 1]];

                                    if (t.getStatus() == "PAID")
                                    {
                                        cout << "Mark this transaction as COMPLETE? (y/n): ";
                                        char yn;
                                        cin >> yn;
                                        cin.ignore();

                                        if (yn == 'y' || yn == 'Y')
                                        {
                                            t.setStatus("COMPLETE");
                                            cout << "Transaction " << t.getId() << " marked as COMPLETE!\n";
                                        }
                                    }
                                    else if (t.getStatus() == "COMPLETE")
                                    {
                                        cout << "Transaction already complete.\n";
                                    }
                                    else if (t.getStatus() == "CANCELLED")
                                    {
                                        cout << "Cannot complete a cancelled transaction.\n";
                                    }
                                    else
                                    {
                                        cout << "Transaction not eligible for completion.\n";
                                    }
                                }
                                else if (choice == 0)
                                {
                                    cout << "Returning to menu...\n";
                                }
                                else
                                {
                                    cout << "Invalid choice!\n";
                                }
                            }
                            else if (opt == 3)
                            { 
                                cout << "\nAvailable Items:\n";
                                for (auto &s2 : sellers)
                                    for (auto &item : s2.getItems())
                                        cout << "[ID:" << s2.getId() * 1000 + item.getId() << "] "
                                             << item.getName() << " | Price:" << item.getPrice()
                                             << " | Stock:" << item.getQuantity()
                                             << " | Seller:" << s2.getName() << endl;

                                int itemCode, qty;
                                cout << "Enter Item ID to buy (0 to cancel): ";
                                cin >> itemCode;
                                if (itemCode != 0)
                                {
                                    cout << "Quantity: ";
                                    cin >> qty;
                                    cin.ignore();
                                    bool found = false;
                                    for (auto &s2 : sellers)
                                    {
                                        for (auto &item : s2.getItems())
                                        {
                                            if (s2.getId() * 1000 + item.getId() == itemCode)
                                            {
                                                found = true;
                                                BankCustomer *buyerAcc = s.getAccount();
                                                BankCustomer *sellerAcc = s2.getAccount();
                                                double total = item.getPrice() * qty;
                                                if (!buyerAcc || !sellerAcc)
                                                    cout << "One or both accounts missing.\n";
                                                else if (buyerAcc->withdrawBalance(total))
                                                {
                                                    sellerAcc->addBalance(total);
                                                    item.setQuantity(item.getQuantity() - qty);
                                                    string tid = "T-" + to_string(transactionCount++);
                                                    transactions.emplace_back(tid, s.getName(), s.getId(),
                                                                              s2.getName(), s2.getId(), item.getName(), qty, total, "PAID", "Debit");
                                                    cout << "Payment successful! Total: " << total << endl;
                                                }
                                                else
                                                    cout << "Not enough balance.\n";
                                                break;
                                            }
                                        }
                                        if (found)
                                            break;
                                    }
                                    if (!found)
                                        cout << "Item not found.\n";
                                }
                            }
                            else if (opt == 4)
                            { 
                                cout << "\n=== Your Orders ===\n";
                                for (auto &t : transactions)
                                    if (t.getBuyerName() == s.getName())
                                        cout << t.getId() << " | " << t.getSellerName()
                                             << " | " << t.getItemName()
                                             << " | " << t.getTotalPrice()
                                             << " | " << t.getStatus() << endl;
                            }
                            else if (opt == 5)
                            { 
                                BankCustomer *acc = s.getAccount();
                                if (!acc)
                                {
                                    cout << "No bank account!\n";
                                    continue;
                                }
                                bool bankActive = true;
                                while (bankActive)
                                {
                                    cout << "\n--- Bank Account ---\n";
                                    cout << "Account ID: " << acc->getId() << endl;
                                    cout << "Balance: " << acc->getBalance() << endl;

                                    cout << "1. Top Up\n2. Withdraw\n3. View Cash Flow\n4. Back\nChoose: ";
                                    int choice2;
                                    cin >> choice2;
                                    cin.ignore();
                                    if (choice2 == 1)
                                    {
                                        double amt;
                                        cout << "Amount: ";
                                        cin >> amt;
                                        cin.ignore();
                                        acc->addBalance(amt, "Top Up");
                                        cout << "Top Up Successful!\n";
                                    }
                                    else if (choice2 == 2)
                                    {
                                        double amt;
                                        cout << "Amount: ";
                                        cin >> amt;
                                        cin.ignore();
                                        if (acc->withdrawBalance(amt, "Withdraw"))
                                            cout << "Withdraw Successful!\n";
                                        else
                                            cout << "Not enough balance!\n";
                                    }
                                    else if (choice2 == 3)
                                        acc->printCashFlow();
                                    else if (choice2 == 4)
                                        bankActive = false;
                                    else
                                        cout << "Invalid choice!\n";
                                }
                            }
                            else if (opt == 6)
                                active = false;
                            else
                                cout << "Invalid choice!\n";
                        }
                        loggedIn = true;
                        break;
                    }
                }

                if (!loggedIn)
                    cout << "Invalid credentials.\n";
            }
        }

        // ===== ADMIN LOGIN =====
        else if (choice == 3)
        {
            cout << "\nLogin as:\n1. Store Admin\n2. Bank Admin\nChoose: ";
            int adminType;
            cin >> adminType;
            cin.ignore();

            if (adminType == 1) 
            {
                string pass;
                cout << "=== STORE ADMIN LOGIN ===\n";
                cout << "Enter Admin Password(store123): ";
                cin >> pass;
                cin.ignore();

                if (pass == "store123") 
                {
                    cout << "Logged in as Store Admin\n";
                    bool storeActive = true;
                    while (storeActive)
                    {
                        cout << "\n--- STORE MENU ---\n";
                        cout << "1. List All Transactions (Last K Days)\n";
                        cout << "2. List Paid but Not Completed\n";
                        cout << "3. List Top M Frequent Items\n";
                        cout << "4. List Most Active Buyers (Today)\n";
                        cout << "5. List Most Active Sellers (Today)\n";
                        cout << "6. Logout\nChoose: ";
                        int opt;
                        cin >> opt;
                        cin.ignore();

                        switch (opt)
                        {
                        case 1:
                        {
                            cout << "\n=== Transactions (Last K Days) ===\n";
                            int K;
                            cout << "Enter K (Number of last days, enter 0 for ALL): ";
                            cin >> K;
                            cin.ignore();

                            bool found = false;

                            for (auto &t : transactions)
                            {
                                if (K == 0 || true) 
                                {
                                    cout << t.getDateString() << " | "
                                         << t.getId() << " | "
                                         << t.getBuyerName() << " -> "
                                         << t.getSellerName() << " | "
                                         << "$" << t.getTotalPrice()
                                         << " | Status: " << t.getStatus()
                                         << endl;
                                    found = true;
                                }
                            }

                            if (!found)
                                cout << (K == 0 ? "No transactions found." : "No transactions found within the last " + to_string(K) + " days.") << endl;

                            break;
                        }

                        case 2:
                        {
                            cout << "\n=== Paid but Not Completed ===\n";
                            for (auto &t : transactions)
                            {
                                if (t.getStatus() == "PAID")
                                {
                                    cout << t.getId() << " | "
                                         << t.getBuyerName() << " -> "
                                         << t.getSellerName() << " | "
                                         << t.getItemName() << " | "
                                         << t.getTotalPrice() << endl;
                                }
                            }
                            break;
                        }

                        case 3:
                        {
                            int M;
                            cout << "Enter M (Top M frequent items): ";
                            cin >> M;
                            cin.ignore();
                            cout << "\n=== Top " << M << " Frequent Items ===\n";

                            vector<pair<string, int>> itemCounts;
                            for (auto &s : sellers)
                                for (auto &it : s.getItems())
                                {
                                    int count = 0;
                                    for (auto &t : transactions)
                                        if (t.getItemName() == it.getName())
                                            count++;
                                    itemCounts.push_back({it.getName(), count});
                                }

                            sort(itemCounts.begin(), itemCounts.end(),
                                 [](auto &a, auto &b)
                                 { return a.second > b.second; });

                            for (int i = 0; i < min(M, (int)itemCounts.size()); i++)
                                cout << i + 1 << ". " << itemCounts[i].first
                                     << " (Sold " << itemCounts[i].second << " times)\n";

                            break;
                        }

                        case 4:
                        {
                            cout << "\n=== Most Active Buyers (Today) ===\n";
                            vector<pair<string, int>> userTotals;
                            for (auto &b : buyers)
                                userTotals.push_back({b.getName(), 0});
                            for (auto &t : transactions)
                            {
                                if (t.isToday())
                                {
                                    for (auto &ut : userTotals)
                                        if (ut.first == t.getBuyerName())
                                            ut.second++;
                                }
                            }
                            sort(userTotals.begin(), userTotals.end(),
                                 [](auto &a, auto &b)
                                 { return a.second > b.second; });
                            for (auto &ut : userTotals)
                                if (ut.second > 0)
                                    cout << ut.first << " - " << ut.second << " Transaction(s)\n";
                            break;
                        }

                        case 5:
                        {
                            cout << "\n=== Most Active Sellers (Today) ===\n";
                            vector<pair<string, int>> sellerTotals;
                            for (auto &s : sellers)
                                sellerTotals.push_back({s.getName(), 0});
                            for (auto &t : transactions)
                            {
                                if (t.isToday())
                                {
                                    for (auto &st : sellerTotals)
                                        if (st.first == t.getSellerName())
                                            st.second++;
                                }
                            }
                            sort(sellerTotals.begin(), sellerTotals.end(),
                                 [](auto &a, auto &b)
                                 { return a.second > b.second; });
                            for (auto &st : sellerTotals)
                                if (st.second > 0)
                                    cout << st.first << " - " << st.second << " Transaction(s)\n";
                            break;
                        }

                        case 6:
                        {
                            cout << "Logging out Store Admin...\n";
                            storeActive = false;
                            break;
                        }

                        default:
                        {
                            cout << "Invalid choice!\n";
                            break;
                        }
                    }
                }
            }
                else
                    cout << "Wrong Store Admin Password!\n";
            }
            else if (adminType == 2) 
            {
                string pass;
                cout << "=== BANK ADMIN LOGIN ===\n";
                cout << "Enter Admin Password (bank123): ";
                cin >> pass;
                cin.ignore();

                if (pass == "bank123") 
                {
                    cout << "Logged in as Bank Admin\n";
                    bool bankActive = true;
                    while (bankActive)
                    {
                        cout << "\n--- BANK MENU ---\n";
                        cout << "1. List All Transactions (Last 7 Days)\n";
                        cout << "2. List All Customers\n";
                        cout << "3. List Dormant Accounts (No transaction in a month)\n";
                        cout << "4. List Top Users (Today)\n";
                        cout << "5. Logout\n";
                        cout << "Choose: ";
                        int adminChoice;
                        cin >> adminChoice;
                        cin.ignore();

                        switch (adminChoice)
                        {
                        case 1:
                            cout << "\n=== Transactions (Last 7 Days) ===\n";
                            for (auto &t : transactions)
                            {
                                cout << t.getDateString() << " | "
                                     << t.getId() << " | "
                                     << t.getBuyerName() << " -> "
                                     << t.getSellerName() << " | "
                                     << t.getTotalPrice() << endl;
                            }
                            break;

                        case 2:
                            cout << "\n=== All Customers ===\n";
                            for (auto &b : buyers)
                                displayBuyer(b);
                            for (auto &s : sellers)
                                displaySeller(s);
                            break;

                        case 3:
                            cout << "\n=== Dormant Accounts ===\n";
                            for (auto &acc : bankAccounts)
                                if (acc->isDormant())
                                    acc->printInfo();
                            break;

                        case 4:
                        {
                            cout << "\n=== Top Users (Today) ===\n";
                            vector<pair<string, int>> userTotals;
                            for (auto &b : buyers)
                                userTotals.push_back({b.getName(), 0});
                            for (auto &t : transactions)
                                if (t.isToday())
                                    for (auto &ut : userTotals)
                                        if (ut.first == t.getBuyerName())
                                            ut.second++;
                            sort(userTotals.begin(), userTotals.end(),
                                 [](auto &a, auto &b)
                                 { return a.second > b.second; });
                            for (auto &ut : userTotals)
                                if (ut.second > 0)
                                    cout << ut.first << " | Transactions: " << ut.second << endl;
                            break;
                        }

                        case 5:
                            cout << "Logging out Bank Admin...\n";
                            bankActive = false;
                            break;

                        default:
                            cout << "Invalid choice.\n";
                            break;
                        }
                    }
                }
                else
                    cout << "Wrong Bank Admin Password!\n";
            }
            else
            {
                cout << "Invalid choice!\n";
            }
        }
    }
    return 0;
}