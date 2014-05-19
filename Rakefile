srcs = FileList["src/*.cpp"]
objs = []
srcs.each do |i|
	j = i.sub /src/, "obj"
	j.sub! /cpp/, "obj"
	objs.push j
end

flag = "-g -Wall -DWPCAP -DHAVE_REMOTE"
inc = "D:/WpdPack/Include"

lib = "D:/WpdPack/Lib"
dev_lib = "D:/Dev-Cpp/lib"

req_lib = "-L#{lib} -lwpcap -L#{dev_lib} -lwsock32"

objs.each_with_index do |obj, i|
	src = srcs[i]
	task obj => src do
		sh "g++ -c #{flag} -I#{inc} -o#{obj} #{src}"
	end
end

file "bin/network.dll" => ["obj/device.obj", "obj/packet.obj"] do |t|
	sh "g++ -shared -o#{t.name} #{t.prerequisites.join(' ')} #{req_lib}"
end

file "bin/socket.dll" => ["obj/tcp.obj"] do |t|
	sh "g++ -shared -o#{t.name} #{t.prerequisites.join(' ')} #{req_lib}"
end

task :library => ["bin/network.dll", "bin/socket.dll"]

file "bin/main.exe" => ["obj/main.obj", "bin/network.dll", "bin/socket.dll"] do |t|
	sh "g++ -o#{t.name} #{t.prerequisites.join(' ')} #{req_lib}"
end

task :excutive => ["bin/main.exe"]

task :objects => objs

task :all => :excutive

task :run => :all do
	sh "bin/main.exe"
end

task :default => :all