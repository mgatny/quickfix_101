Given(/^my app is running and it is subscribed to "(.*?)"$/) do |symbol|
    steps %{Then I should receive a message on FIX of type "Logon" with agent "marketdata"}
    steps %{Then I should receive a message on FIX of type "Logon" with agent "executor"}
end

