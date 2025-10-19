#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
using namespace std;

class Transaction {
private:
    string tid;
    string buyerName;
    int buyerBankId;
    string sellerName;
    int sellerBankId;
    string itemName;
    int quantity;
    double totalPrice;
    string status;
    time_t date;
    string type; 

public:
    Transaction(string tid, string buyer, int buyerId,
                string seller, int sellerId,
                string itemName, int quantity,
                double totalPrice, string status, string type)
        : tid(tid), buyerName(buyer), buyerBankId(buyerId),
          sellerName(seller), sellerBankId(sellerId),
          itemName(itemName), quantity(quantity),
          totalPrice(totalPrice), status(status), type(type)
    {
        date = time(nullptr);
    }

    void setStatus(const string &newStatus) { status = newStatus; }

    string getId() const { return tid; }
    string getBuyerName() const { return buyerName; }
    int getBuyerBankId() const { return buyerBankId; }
    string getSellerName() const { return sellerName; }
    int getSellerBankId() const { return sellerBankId; }
    string getItemName() const { return itemName; }
    int getQuantity() const { return quantity; }
    double getTotalPrice() const { return totalPrice; }
    string getStatus() const { return status; }
    string getType() const { return type; }

    string getDateString() const {
        char buffer[20];
        tm* timeinfo = localtime(&date);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
        return string(buffer);
    }

    bool isToday() const {
        tm* now = localtime(new time_t(time(nullptr)));
        tm* txn = localtime(&date);
        return (now->tm_year == txn->tm_year &&
                now->tm_mon == txn->tm_mon &&
                now->tm_mday == txn->tm_mday);
    }
};

#endif
