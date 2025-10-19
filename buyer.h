#ifndef BUYER_H
#define BUYER_H

#include <string>
#include "bank_customer.h"
using namespace std;

class Buyer {
private:
    int id;
    string name;
    string password;
    BankCustomer* account;

public:
    Buyer(int id, const string& name, const string& password, BankCustomer* account);

    int getId() const;
    string getName() const;
    string getPassword() const; 
    BankCustomer* getAccount() const;
};

#endif
