#include <kvstore/KVStoreHeader_v2.h>
#include <hiredis-vip/hircluster.h>
#include <mutex>
#include <atomic>
#include <queue>
#include <chrono>
#include <iostream>
#include <thread>
using namespace std;
namespace kvstore {

  #define c_kvsclient ((KVStoreClient*)dataholder)

  struct async_data{
    void (*fn)(std::shared_ptr<KVData<string>>,void *);
    void *data;
    int type;
  };

  class KVStoreClient{
  public:
    redisClusterContext* rc;
    string tablename;
    string conn;
    std::mutex mtx;
    std::queue<struct async_data> q;
    bool keeprunning = true;
    thread td;
    std::atomic<long long> count;
    KVStoreClient(){
      count=0;
    }
    ~KVStoreClient(){
      keeprunning = false;
      redisClusterFree(rc);
    }
    void eventLoop(){
      redisReply *reply;
      int rep;
      std::chrono::milliseconds waittime(100);

      while(keeprunning){
        while(count==0){std::this_thread::sleep_for(waittime);}
        mtx.lock();
        rep = redisClusterGetReply(rc, &reply);
        mtx.unlock();

// #define REDIS_REPLY_STRING 1
// #define REDIS_REPLY_ARRAY 2
// #define REDIS_REPLY_INTEGER 3
// #define REDIS_REPLY_NIL 4
// #define REDIS_REPLY_STATUS 5
// #define REDIS_REPLY_ERROR 6
				if(rep == REDIS_OK){
          if(reply == NULL){
            cerr<<"Reply Null"<<endl;
          } else {
            std::shared_ptr<KVData<string>> ret = std::make_shared<KVData<string>>();
            struct async_data ad = q.front(); q.pop();  //? lock required?
            count--;
            if(reply->type == REDIS_REPLY_STRING){
                ret->ierr = 0;
                ret->value = string(reply->str);
            } else if (reply->type == REDIS_REPLY_STATUS){
                ret->ierr = 0;
              //str == OK
            } else if (reply->type == REDIS_REPLY_NIL){
                ret->ierr = -1;
                ret->serr = "Value doesn't exists.";
              //value doesnt exists
            } else if (reply->type == REDIS_REPLY_INTEGER){
                ret->ierr = reply->integer;
                if(ret->ierr != 0){
                  ret->serr = "Value doesn't exists.";
                }
              //del reply in reply->integer ; 0==OK -1==Doesnt exists
            } else {
              cerr<<"Reply type:"<<reply->type<<endl;
            }
            ad.fn(ret,ad.data);
            freeReplyObject(reply);
          }
				} else {
          cerr<<"Error in return file:"<<endl;//<<__FILENAME__<<" line:"<<__LINE__<<endl;
					// redisClusterReset(cc);
				}
      }
    }
    void startEventLoop(){
      td = thread([&]{eventLoop();});
    }
  };

  KVImplHelper::KVImplHelper(){
    dataholder = (void*) new KVStoreClient();
  }

  KVImplHelper::KVImplHelper(KVImplHelper& kh){
    dataholder = (void*) new KVStoreClient();
    bool succ = bind(((KVStoreClient*)kh.dataholder)->conn,((KVStoreClient*)kh.dataholder)->tablename);
    if(!succ){
      std::cerr << "Error copying KVImplHelper object" << std::endl;
    }
  }

  KVImplHelper::~KVImplHelper(){
    delete(c_kvsclient);
  }

  bool KVImplHelper::bind(string conn, string tablename){
    c_kvsclient->tablename = tablename;
    c_kvsclient->conn = conn;

    bool retry = true;
    int attempts = 0;
    int MAX_TRIES = 10;
    while(retry){
      c_kvsclient->rc = redisClusterConnect(conn.c_str(), HIRCLUSTER_FLAG_NULL);
      if(c_kvsclient->rc == NULL || c_kvsclient->rc->err)
      {
        attempts++;
        if(attempts == MAX_TRIES){
          //printf("connect error : %s\n", rc == NULL ? "NULL" : rc->errstr);
          return false;
        }
      } else {
        retry = false; /* break; */
      }
    }
    c_kvsclient->startEventLoop();
    return true;
  }

  std::shared_ptr<KVData<string>> KVImplHelper::get(string const& key){
    std::shared_ptr<KVData<string>> ret = std::make_shared<KVData<string>>();
    redisReply *reply = (redisReply *)redisClusterCommand(c_kvsclient->rc, "get %s", (c_kvsclient->tablename+key).c_str());
    if(reply == NULL)
    {
      ret->ierr = -1;
      ret->serr = "Unknown error.";
      // printf("reply is null[%s]\n", c_kvsclient->rc->errstr);
      //redisClusterFree(c_kvsclient->rc); //??
    } else if(reply->str == NULL){
      ret->ierr = -1;
      ret->serr = "Value doesn't exists.";
      freeReplyObject(reply);
      ret->ierr = 0;
      ret->value = string(reply->str);
      freeReplyObject(reply);
    }
    return ret;
  }

