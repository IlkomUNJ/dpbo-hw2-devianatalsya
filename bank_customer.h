#ifndef BANK_CUSTOMER_H
#define BANK_CUSTOMER_H

#include <string>
#include <ctime>
#include <vector>
#include <iostream>
#include "transaction.h"

using namespace std;

class BankCustomer {
private:
    int id;
    string name;
    double balance;
    time_t lastTransaction;
    vector<Transaction> cashFlow; 

public:
    BankCustomer(int id, const string& name, double balance)
        : id(id), name(name), balance(balance) {
        lastTransaction = time(nullptr);
    }

    int getId() const { return id; }
    string getName() const { return name; }
    double getBalance() const { return balance; }

    void addBalance(double amount, const string& note = "Top Up") {
        balance += amount;
        lastTransaction = time(nullptr);
        cashFlow.emplace_back("T-" + to_string(id), name, id, "", 0, note, 1, amount, "PAID", "Credit");
    }

    bool withdrawBalance(double amount, const string& note = "Withdraw") {
        if (balance >= amount) {
            balance -= amount;
            lastTransaction = time(nullptr);
            cashFlow.emplace_back("T-" + to_string(id), name, id, "", 0, note, 1, amount, "PAID", "Debit");
            return true;
        }
        return false;
    }

    bool isDormant() const {
        time_t now = time(nullptr);
        double diff = difftime(now, lastTransaction);
        return diff >= 30*24*3600; 
    }

    void printInfo() const {
        printf("ID: %d | Name: %s | Balance: %.2f\n", id, name.c_str(), balance);
    }

    void printCashFlow() const {
        cout << "\n=== CASH FLOW (Last 1 Month) ===\n";
        cout << "------------------------------------------------\n";
        cout << "Date       | Type   | Amount  | Note\n";
        cout << "------------------------------------------------\n";
        for (const auto& t : cashFlow) {
            cout << t.getDateString() << " | " << t.getType()
                 << " | " << t.getTotalPrice() << " | " << t.getItemName() << endl;
        }
        cout << "------------------------------------------------\n";
    }
};

#endif
