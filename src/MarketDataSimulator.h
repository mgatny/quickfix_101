#ifndef MARKETDATASIMULATOR_H_
#define MARKETDATASIMULATOR_H_

#include <string>
#include <iostream>
#include <quickfix/Application.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/SocketAcceptor.h>
#include <quickfix/fix42/MarketDataRequest.h>

/// Generate fake market data
class MarketDataSimulator : public FIX::Application,
                            public FIX::MessageCracker
{
public:
	MarketDataSimulator();
	~MarketDataSimulator();

	/// Create FIX acceptor and do any other setup.
	void init(const std::string & configFile);

    void sendLastTradeMessage();

private: // quickfix callbacks:
	void onCreate(const FIX::SessionID&);
	void onLogon(const FIX::SessionID&);
	void onLogout(const FIX::SessionID&);
	void toAdmin(FIX::Message&, const FIX::SessionID&);
	void toApp(FIX::Message&, const FIX::SessionID&) throw(FIX::DoNotSend);
	void fromAdmin(const FIX::Message&, const FIX::SessionID&) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon);
	void fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType);
	void onMessage(const FIX42::MarketDataRequest& msg, const FIX::SessionID&);

    bool buildLastTradeMessage(FIX::Message & msg);

    FIX::MessageStoreFactory * pMessageStoreFactory_;
    FIX::FileLogFactory * pLogFactory_;
    FIX::SessionSettings * pSessionSettings_;
    FIX::SocketAcceptor * pAcceptor_;
    FIX::Mutex mutex_;
    FIX::SessionID sessionId_;
    FIX::Symbol symbol_;
    int lastPx_;
    int lastQty_;
    int pxDistance_;
    int pxVelocity_;
};

#endif  // MARKETDATASIMULATOR_H_
