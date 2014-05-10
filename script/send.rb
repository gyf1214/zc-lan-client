require 'socket'

ul = UDPSocket.new
ul.bind '0.0.0.0', 6112
ul.send 'Hello!', 0, ARGV[0], 6112