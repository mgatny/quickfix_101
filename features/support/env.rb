require 'rubygems'
require 'bundler/setup'
require 'minitest/unit'

require 'cuke_mem'
require 'agent_fix'
require 'agent_fix/cucumber'
require 'agent_fix/cucumber/report'
require 'fix_spec/cucumber'
require 'fix_spec/builder'
require 'rspec'
require 'anticipate'
require 'spoon'

FIXSpec.data_dictionary = quickfix.DataDictionary.new("etc/FIX42.xml")

@app_pid = Spoon.spawnp("./app", "etc/app.cfg", "AAPL") #symbol)

at_exit do
    Process.kill("TERM", @app_pid) unless @app_pid.nil?
    Process.waitall
end

