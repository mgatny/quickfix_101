#include "Vwap.h"

Vwap::Vwap(std::size_t numTradesPerPeriod) :
    numTradesPerPeriod_(numTradesPerPeriod)
{ }

void Vwap::addTrade(double price, double qty)
{
    Trade t = { price, qty };
    trades_.push_back(t);
    while(trades_.size() > numTradesPerPeriod_)
        trades_.pop_front();
}

double Vwap::calculateVwap() const
{
    if(trades_.empty())
        return 0.0;

    double totalValue = 0.0;
    int totalQty = 0;

    for(std::list<Trade>::const_iterator t = trades_.begin(); t != trades_.end(); ++t)
    {
        totalValue += (t->price * t->qty);
        totalQty += (static_cast<int>(t->qty));
    }

    return totalValue / static_cast<double>(totalQty);
}

void Vwap::reset()
{
    trades_.clear();
}

std::size_t Vwap::getNumTrades() const
{
    return trades_.size();
}
