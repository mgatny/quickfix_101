AM_CXXFLAGS = -Wall -std=c++11

nobase_include_HEADERS = \
    src/ExecutorApp.h    \
    src/IdHelper.h       \
    src/Position.h       \
    src/Workspace.h      \
    src/Strategy.h       \
    src/Vwap.h

bin_PROGRAMS = app mdsim executor

app_LDADD = -lquickfix
app_SOURCES =           \
    src/App.cpp         \
    src/IdHelper.cpp    \
    src/Position.cpp    \
    src/Workspace.cpp   \
    src/Strategy.cpp    \
    src/Vwap.cpp

mdsim_LDADD = -lquickfix
mdsim_SOURCES =           \
    src/MarketDataSimulator.cpp

executor_LDADD = -lquickfix
executor_SOURCES =        \
    src/Executor.cpp      \
    src/ExecutorApp.cpp

