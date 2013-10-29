#ifndef STRATEGY_H_
#define STRATEGY_H_

#include "Workspace.h"
#include "Position.h"
#include "Vwap.h"

/// A place to implement our strategy logic, receive market data, receive fills, etc.
struct Strategy
{
    virtual ~Strategy() { }
    
    /// Called once by Workspace to let us know that it is done initializing itself.
    virtual void onInit(Workspace & workspace) = 0;
    
    /// Called by Workspace to let us know when the best bid changes.
    virtual void onBestBidUpdate(Workspace & workspace, double qty, double px) = 0;
    
    /// Called by Workspace to let us know when the best offer changes.
    virtual void onBestOfferUpdate(Workspace & workspace, double qty, double px) = 0;
    
    /// Called by Workspace whenever the trade ticker changes
    virtual void onLastTradeUpdate(Workspace & workspace, double qty, double px) = 0;
    
    /// Called by Workspace to let us know that an order was filled.
    virtual void onOrderFill(Workspace & workspace, Workspace::Side side, double qty, double px) = 0;
    
    /// Called by Workspace to let us know that an order was rejected.
    virtual void onOrderReject(Workspace & workspace, Workspace::Side side, double qty) = 0;
};

class VwapStrategy : public Strategy
{
  public:
    /// Create a strategy that trades the given symbol
    explicit VwapStrategy(const std::string & symbol);

    /// Strategy interface methods:
    void onInit(Workspace & workspace);
    void onBestBidUpdate(Workspace & workspace, double qty, double px);
    void onBestOfferUpdate(Workspace & workspace, double qty, double px);
    void onLastTradeUpdate(Workspace & workspace, double qty, double px);
    void onOrderFill(Workspace & workspace, Workspace::Side side, double qty, double px);
    void onOrderReject(Workspace & workspace, Workspace::Side side, double qty);

  private:
    const std::string symbol_;
    const std::string maturityMonthYear_;
    const std::string account_;
    const int signedLongPositionLimit_;
    const int signedShortPositionLimit_;
    Position position_;
    Vwap vwap_;
};

#endif  // STRATEGY_H_
