require 'socket'

tl = TCPServer.open '0.0.0.0', 6112

client = tl.accept

loop do
	str = client.gets
	puts str
	client.puts str
end

client.close