  std::shared_ptr<KVData<string>> KVImplHelper::put(string const& key,string const& val){
    std::shared_ptr<KVData<string>> ret = std::make_shared<KVData<string>>();
    redisReply *reply = (redisReply *)redisClusterCommand(c_kvsclient->rc, "set %s %s", (c_kvsclient->tablename+key).c_str(), val.c_str());
    if(reply == NULL)
    {
      ret->ierr = -1;
      ret->serr = "Unknown error.";
      // printf("reply is null[%s]\n", c_kvsclient->rc->errstr);
      //redisClusterFree(c_kvsclient->rc); //??
    } else {
      ret->ierr = 0;
      freeReplyObject(reply);
    }
    return ret;
  }

  std::shared_ptr<KVData<string>> KVImplHelper::del(string const& key){
    std::shared_ptr<KVData<string>> ret = std::make_shared<KVData<string>>();
    redisReply *reply = (redisReply *)redisClusterCommand(c_kvsclient->rc, "del %s", (c_kvsclient->tablename+key).c_str());
    if(reply == NULL)
    {
      ret->ierr = -1;
      ret->serr = "Unknown error.";
      // printf("reply is null[%s]\n", c_kvsclient->rc->errstr);
      //redisClusterFree(c_kvsclient->rc); //??
    } else {
      ret->ierr = reply->integer; // 0;
      if(ret->ierr != 0){
        ret->serr = "Value doesn't exists.";
      }
      freeReplyObject(reply);
    }
    return ret;
  }

  bool KVImplHelper::clear(){
    /* Not yet implemented */
    return false;
  };

  int KVImplHelper::mget(vector<string>& key, vector<string>& tablename, vector<std::shared_ptr<KVData<string>>>& ret){
    int sz = key.size();
    for(int i=0;i<sz;i++){
      c_kvsclient->tablename = tablename[i];
      ret.push_back(get(key[i]));
    }
    return 0;
  }

  int KVImplHelper::mput(vector<string>& key, vector<string>& val, vector<string>& tablename, vector<std::shared_ptr<KVData<string>>>& ret){
    int sz = key.size();
    for(int i=0;i<sz;i++){
      c_kvsclient->tablename = tablename[i];
      ret.push_back(put(key[i],val[i]));
    }
    return 0;
  }

  int KVImplHelper::mdel(vector<string>& key, vector<string>& tablename, vector<std::shared_ptr<KVData<string>>>& ret){
    int sz = key.size();
    for(int i=0;i<sz;i++){
      c_kvsclient->tablename = tablename[i];
      ret.push_back(del(key[i]));
    }
    return 0;
  }

  void KVImplHelper::async_get(string key, void (*fn)(std::shared_ptr<KVData<string>>,void *),void *data){
      c_kvsclient->mtx.lock();
      int ret = redisClusterAppendCommand(c_kvsclient->rc, "get %d",key.c_str()) ;
      c_kvsclient->mtx.unlock();
			if(ret!= REDIS_ERR){
				c_kvsclient->count++;
        struct async_data ad{fn,data,1};
        c_kvsclient->q.push(ad);
			}	else {
				cerr<<"\n\n\nget Append error\n\n\n"<<endl;
			}
  }

  void KVImplHelper::async_put(string key,string val, void (*fn)(std::shared_ptr<KVData<string>>,void *),void *data){
    c_kvsclient->mtx.lock();
    int ret = redisClusterAppendCommand(c_kvsclient->rc, "put %d %d",key.c_str(),val.c_str()) ;
    c_kvsclient->mtx.unlock();
		if(ret!= REDIS_ERR){
			c_kvsclient->count++;
      struct async_data ad{fn,data,1};
      c_kvsclient->q.push(ad);
		}	else {
			cerr<<"\n\n\nput Append error\n\n\n"<<endl;
		}
  }

  void KVImplHelper::async_del(string key, void (*fn)(std::shared_ptr<KVData<string>>,void *),void *data){
    c_kvsclient->mtx.lock();
    int ret = redisClusterAppendCommand(c_kvsclient->rc, "del %d",key.c_str()) ;
    c_kvsclient->mtx.unlock();
		if(ret!= REDIS_ERR){
      c_kvsclient->count++;
      struct async_data ad{fn,data,1};
      c_kvsclient->q.push(ad);
		}	else {
			cerr<<"\n\n\nget Append error\n\n\n"<<endl;
		}
  }
}
