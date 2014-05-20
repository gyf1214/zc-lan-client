require 'socket'

ul = UDPSocket.new
ul.send 'Hello!', 0, ARGV[0], 6112