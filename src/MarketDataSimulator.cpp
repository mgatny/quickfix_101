#include "MarketDataSimulator.h"
#include "Strategy.h"

MarketDataSimulator::MarketDataSimulator() :
    lastPx_(10000),
    lastQty_(500),
    pxDistance_(0),
    pxVelocity_(1)
{ }

MarketDataSimulator::~MarketDataSimulator()
{
	std::cout << "Shutting down..." << std::endl;
	if(NULL != pAcceptor_)
		pAcceptor_->stop();
	delete pAcceptor_;
	delete pLogFactory_;
	delete pMessageStoreFactory_;
	delete pSessionSettings_;
}

/// Establish FIX connections and do any other setup.
void MarketDataSimulator::init(const std::string & configFile)
{
	// Boilerplate quickfix setup:
	pSessionSettings_ = new FIX::SessionSettings(configFile);
	pMessageStoreFactory_ = new FIX::FileStoreFactory(*pSessionSettings_);
	pLogFactory_ = new FIX::FileLogFactory(*pSessionSettings_);
	pAcceptor_ = new FIX::SocketAcceptor(*this, *pMessageStoreFactory_, *pSessionSettings_, *pLogFactory_);
	pAcceptor_->start();
}

void MarketDataSimulator::onMessage(const FIX42::MarketDataRequest & msg, const FIX::SessionID & sessionId)
{
    FIX::Locker lock(mutex_);
    sessionId_ = sessionId;
    msg.getField(symbol_);
    std::cout << "[onMessage] got market data request from " << sessionId_ << std::endl;
}

bool MarketDataSimulator::buildLastTradeMessage(FIX::Message & msg)
{
    FIX::Locker lock(mutex_);
    if(symbol_.getValue().empty())
        return false;

    if(pxDistance_ >= 26) // change direction
    {
        pxVelocity_ = -1;
        pxDistance_ = 0;
    }
    
    if(pxDistance_ <= -14) // change direction
    {
        pxVelocity_ = 2;
        pxDistance_ = 0;
    }

    lastPx_ += pxVelocity_;
    lastQty_ += pxVelocity_;
    pxDistance_ += pxVelocity_;

    msg.setField(symbol_);
    msg.setField(FIX::MDEntryPx(lastPx_));
    msg.setField(FIX::MDEntrySize(lastQty_));
    msg.setField(FIX::MDEntryType(FIX::MDEntryType_TRADE));
    
    return true;
}

void MarketDataSimulator::sendLastTradeMessage()
{
    FIX42::MarketDataSnapshotFullRefresh msg;
    if(buildLastTradeMessage(msg))
        FIX::Session::sendToTarget(msg, sessionId_);
}

void MarketDataSimulator::fromApp( const FIX::Message& message, const FIX::SessionID& sessionID )
throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{
	crack(message, sessionID);
}

void MarketDataSimulator::onLogout(const FIX::SessionID& sessionId)
{
    std::cout << "[onLogout] " << sessionId << std::endl;
    
    FIX::Locker lock(mutex_);
    symbol_ = FIX::Symbol("");
    sessionId_ = FIX::SessionID();
}


void MarketDataSimulator::toAdmin(FIX::Message&, const FIX::SessionID&) { }
void MarketDataSimulator::toApp(FIX::Message&, const FIX::SessionID& sessionID) throw(FIX::DoNotSend) { }
void MarketDataSimulator::fromAdmin(const FIX::Message&, const FIX::SessionID&) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) { }
void MarketDataSimulator::onCreate(const FIX::SessionID& sessionId) { }
void MarketDataSimulator::onLogon(const FIX::SessionID& sessionId) { std::cout << "[onLogon] " << sessionId << std::endl; }


bool random_timespec(timespec & t)
{
    const long x = random();
    
    t.tv_sec = x / 1000000000L;
    if(0 != t.tv_sec)
    {
        t.tv_nsec = 750 * 1000 * 1000;
        t.tv_sec = 0;
        return false;
    }

    t.tv_nsec = x / 2;
    return true;
}

int main(int argc, char* argv[])
{
    srandom(time(NULL));

    try
    {
        // parse the command line args
        if ( argc != 2 )
        {
            throw std::invalid_argument(std::string("usage: ") + argv[0] + " CFG_FILE");
        }
        const std::string cfgFileName = argv[1];

        MarketDataSimulator fakeMarketData;
        fakeMarketData.init(cfgFileName);

        // quickfix is now running in the background
        std::cout << std::endl << "Hit CTRL-c to quit..." << std::endl;
        timespec t;
        while(1)
        {
            if(random_timespec(t))
                fakeMarketData.sendLastTradeMessage();
            nanosleep(&t, NULL);
        }
    }
    catch(const std::exception & e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}

