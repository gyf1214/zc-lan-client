require 'socket'

ul = UDPSocket.new
ul.bind '0.0.0.0', 6112
data = ul.recvfrom 6112
puts data[0]