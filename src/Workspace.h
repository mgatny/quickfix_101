#ifndef WORKSPACE_H_
#define WORKSPACE_H_

#include <string>
#include <iostream>
#include <quickfix/Application.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/ThreadedSocketInitiator.h>
#include <quickfix/fix42/MarketDataRequest.h>
#include <quickfix/fix42/MarketDataRequestReject.h>
#include <quickfix/fix42/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix42/MarketDataIncrementalRefresh.h>
#include <quickfix/fix42/NewOrderSingle.h>
#include <quickfix/fix42/ExecutionReport.h>
#include "IdHelper.h"

class Strategy;

/// A simple interface that allows our Strategy to subscribe to market data and send orders.
class Workspace : public FIX::Application,
                  public FIX::MessageCracker
{
  public:
    enum Side { BUY = '1', SELL = '2' };

    explicit Workspace(Strategy & strategy);
    ~Workspace();

    /// Establish FIX connections and do any other setup.
    void init(const std::string & configFile);

    /// Subscribe to market data updates for an instrument.
    void sendMarketDataSubscription(const std::string & symbol);

    /// Send a limit order
    void sendOrder(const std::string & symbol, Side side, double qty, double limitPrice);

  private: // quickfix callbacks:
    void onCreate(const FIX::SessionID&);
    void onLogon(const FIX::SessionID&);
    void onLogout(const FIX::SessionID&);
    void toAdmin(FIX::Message&, const FIX::SessionID&);
    void toApp(FIX::Message&, const FIX::SessionID&) throw(FIX::DoNotSend);
    void fromAdmin(const FIX::Message&, const FIX::SessionID&) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon);
    void fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType);
    void onMessage(const FIX42::ExecutionReport&, const FIX::SessionID&);
    void onMessage(const FIX42::MarketDataSnapshotFullRefresh&, const FIX::SessionID&);
    void onMessage(const FIX42::MarketDataIncrementalRefresh& msg, const FIX::SessionID&);
    void onMessage(const FIX42::MarketDataRequestReject&, const FIX::SessionID&);

  private:
    Strategy & strategy_;
    IdHelper idHelper_;
    FIX::SessionID marketDataSessionId_;
    FIX::SessionID orderSessionId_;
    FIX::MessageStoreFactory * pMessageStoreFactory_;
    FIX::FileLogFactory * pLogFactory_;
    FIX::SessionSettings * pSessionSettings_;
    FIX::ThreadedSocketInitiator * pInitiator_;
};

std::ostream & operator<<(std::ostream & out, const Workspace::Side & side);

#endif  // WORKSPACE_H_
