AgentFIX.session_defaults.merge! BeginString: "FIX.4.2", SocketConnectHost: "localhost"

AgentFIX.define_acceptor :marketdata do |a|
    a.default = { SenderCompID: "MARKETDATA", SocketAcceptPort: 5002 }
    a.session = { TargetCompID: "CLIENT1"    }
end

AgentFIX.define_acceptor :executor do |a|
    a.default = { SenderCompID: "EXECUTOR", SocketAcceptPort: 5001 }
    a.session = { TargetCompID: "CLIENT1"  }
end
