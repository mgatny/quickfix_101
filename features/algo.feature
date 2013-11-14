Feature: Algo sends orders based on Last Trade data

Background:
    Given my app is running and it is subscribed to "AAPL"

Scenario: Receive market data, send order
        #    When I send the following FIX message from agent "marketdata": 
        #"""
        #8=FIX.4.29=9235=W34=249=MARKETDATA52=20131113-22:32:22.01756=CLIENT155=AAPL269=2270=10001271=50110=202
        #8=FIX.4.29=9235=W34=349=MARKETDATA52=20131113-22:32:23.82656=CLIENT155=AAPL269=2270=10002271=50210=214
        #8=FIX.4.29=9235=W34=449=MARKETDATA52=20131113-22:32:26.19056=CLIENT155=AAPL269=2270=10003271=50310=214
        #8=FIX.4.29=9235=W34=549=MARKETDATA52=20131113-22:32:28.11856=CLIENT155=AAPL269=2270=10004271=50410=219
        #"""

    Then I should receive a message over FIX with agent "executor"
    And the FIX message should have the following:
        | MsgType  | D       |
        | Symbol   | "AAPL"  |
        | Side     | "BUY"   |
        | OrderQty | 1       |
        | Price    | 10004   |
        | OrdType  | "LIMIT" |

