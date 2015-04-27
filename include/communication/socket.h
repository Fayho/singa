#ifndef INCLUDE_COMMUNICATION_SOCKET_H_
#define INCLUDE_COMMUNICATION_SOCKET_H_
#include <map>
#include <vector>
#include <czmq.h>
#include "communication/msg.h"
namespace singa {

class Socket{
  public:
  /**
    * @param args depending on the underlying implementation.
    */
  Socket(void* args){};
  Socket(){};
  /**
    * Send a message to connected socket(s), non-blocking. The message will
    * be deallocated after sending, thus should not be used after calling Send();
    * @param  the message to be sent
    * @return 1 for success queuing the message for sending, 0 for failure
    */
  virtual int Send(Msg* msg)=0;
  /**
    * Receive a message from any connected socket.
    * @return a message pointer if success; nullptr if failure
    */
  virtual Msg* Receive()=0;
  /**
   * @return Identifier of the implementation dependent socket. E.g., zsock_t*
   * for ZeroMQ implementation and rank for MPI implementation.
   */
  virtual void* InternalID() const=0;
};

class Poller{
 public:
  /**
    * Add a socket for polling; Multiple sockets can be polled together by
    * adding them into the same poller.
    */
  virtual void Add(Socket* socket)=0;
  /**
    * Poll for all sockets added into this poller.
    * @param timeout stop after this number of milliseconds
    * @return pointer to the socket if it has one message in the receiving
    * queue; nullptr if no message in any sockets,
    */
  virtual Socket* Poll(int timeout)=0;
};
class Dealer : public Socket{
  public:
  /**
    * Setup the connection with the router.
    *
    * @param endpoint identifier of the router. For intra-process
    * connection, the endpoint follows the format of ZeroMQ, i.e.,
    * starting with "inproc://"; in Singa, since each process has one
    * router, hence we can fix the endpoint to be "inproc://router" for
    * intra-process. For inter-process, the endpoint follows ZeroMQ's
    * format, i.e., IP:port, where IP is the connected process.
    * @return 1 connection sets up successfully; 0 otherwise
    */
  virtual int Connect(string endpoint)=0;
  virtual int Send(Msg* msg)=0;
  virtual Msg* Receive()=0;
  virtual void* InternalID() const=0;
};

class Router : public Socket{
 public:
  /**
   * Constructor.
   *
   * @param bufsize buffer at most this number of messages
   */
  Router(int bufsize=100){
    bufsize_=bufsize;
  }
  /**
  * Setup the connection with dealers.
  *
  * It automatically binds to the endpoint for intra-process communication,
  * i.e., "inproc://router".
  *
  * @param endpoint the identifier for the Dealer socket in other process
  * to connect. It has the format IP:Port, where IP is the host machine.
  * If endpoint is empty, it means that all connections are
  * intra-process connection.
  * @return number of connected dealers.
  */
  virtual int Bind(string endpoint)=0;
  virtual int Send(Msg* msg)=0;
  virtual Msg* Receive()=0;
  virtual void* InternalID() const=0;
 protected:
  int bufsize_;
};

class ZMQPoller: public Poller{
 public:
  ZMQPoller();
  virtual void Add(Socket* socket);
  virtual Socket* Poll(int duration);
 protected:
  zpoller_t *poller_;
  std::map<zsock_t*, Socket*> zsock2Socket_;
};

class ZMQDealer: public Dealer{
 public:
  virtual~ ZMQDealer();
  virtual int Connect(string endpoint);
  virtual int Send(Msg* msg);
  virtual Msg* Receive();
  virtual void* InternalID() const{
    return dealer_;
  }
 protected:
  zsock_t* dealer_;
};

class ZMQRouter: public Router{
 public:
  virtual~ ZMQRouter();
  virtual int Bind(string endpoint);
  /**
   * If the destination socket has not connected yet, buffer this the message.
   */
  virtual int Send(Msg* msg);
  virtual Msg* Receive();
  virtual void* InternalID() const{
    return router_;
  }
 protected:
  zsock_t* router_;
  std::map<int, zframe_t*> id2addr_;
  std::map<int, std::vector<zmsg_t*>> bufmsg_;
  int nBufmsg_;
};

} /* singa */

#endif // INCLUDE_COMMUNICATION_SOCKET_H_
