#ifndef PARAM_SERVER_H_
#define PARAM_SERVER_H_


#include <czmq.h>
#include <memory>
#include <vector>
#include <map>
#include <string.h>
#include "proto/model.pb.h"
#include "utils/updater.h"
#include "utils/param.h"
#include "communication/msg.h"
#include "communication/socket.h"
using std::vector;
using std::string;
using std::shared_ptr;

namespace singa{

/**
 * Parameter manager at the server side: repsonding to client's get/udpate
 * request, and periodically syncing with other servers.
 *
 */
typedef std::map<int, shared_ptr<Param>> ParamShard;
class PMServer{
public:
	void Setup(int group_id, int server_id, shared_ptr<ParamShard> shard,
       const UpdaterProto& proto);

	~PMServer();

	/**
	 * Process GET request. Return NULL if successful, in this case the response
	 * is also sent back. Non-NULL return value needed to be re-queued.
	 */
	virtual Msg* HandleGet(Msg** msg);

	/**
	 * Process Update request. Return NULL if successful: the update is applied
   * and msg destroyed.
	 * Else, return the the request to be re-processed.
	 */
	virtual Msg* HandleUpdate(Msg** msg);

	/**
	 * Create new Param object and insert to the map
	 * using paramId as the key. Always return NULL.
	 */
	virtual Msg* HandlePut(Msg **msg);

	/**
	 * Process Sync request from a neighbor server. Return values are the same
   * as HandleGet/Update.
	 */
	virtual Msg* HandleSyncRequest(Msg** msg);

	/**
	 * Simply update, do not send response back.
	 */
	virtual int HandleSyncResponse(Msg** msg);

  virtual bool SyncNow();
 protected:
  int group_id_, server_id_;
  shared_ptr<ParamShard> shard_;
  shared_ptr<Dealer> dealer_;
  shared_ptr<Updater> updater_;
};

} // namespace singa

#endif /* PARAM_SERVER_H_ */
