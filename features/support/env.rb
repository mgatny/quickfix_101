require 'bundler/setup'
require 'agent_fix/cucumber'
require 'fix_spec/cucumber'
require 'rspec'
require 'anticipate'
require 'spoon'

World(Anticipate)

FIXSpec::data_dictionary = FIXSpec::DataDictionary.new "etc/FIX42.xml"
AgentFIX.include_session_level = true
AgentFIX.start
@app_pid = Spoon.spawnp("./app", "etc/app.cfg", "AAPL") #symbol)

at_exit do
    AgentFIX.stop
    Process.kill("TERM", @app_pid) unless @app_pid.nil?
    Process.waitall
end

