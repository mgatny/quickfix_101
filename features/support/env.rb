require 'bundler/setup'
require 'agent_fix/cucumber'
require 'fix_spec/cucumber'
require 'rspec'
require 'anticipate'
require 'spoon'

#---- Start the app in the background, and stash the pid so we can kill it later:

$app_pid = nil
def start_app symbol
    $app_pid = Spoon.spawnp("./app", "etc/app.cfg", symbol)
end

#---- At exit, kill the app if it is still running:

at_exit do
    AgentFIX.stop
    unless $app_pid.nil?
        puts "Killing app (pid=#{$app_pid})"
        Process.kill("TERM", $app_pid)
        Process.waitall
    end
end

#---- Set up fix_spec and agent_fix:

FIXSpec::data_dictionary = FIXSpec::DataDictionary.new "etc/FIX42.xml"
AgentFIX.include_session_level = true
AgentFIX.start

World(Anticipate)

