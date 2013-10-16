#include <iostream>
#include <sstream>
#include <fstream>
#include "IdHelper.h"


IdHelper::IdHelper()
    : mutex_(),
      mdRequestId_(0),
      orderId_(readOrderIdFromFile())
{ }

IdHelper::~IdHelper()
{
    writeOrderIdToFile();
}

void IdHelper::writeOrderIdToFile()
{
    FIX::Locker lock(mutex_);
    std::ofstream file("orderid.txt", std::ios::trunc);
    file << orderId_ << '\n';
}

int IdHelper::readOrderIdFromFile()
{
    FIX::Locker lock(mutex_);
    std::ifstream file("orderid.txt");
    std::string s;
    std::getline(file, s);
    return s.empty() ? 0 : std::stoi(s);
}

std::string IdHelper::getNextOrderId()
{
    FIX::Locker lock(mutex_);
    orderId_++;
    std::stringstream s;
    s << orderId_;
    return s.str();
}

std::string IdHelper::getCurrentOrderId() const
{
    FIX::Locker lock(mutex_);
    std::stringstream s;
    s << orderId_;
    return s.str();
}

std::string IdHelper::getNextMarketDataRequestId()
{
    FIX::Locker lock(mutex_);
    mdRequestId_++;
    std::stringstream s;
    s << mdRequestId_;
    return s.str();
}

