#include "Workspace.h"
#include "Strategy.h"

Workspace::Workspace(Strategy & strategy)
	: strategy_(strategy)
{ }

Workspace::~Workspace()
{
	std::cout << "Shutting down..." << std::endl;
	if(NULL != pInitiator_)
		pInitiator_->stop();
	delete pInitiator_;
	delete pLogFactory_;
	delete pMessageStoreFactory_;
	delete pSessionSettings_;
}

/// Establish FIX connections and do any other setup.
void Workspace::init(const std::string & configFile)
{
	// Boilerplate quickfix setup:
	pSessionSettings_ = new FIX::SessionSettings(configFile);
	pMessageStoreFactory_ = new FIX::FileStoreFactory(*pSessionSettings_);
	pLogFactory_ = new FIX::FileLogFactory(*pSessionSettings_);
	pInitiator_ = new FIX::ThreadedSocketInitiator(*this, *pMessageStoreFactory_, *pSessionSettings_, *pLogFactory_);
	pInitiator_->start();
	
	// Make sure all Sessions are logged on before we tell our Strategy it is OK to start:
	for(int i = 0; i < 10; ++i)
	{
		FIX::Session * pMarketDataSession = FIX::Session::lookupSession(marketDataSessionId_);
		FIX::Session * pOrderSession = FIX::Session::lookupSession(orderSessionId_);
		if(pMarketDataSession && pOrderSession)
		{
			if(pMarketDataSession->isLoggedOn() && pOrderSession->isLoggedOn())
			{
				strategy_.onInit(*this);
				return;
			}
		}
		std::cout << "[init] Waiting for all FIX Sessions to logon..." << std::endl;
		FIX::process_sleep(1);
	}

	throw std::runtime_error("[init] Fatal error: timed out waiting for all FIX Sessions to logon!");
}

void Workspace::sendMarketDataSubscription(const std::string & symbol)
{
    FIX42::MarketDataRequest msg;
	msg.setField(FIX::Symbol(symbol));
	FIX::Session::sendToTarget(msg, marketDataSessionId_);
}

void Workspace::sendOrder(const std::string & symbol, Workspace::Side side, double qty, double limitPrice)
{
    FIX42::NewOrderSingle msg(
        idHelper_.getNextOrderId(),
        FIX::HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION,
        symbol,
        Workspace::Side::BUY == side? FIX::Side_BUY: FIX::Side_SELL,
        FIX::TransactTime(),
        FIX::OrdType_MARKET
    );
    msg.set(FIX::OrdType(FIX::OrdType_LIMIT));
    msg.set(FIX::OrderQty(qty));
    msg.set(FIX::Price(limitPrice));
    FIX::Session::sendToTarget(msg, orderSessionId_);
}


//-----------------------------------------------------------------------------
// Called by QF whenever a Session is successfully logged on.
//
// We need to know which SessionID to use when sending orders vs sending a
// market data subscription.  The onLogon() callback is a good time to
// distinguish between the two Sessions.  We will use a couple of custom config
// file options to help us do that.
void Workspace::onLogon(const FIX::SessionID& sessionId)
{
	// Ask QF for the SessionSettings for this Session
	const FIX::Dictionary * pSettings = pInitiator_->getSessionSettings(sessionId);

	// Grab our custom "MyMarketDataSession" parameter (if it exists) from the SessionSettings
	if(pSettings->has("MyMarketDataSession") && pSettings->getBool("MyMarketDataSession"))
	{
		marketDataSessionId_ = sessionId;
		std::cout << "[onLogon] " << marketDataSessionId_ << " (MyMarketDataSession)" << std::endl;
	}

	// Grab our custom "MyOrderSession" parameter (if it exists) from the SessionSettings
	if(pSettings->has("MyOrderSession") && pSettings->getBool("MyOrderSession"))
	{
		orderSessionId_ = sessionId;
		std::cout << "[onLogon] " << orderSessionId_ << " (MyOrderSession)" << std::endl;
	}
}


