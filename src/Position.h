#ifndef POSITION_H_
#define POSITION_H_

#include <string>
#include <iostream>
#include <quickfix/Mutex.h> // borrowing qf's mutex for convenience

/// Synchronized position.
class Position
{
public:
	virtual ~Position();
	Position();
	Position(const std::string & instrumentId, const int contractSize);

	/// Add to the working long qty.
	void buy(const int unsignedQty);
	/// Add to the working short qty.
	void sell(const int unsignedQty);
	/// Update the net qty and cost basis, and subtract from the working long qty. 
	void buyFill(const int unsignedQty, const double price);
	/// Update the net qty and cost basis, and subtract from the working short qty.
	void sellFill(const int unsignedQty, const double price);
	std::string toString() const;

	// accessors:
	int contractSize() const;
	int signedWorkingLongQty() const;
	int signedWorkingShortQty() const;
	int signedNetQty() const;
	double costBasis() const;
	const std::string & instrumentId() const;

protected:
	int signedWorkingLongQty_;
	int signedWorkingShortQty_;
	int signedNetQty_;
	double costBasis_;
	int contractSize_;
	const std::string instrumentId_;
    // borrowing qf's mutex for convenience
	mutable FIX::Mutex mutex_;
};

std::ostream & operator<<(std::ostream & out, const Position &);

#endif  // POSITION_H_
