compile:
	g++ main.cpp consensus.cpp messages/consensus.pb.cc messages/validator.pb.cc messages/pbft-message.pb.cc -o peer -lzmqpp -lpthread -lzmq -lprotobuf
	g++ proxy.cpp -o proxy -lzmqpp -lzmq
	cd ~/Hyperledger/Sawtooth/sawtooth-pbft; cargo build
run:
	#./proxy &
	#sleep 1
	./peer tcp://*:5050 tcp://localhost:7777 tcp://localhost:7778 A1 
oneTest:
	#make compile
	./peer tcp://*:505$(foo) tcp://localhost:7777 tcp://localhost:7778 A$(foo) &
	sleep 1 
	cd ~/Hyperledger/Sawtooth/sawtooth-pbft; cargo run -- -v --connect tcp://localhost:505$(foo) &
fourTest:
	make compile
	./proxy &
	sleep 1
	./peer tcp://*:5051 tcp://localhost:7777 tcp://localhost:7778 A1 &
	echo peer 1 up

	./peer tcp://*:5052 tcp://localhost:7777 tcp://localhost:7778 A2 &
	echo peer 2 up

	./peer tcp://*:5053 tcp://localhost:7777 tcp://localhost:7778 A3 &
	echo peer 3 up

	./peer tcp://*:5054 tcp://localhost:7777 tcp://localhost:7778 A4 &
	echo peer 4 up

	
	cd ~/Hyperledger/Sawtooth/sawtooth-pbft; cargo run -- --connect  &

	cd ~/Hyperledger/Sawtooth/sawtooth-pbft; cargo run -- --connect &
	
	cd ~/Hyperledger/Sawtooth/sawtooth-pbft; cargo run -- --connect &

	cd ~/Hyperledger/Sawtooth/sawtooth-pbft; cargo run -- --connect &
	
	sleep 7
	make kill

kill:
	-kill -9 `pgrep peer`
	-kill -9 `pgrep pbft`
proto:
	protoc -I=./protos --cpp_out=./messages ./protos/consensus.proto
	protoc -I=./protos --cpp_out=./messages ./protos/validator.proto
	protoc -I=./protos --cpp_out=./messages ./protos/pbft-message.proto