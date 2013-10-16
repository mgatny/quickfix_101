#include "Strategy.h"

VwapStrategy::VwapStrategy(const std::string & symbol)
	: symbol_(symbol),
	  signedLongPositionLimit_(3),
	  signedShortPositionLimit_(-3),
	  position_(symbol, 1),
      vwap_(5)
{ }

void VwapStrategy::onInit(Workspace & workspace)
{
	workspace.sendMarketDataSubscription(symbol_);
}

void VwapStrategy::onLastTradeUpdate(Workspace & workspace, double qty, double px)
{
	// Let our Vwap object know that we got a trade notification:
	vwap_.addTrade(px, qty);

	// Tip: If you only consider your net position qty, you risk exceeding your position limits due to an in-flight
	// order getting filled. To play it safe, consider your combined working qty and net qty as your "actual" position.
	const int safeBuyQty = signedLongPositionLimit_ - position_.signedNetQty() - position_.signedWorkingLongQty();
	const int safeSellQty = 0 - signedShortPositionLimit_ + position_.signedNetQty() + position_.signedWorkingShortQty();
	const double currentVwap = vwap_.calculateVwap();
	std::cout << "[MarketDataUpdate] TRADE " << px << " / " << qty << "    VWAP(" << vwap_.getNumTrades() << ") = " << currentVwap << std::endl;

	if(px < (currentVwap - 1) && safeSellQty > 0)
	{	
		std::cout << std::endl << "    SELL SIGNAL! lastPx=" << px << ", vwap=" << currentVwap << ", safeSellQty=" << safeSellQty << std::endl;
		position_.sell(1);
		workspace.sendOrder(symbol_, Workspace::Side::SELL, 1, px);
	}
	else if(px > (currentVwap + 1) && safeBuyQty > 0)
	{
		std::cout << std::endl << "    BUY SIGNAL! lastPx=" << px << ", vwap=" << currentVwap << ", safeBuyQty=" << safeBuyQty << std::endl;
		position_.buy(1);
		workspace.sendOrder(symbol_, Workspace::Side::BUY, 1, px);
	}
}

void VwapStrategy::onOrderFill(Workspace & workspace, Workspace::Side side, double qty, double px)
{
	// Let our Position object know about the fill
	if(Workspace::Side::BUY == side)
		position_.buyFill((int)qty, px);
	else if(Workspace::Side::SELL == side)
		position_.sellFill((int)qty, px);

	std::cout << std::endl << "RECEIVED FILL: side=" << side << ", price=" << px << ", qty=" << qty << ", " << position_ << std::endl;
}

void VwapStrategy::onOrderReject(Workspace & workspace, Workspace::Side side, double qty)
{
	// Let our Position object know about the reject by reversing out the working qty
	if(FIX::Side_BUY == side)
		position_.sell((int)qty);
	else if(FIX::Side_SELL == side)
		position_.buy((int)qty);
}

void VwapStrategy::onBestBidUpdate(Workspace & workspace, double qty, double px)
{
	std::cout << "MarketDataUpdate:   BID " << px << " / " << qty << std::endl;
}

void VwapStrategy::onBestOfferUpdate(Workspace & workspace, double qty, double px)
{
	std::cout << "MarketDataUpdate: OFFER " << px << " / " << qty << std::endl;
}
