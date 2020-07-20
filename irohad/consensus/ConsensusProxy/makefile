compile:
	g++ main.cpp consensus.cpp messages/consensus.pb.cc messages/validator.pb.cc messages/pbft-message.pb.cc -o peer -lzmqpp -lpthread -lzmq -lprotobuf
	g++ proxy.cpp -o proxy -lzmqpp -lzmq
run:
	#./proxy &
	#sleep 1
	./peer  tcp://*:5050 test.com tcp://localhost:7777 A1 
proto:
	protoc -I=./protos --cpp_out=./messages ./protos/consensus.proto
	protoc -I=./protos --cpp_out=./messages ./protos/validator.proto
	protoc -I=./protos --cpp_out=./messages ./protos/pbft-message.proto