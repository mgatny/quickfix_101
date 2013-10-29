#ifndef IDHELPER_H_
#define IDHELPER_H_

#include <string>
#include <quickfix/Mutex.h> // borrowing qf's mutex for convenience

/// Creates unique IDs for orders and market data subscriptions.
/// Synchronized.
class IdHelper
{
  public:
    IdHelper();
    ~IdHelper();
    std::string getNextOrderId();
    std::string getCurrentOrderId() const;
    std::string getNextMarketDataRequestId();

  private:
    void writeOrderIdToFile();
    int readOrderIdFromFile();
    IdHelper(const IdHelper & rhs);
    IdHelper & operator=(const IdHelper & rhs);

  private:
    // borrowing qf's mutex for convenience
    mutable FIX::Mutex mutex_;
    int mdRequestId_;
    int orderId_;
};

#endif  // IDHELPER_H_