//-----------------------------------------------------------------------------
// Called by QF right before an Admin-type message is sent to the counterparty.
//
// Examples of Admin-type messages are Logon, Logout, and Heartbeat.  We never
// send these types of messages ourselves -- QF does so for us automatically.
//
// However, we may need to customize the content of an Admin-type -- e.g., our
// counterparty may require us to specify a username or password.
void Workspace::toAdmin( FIX::Message& message, const FIX::SessionID& sessionId) 
{
	// First we have to figure out what the message is
	// Remember: Msgtype is in the header, not the body!
	FIX::MsgType msgType;
	message.getHeader().getField(msgType);
	
	if(FIX::MsgType_Logon == msgType)
	{
		const FIX::Dictionary * pSettings = pInitiator_->getSessionSettings(sessionId);
		if(pSettings->has("MyPassword"))
		{
			message.setField(FIX::RawData(pSettings->getString("MyPassword")));
			message.setField(FIX::RawDataLength(pSettings->getString("MyPassword").size()));
		}
		else
		{
			std::cout << "Warning: MyPassword not found in cfg file for session " << sessionId << std::endl;
		}
	}
}


//-----------------------------------------------------------------------------
// Called by QF after we call Session::sendToTarget() to send a message, right
// before the message is actually transmitted.  
//
// Examples of App-type messages that we might send are NewOrderSingle,
// OrderCancelRequest, and MarketDataRequest.
//
// The FIX Protocol guarantees in-order delivery of all messages.  For example,
// if you temporarily lose your network connection, FIX Protocol ensures that
// any messages that failed to make it to either counterparty will be
// re-transmitted.
//
// This is helpful behaviour when you are, say, receiving an ExecutionReport.  It
// is probably NOT helpful behaviour if say, you send a NewOrderSingle which gets
// re-transmitted an hour later when the network suddenly comes back up and the
// market has moved significantly!
//
// This is your chance to thwart the automatic resend behaviour if you do not
// want it.
void Workspace::toApp( FIX::Message& message, const FIX::SessionID& sessionID ) throw( FIX::DoNotSend )
{
	try
	{
		FIX::PossDupFlag possDupFlag;
		message.getHeader().getField(possDupFlag); // Throws FIX::FieldNotFound if PossDupFlag is not present.
		if(true == possDupFlag.getValue())
			throw FIX::DoNotSend();                // This will prevent QF from sending the message.
	}
	catch(FIX::FieldNotFound &)
	{ }
}

//-----------------------------------------------------------------------------
// Called by QF when an App-type message is received from the counterparty.
//
// We could just write all of our code for handling FIX messages right here in
// this callback.
//
// However, we would probably end up with a really, really long function if we
// did that.  Instead, we usually just call the QF crack() function here, which
// calls the proper onMessage() callback for whatever MsgType we just received.
void Workspace::fromApp( const FIX::Message& message, const FIX::SessionID& sessionID )
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{
	crack( message, sessionID );
}


//-----------------------------------------------------------------------------
// Called by QF when an Admin-type message is received from the counterparty.
//
// Examples of Admin-type messages are Logon, Logout, and Heartbeat.
//
// We almost never want or need to do anything in this callback.  QF handles
// these types of messages for us automatically.
void Workspace::fromAdmin( const FIX::Message&, const FIX::SessionID& ) throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon )
{ }


//-----------------------------------------------------------------------------
// Called by QF when a Session is created (but before it is logged on).
// We do not usually do anything here.
void Workspace::onCreate( const FIX::SessionID& sessionId)
{ }


//-----------------------------------------------------------------------------
// Called by QF when a Session is either logged out or suddenly disconnected.
// We can use this to notify our application of a lost connection.
void Workspace::onLogout( const FIX::SessionID& sessionId )
{
	std::cout << "[onLogout] " << sessionId << std::endl;
}


