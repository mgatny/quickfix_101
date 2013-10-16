#include "Position.h"
#include <sstream>

Position::~Position()
{ }

Position::Position()
	: signedWorkingLongQty_(0), signedWorkingShortQty_(0), signedNetQty_(0), costBasis_(0), contractSize_(1)
{ }

Position::Position(const std::string & instrumentId, const int contractSize)
	: signedWorkingLongQty_(0), signedWorkingShortQty_(0), signedNetQty_(0), costBasis_(0), contractSize_(contractSize), instrumentId_(instrumentId)
{ }

void Position::buy(const int unsignedQty)
{
	FIX::Locker lock(mutex_);
	signedWorkingLongQty_ += unsignedQty;
}

void Position::sell(const int unsignedQty)
{
	FIX::Locker lock(mutex_);
	signedWorkingShortQty_ -= unsignedQty;
}

void Position::buyFill(const int unsignedQty, const double price)
{	
	FIX::Locker lock(mutex_);
	const double lastCost = price * unsignedQty * contractSize_;	
	signedNetQty_ += unsignedQty;
	signedWorkingLongQty_ -= unsignedQty;
	costBasis_ -= lastCost; 
}
	
void Position::sellFill(const int unsignedQty, const double price)
{
	FIX::Locker lock(mutex_);
	const double lastCost = price * unsignedQty * contractSize_;	
	signedNetQty_ -= unsignedQty;
	signedWorkingShortQty_ += unsignedQty;
	costBasis_ += lastCost; 
}

std::string Position::toString() const
{
	FIX::Locker lock(mutex_);
	std::ostringstream out;
	out << "POSITION {" << std::endl
		<< "  instrumentId          => " << instrumentId_          << std::endl
	    << "  signedWorkingLongQty  => " << signedWorkingLongQty_  << std::endl
	    << "  signedWorkingShortQty => " << signedWorkingShortQty_ << std::endl
	    << "  signedNetQty          => " << signedNetQty_          << std::endl
	    << "  costBasis             => " << costBasis_             << std::endl
	    << "}";
	return out.str();
}

int Position::contractSize() const          { FIX::Locker lock(mutex_); return contractSize_; }
int Position::signedWorkingLongQty() const  { FIX::Locker lock(mutex_); return signedWorkingLongQty_; }
int Position::signedWorkingShortQty() const { FIX::Locker lock(mutex_); return signedWorkingShortQty_; }
int Position::signedNetQty() const          { FIX::Locker lock(mutex_); return signedNetQty_; }
double Position::costBasis() const          { FIX::Locker lock(mutex_); return costBasis_; }
const std::string & Position::instrumentId() const { return instrumentId_; }

std::ostream & operator<<(std::ostream & out, const Position & position)
{
	return out << position.toString();
}

