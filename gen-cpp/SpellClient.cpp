#include "SpellService.h"

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
//#include <boost/thread.hpp>
//#include <boost/date_time.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <ctype.h>
#include <openssl/md5.h>

#define NUMSHARDS 2

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace SpellServer;

int main(int argc, char **argv){
    //if not enough input, return
    if(argc < 3) return -1;
    //reading server list from different serverList
    std::vector<std::string> serverListS0;
    std::vector<std::string> serverListS1;
    std::ifstream inputStream;

    inputStream.open("serverListS0");
    std::string server;
    std::string lastline = "";
    while(true){
        getline(inputStream, server);
        if(server != lastline){
	  serverListS0.push_back(server);
        }
        else{
          break;
        }
    }
    inputStream.close();
    inputStream.open("serverListS1");
    while(true){
      getline(inputStream, server);
      std::cout << server << std::endl;
      if(server != lastline){
	serverListS1.push_back(server);
      }
      else{
	break;
      }
    }
      printf("server list loaded\n");
    
    //get the input words into 2 request
    SpellRequest requestS0, requestS1;
    SpellResponse responseS0, responseS1;
    
    //hash the input words, put them into different request vector
    
    for(int i = 2; i < argc; i++){
      unsigned char shard, hash[16];
      std::string input = std::string(argv[i]);
      MD5((unsigned char*)argv[i], strlen(argv[i]), hash);
      shard = hash[15] % NUMSHARDS;

      if(shard){
	//std::cout<< "S1 " << input <<std::endl;
	requestS1.to_check.push_back(input);
      }
      else{
	//std::cout<< "S0 " << input <<std::endl;
	requestS0.to_check.push_back(input);
      }
    }
    
    //time out configure
    int timeOut = atoi(argv[2]);
    
    //randomize the server list
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(serverListS0.begin(), serverListS0.end(), g);
    std::shuffle(serverListS1.begin(), serverListS1.end(), g);
    
    //get server IP from serverList, the port is in the list
    for (std::vector<std::string>::iterator i = serverListS0.begin(); i != serverListS0.end(); i++) {
        std::istringstream buf(*i);
        std::istream_iterator<std::string> beg(buf), end;
        std::vector<std::string> tokens(beg, end);
        boost::shared_ptr<TSocket> socket(new TSocket(tokens[0], atoi(tokens[1].c_str())));
        
#pragma mark - timeOut setting
        socket->setConnTimeout(100);
        socket->setSendTimeout(100);
        socket->setRecvTimeout(timeOut * 1000);
        boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

        SpellServiceClient client(protocol);
        //handle TCP connection error with 100ms time out
        try {
            //std::cout << "connecting " + *i /*<< e.what()*/ << std::endl;
            transport -> open();
        } catch (TTransportException &e) {
            printf("open error...changing server...\n");
            continue;
        }
        
        //RPC call to the server
        printf("requesting... \n");
        try {
            client.spellcheck(responseS0, requestS0);
            transport -> close();
            break;
        } catch (TTransportException &e) {
            //std::cout << e.what() << std::endl;
            printf("server busy... changing server...\n");
            if (transport->isOpen()) {
                transport->close();
            }
            continue;
        }
    }
    
    for (std::vector<std::string>::iterator i = serverListS1.begin(); i != serverListS1.end(); i++) {
        std::istringstream buf(*i);
        std::istream_iterator<std::string> beg(buf), end;
        std::vector<std::string> tokens(beg, end);
        boost::shared_ptr<TSocket> socket(new TSocket(tokens[0], atoi(tokens[1].c_str())));
        
#pragma mark - timeOut setting
        socket->setConnTimeout(100);
        socket->setSendTimeout(100);
        socket->setRecvTimeout(timeOut * 1000);
        boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        
        SpellServiceClient client(protocol);
        //handle TCP connection error with 100ms time out
        try {
            //std::cout << "connecting " + *i /*<< e.what()*/ << std::endl;
            transport -> open();
        } catch (TTransportException &e) {
            printf("open error...changing server...\n");
            continue;
        }
        
        //RPC call to the server
        printf("requesting... \n");
        try {
            client.spellcheck(responseS1, requestS1);
            transport -> close();
            break;
        } catch (TTransportException &e) {
            //std::cout << e.what() << std::endl;
            printf("server busy... changing server...\n");
            if (transport->isOpen()) {
                transport->close();
            }
            continue;
        }
    }
    
    //Output the response (misspell words only)
    printf("responsing...\n");
    printf("misspell words: \n");
    for(int i = 0; i < responseS1.is_correct.size(); i++){
          if(!responseS1.is_correct[i]){
	     std::cout << requestS1.to_check.at(i) << " ";
	  } 
    }
        
	for(int i = 0; i< responseS0.is_correct.size(); i++){
	  if(!responseS0.is_correct[i]){
	    std::cout << requestS0.to_check.at(i) << " ";
	  }
    }
	std::cout << std::endl;
    
    return 0;
}