void Workspace::onMessage(const FIX42::ExecutionReport& msg, const FIX::SessionID& )
{
	FIX::ExecType execType;
	FIX::Symbol symbol;
	FIX::Side side;
	
	// See what kind of execution report this is:
	msg.get(execType);
	if(FIX::ExecType_FILL == execType.getValue() || FIX::ExecType_PARTIAL_FILL == execType.getValue())
	{
		FIX::LastShares lastQty;
		FIX::LastPx lastPx;
		msg.get(symbol);
		msg.get(side);
		msg.get(lastQty);
		msg.get(lastPx);

		// Let our Strategy know about the fill:
		if(FIX::Side_BUY == side)
			strategy_.onOrderFill(*this, Workspace::Side::BUY, lastQty.getValue(), lastPx.getValue());
		else if(FIX::Side_SELL == side)
			strategy_.onOrderFill(*this, Workspace::Side::SELL, lastQty.getValue(), lastPx.getValue());
	}
	else if(FIX::ExecType_REJECTED == execType.getValue())
	{
		FIX::OrderQty orderQty;
		msg.get(symbol);
		msg.get(side);
		msg.get(orderQty);

		// Let our Strategy know about the reject
		if(FIX::Side_BUY == side)
			strategy_.onOrderReject(*this, Workspace::Side::BUY, orderQty.getValue());
		else if(FIX::Side_SELL == side)
			strategy_.onOrderReject(*this, Workspace::Side::SELL, orderQty.getValue());

		std::cout << std::endl << "RECEIVED REJECT: " << msg << std::endl;
	}
	else if(FIX::ExecType_NEW == execType.getValue())
	{
		// Our order was accepted (but has not yet been filled)
	}
	else
	{
		std::cout << "Not sure what to do with ExecutionReport with ExecType=" << execType << ": " << msg << std::endl;
	}
}


void Workspace::onMessage(const FIX42::MarketDataSnapshotFullRefresh& msg, const FIX::SessionID& )
{
    FIX::MDEntryType type;
    FIX::MDEntryPx px;
    FIX::MDEntrySize qty;

    msg.getField(type);
    msg.getField(px);
    msg.getField(qty);

    if(FIX::MDEntryType_TRADE == type.getValue())
    {
        strategy_.onLastTradeUpdate(*this, qty.getValue(), px.getValue());
    } 
    else
    {
        std::cout << "Unknown MDEntryType: " << type << std::endl;
    }
}

void Workspace::onMessage(const FIX42::MarketDataIncrementalRefresh& msg, const FIX::SessionID& )
{
	FIX::NoMDEntries noMDEntries;
	msg.get(noMDEntries);
	for (int i = 1; i <= noMDEntries; ++i)
	{
		FIX42::MarketDataIncrementalRefresh::NoMDEntries group;
		FIX::MDEntryType type;
		FIX::MDEntryPx px;
		FIX::MDEntrySize qty;
		FIX::MDUpdateAction action;
		
		msg.getGroup(i, group);
		group.get(type);
		group.get(px);
		group.get(qty);
		group.get(action);

		if(FIX::MDUpdateAction_NEW == action.getValue() || FIX::MDUpdateAction_CHANGE == action.getValue())
		{
			if(FIX::MDEntryType_BID == type.getValue())
			{
				strategy_.onBestBidUpdate(*this, qty.getValue(), px.getValue());
			}
			else if (FIX::MDEntryType_OFFER == type.getValue())
			{
				strategy_.onBestOfferUpdate(*this, qty.getValue(), px.getValue());
			}
			else if(FIX::MDEntryType_TRADE == type.getValue())
			{
				strategy_.onLastTradeUpdate(*this, qty.getValue(), px.getValue());
			} 
			else
			{
				std::cout << "Unknown MDEntryType: " << type << std::endl;
			}
		}
	}
}

void Workspace::onMessage(const FIX42::MarketDataRequestReject& msg, const FIX::SessionID&)
{
	FIX::MDReqID reqId;
	FIX::MDReqRejReason reason;
	FIX::Text text;

	if(msg.isSetField(FIX::FIELD::MDReqID))
		msg.get(reqId);
	if(msg.isSetField(FIX::FIELD::MDReqRejReason))
		msg.get(reason);
	if(msg.isSetField(FIX::FIELD::Text))
		msg.get(text);
	
	std::cout << "MarketDataRequestReject: MDReqID=" << reqId << ", reason=" << reason << ", text=" << text << std::endl;
}

/// C++ magic. Useful for printing.
std::ostream & operator<<(std::ostream & out, const Workspace::Side & side)
{
    switch(side)
    {
        case Workspace::BUY:
            out << "BUY";
            return out;
        case Workspace::SELL:
            out << "SELL";
            return out;
        default:
            throw std::invalid_argument("Unknown side");
    }
}

