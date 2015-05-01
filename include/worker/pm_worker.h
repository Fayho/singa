#ifndef PARAM_CLIENT_H_
#define PARAM_CLIENT_H_

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <atomic>
#include "utils/param.h"
#include "communication/msg.h"

using std::string;
using std::vector;
using std::shared_ptr;
using std::map;

namespace singa {


enum RequestReturnType {
	NON_LOCAL, LOCAL_SUCCESS, LOCAL_FAIL
};

#define POPULATE "put"
#define WAIT "wait"
class ParamCounter{
  public:
  ParamCounter(shared_ptr<Param> p):
    nUpdate(0), nGet(0), nPut(0), nCollect(0), nLocal(0), nTotal(0),
    owner_procs(-1), param(p){}
  std::atomic<int> nUpdate, nGet, nPut, nCollect;
  int nLocal, nTotal;
  int owner_procs;
  shared_ptr<Param> param;
};

typedef map<int, shared_ptr<ParamCounter>> SharedParamShard;

/**
 * Parameter manager at the worker side.
 *
 * It supports get/update requests from the worker.
 * Each worker thread has a PMClient object, these objects share the same ParamShard.
 */
class PMWorker{
public:

	void Setup(int group_id, int worker_id, shared_ptr<SharedParamShard> shard);

  void set_id(int group_id, int worker_id){
    group_id_=group_id;
    worker_id_=worker_id;
  }

  /**
   * @return global procs id where the parameter is maintained.
   */
  virtual int Sharding(int param_id);

	/**
	 * Get the parameter object with key paramId. Return getReturnType:
	 * 1. If non-local, send the message to remote server. Collect later
	 * 2. If local and HandleGet() return true -> LOCAL_SUCCESS, can use *param object
	 * 3. Local but block at HandleGet() -> LOCAL_FAIL -> to call Get() again
	 */
	virtual Msg* Get(shared_ptr<Param> param, int step);
  virtual Msg* Get(Msg** msg);

	/**
	 * Update operation, similar to Get.
	 */
	virtual Msg* Update(shared_ptr<Param> param, int step);
  virtual Msg* Update(Msg** msg);

	/**
	 * Collect a Param object returned from remote server. Return FALSE
	 * if no object is ready.
	 */
	virtual Msg* Collect(shared_ptr<Param> param, Msg**);
  virtual Msg* Collect(Msg** msg);


	/**
	 * Send put request to remote server.
	 */
	virtual Msg* Put(shared_ptr<Param> param, int step);
  virtual Msg* Put(Msg** msg);

 protected:
  int group_id_, worker_id_;
  shared_ptr<SharedParamShard> shard_;
};

/**
 * Testing worker functionality.The main thread reads the config file and set up the socket.
 *
 * Create the shared ParamShard, then starts worker thread which basically carries out the work.
 * Each thread creates a PMClient object.
 *
 * The main thread then enter the loops to forward messages.
 *
 * Requests from the worker thread is prepend the paramId, which is stripped by the main thread
 * before forwarding to the correct server.
 *
 * The 1st thread in Client 0 populates the servers with data (PUT request). Wait
 * for a while before starting the client thread (which does get/update
 * continuously).
class SingaClient {
public:
	SingaClient(int worker_id, Topology &topology, vector<string> &hosts);
	void StartClient();

	int id() {
		return id_;
	}
	ParamShard *param_shard() {
		return param_shard_;
	}
	char *backend_endpoint() {
		return backend_endpoint_;
	}

private:
	int id_, local_id_, group_id_;
	char backend_endpoint_[256];
	vector<char*> neighbors_;
	ParamShard *param_shard_;

	int param_to_server_id(int paramId);//< mapping paramId to server ID
};

//Zthread function for the worker thread, in the global namespace.
//Basically a loop of: compute, get, update, compute, etc.
void ClientThread(void *args, zctx_t *ctx, void *pipe);

vector<Param*> gen_random_params();
void test_get(PMClient *client);
void test_update(PMClient *client, vector<Param*> params);
void test_collect(PMClient *client);
 */

} // namespace singa
#endif /* PARAM_SERVER_H_ */
