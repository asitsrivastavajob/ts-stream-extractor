#include <iostream>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <condition_variable>
#include <queue>
#include <sys/socket.h>
#include <arpa/inet.h>
//#include <sys/resource.h>
#include "TSParser.h"

using namespace std;

#define TS_PACKET_SIZE      188     //Ts packet size
#define IP_HEADER_SIZE      20      // considering ip header of 20 bytes
#define UDP_HEADER_SIZE     8       // udp header of 8 bytes
#define PAT_PID             0       // pat pid
#define SRC_ADDR            "127.0.0.1"
#define PORT_NO             5000
#define MAX_QUEUE_SIZE      20;


mutex m;
condition_variable cv;

int g_consumed_pkt_cnt = 0;
int g_produced_pkt_cnt = 0;
bool is_transmission_continue = true;

//IP packets contain IP header , udp header and TS packet 
//Created a buffer considering these , Ip header varies from 20 to max 60 bytes. 
//i am considering it 20 bytes. 
struct ReceiveBufferArray {
    uint8_t buf[TS_PACKET_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE];
};

std::queue<ReceiveBufferArray> qq;

int gmSocket = -1;

struct sockaddr_in gmClientAddr;
struct sockaddr_in gmServerAddr;

socklen_t gmClientLen = sizeof(gmServerAddr);

//Creating a socket to listen the udp stream at particular PORT_NO
int openSocket(const std::string& IpAddress, int Port)
{
    //struct timeval timeout;
    int optval = 1;

    gmSocket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (gmSocket < 0)
    {
        std::cout << "cannot Open datagram socket!! Ip: " << IpAddress << " - Port " << std::to_string(Port) << std::endl;

        return -1;
    }
    /* Bind our local address so that the client can send to us */
    gmServerAddr.sin_family = AF_INET;
    gmServerAddr.sin_addr.s_addr = INADDR_ANY;
    gmServerAddr.sin_port = htons(Port);

    //timeout.tv_sec = 20;// timeout for 10seconds
    //timeout.tv_usec = 0;
    //setsockopt(gmSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(gmSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    setsockopt(gmSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    std::cout << "Socket has been opened. Ip: " << IpAddress << " - Port " << std::to_string(Port) << std::endl;
    return 0;
}

//function handling TS packet parsing 
//Note : Using TSParser class (TSparser.cpp/.h) , this is not my implementation , i took it from github 
void parse_Buffer(ReceiveBufferArray& _buff, int _consumed_pkt_cnt)
{
    TSPacket tPkt;
    TSParser parser;
    int ret = tPkt.Parse(_buff.buf + 28 , TS_PKT_LEN);
    parser.PrintPacketInfo(tPkt, 0, g_consumed_pkt_cnt);
}

//Implemented producer consumer functionality in consumer_thraed() and producer_thread()
//Producer_thread puts data to global queue of type ReceiveBufferArray and 
//consumer_thread() consumes qq queue
void consumer_thread()
{
    ReceiveBufferArray temp_rbuf;
    while (is_transmission_continue )
    {
        g_consumed_pkt_cnt ++;

        {
            unique_lock<mutex> ul(m); 
            cv.wait(ul,[](){return qq.size() > 0;});
            temp_rbuf = qq.front();
            qq.pop();
            ul.unlock();
            cv.notify_one();
        }

        parse_Buffer(temp_rbuf,g_consumed_pkt_cnt);
        //std::cout << " Produced Packet no : "<< g_produced_pkt_cnt << ", consumed Packet no : "<< g_consumed_pkt_cnt <<endl;
        //usleep(1);

        //if(g_consumed_pkt_cnt == g_produced_pkt_cnt)
            //is_transmission_continue = false;
    }

    cout<<"CONSUMED ALL PACKETS"<<endl;

}

//producer_thread() function receives data in _rbuf from recvfrom function and i pass 
//the data in global variable qq , which is a queue of type  ReceiveBufferArray
void producer_thread()
{
    openSocket(SRC_ADDR, PORT_NO);
    ReceiveBufferArray _rbuf;
    int packet_size = 0;
    while (is_transmission_continue)
    {
        packet_size = recvfrom(gmSocket, _rbuf.buf, TS_PACKET_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE, 0, NULL, NULL);
        g_produced_pkt_cnt++;

        {
            unique_lock<mutex> ul(m);
            cv.wait(ul,[](){return qq.size() < MAX_QUEUE_SIZE;});
            qq.push(_rbuf);
            ul.unlock();
            cv.notify_one();
        }

        //std::cout << "Packet Size : " << packet_size << ", Produced Packet no : "<< g_produced_pkt_cnt << ", consumed Packet no : "<< g_consumed_pkt_cnt <<endl;
    }
    std::cout << "PRODUCER DONE" << endl;
}

int main()
{
    //setpriority(PRIO_PROCESS, 0, -20);
    thread cons(consumer_thread);
    thread prod(producer_thread);
    prod.join();
    cons.join();
    return 0;
}
