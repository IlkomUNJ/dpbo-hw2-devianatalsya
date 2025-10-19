#include "buyer.h"

Buyer::Buyer(int id, const string& name, const string& password, BankCustomer* account)
    : id(id), name(name), password(password), account(account) {}

int Buyer::getId() const { return id; }
string Buyer::getName() const { return name; }
string Buyer::getPassword() const { return password; }
BankCustomer* Buyer::getAccount() const { return account; }
