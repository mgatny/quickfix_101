#ifndef VWAP_H_
#define VWAP_H_

#include <list>

/// Volume-weighted average price.
/// Unsynchronized.
class Vwap
{
  public:
    struct Trade
    {
        const double price;
        const double qty;
    };

    Vwap(std::size_t numTradesPerPeriod);
    void addTrade(double price, double qty);
    double calculateVwap() const;
    std::size_t getNumTrades() const;
    void reset();

  protected:
    const std::size_t numTradesPerPeriod_;
    std::list<Trade> trades_;
};

#endif  // VWAP_H_
