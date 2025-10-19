#ifndef SELLER_H
#define SELLER_H

#include <iostream>
#include <string>
#include <vector>
#include "bank_customer.h"
#include "item.h"

using namespace std;

class Seller {
private:
    int id;
    string name;
    string password;
    BankCustomer* account;
    vector<Item> items; 

public:
    Seller(int id, const string& name, const string& password, BankCustomer* account)
        : id(id), name(name), password(password), account(account) {}

    int getId() const { return id; }
    string getName() const { return name; }
    string getPassword() const { return password; }
    BankCustomer* getAccount() const { return account; }

    void addItem(const Item& newItem) {
        items.push_back(newItem);
    }

    void displayItems() const {
        if (items.empty()) {
            cout << "Belum ada item yang dijual.\n";
            return;
        }
        cout << "Daftar barang milik " << name << ":\n";
        for (const auto& item : items) {
            cout << "ID: " << item.getId()
                 << " | Nama: " << item.getName()
                 << " | Jumlah: " << item.getQuantity()
                 << " | Harga: " << item.getPrice() << endl;
        }
    }

    vector<Item>& getItems() { 
    return items; 
    }
    
    const vector<Item>& getItems() const { 
    return items;
    }

};

#endif
