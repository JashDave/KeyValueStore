#include <kvstore/KVStoreHeader_v2.h>
/* #include <YourIncludes> */

namespace kvstore {
  KVImplHelper::KVImplHelper(){
  }

  KVImplHelper::~KVImplHelper(){
  }

  KVImplHelper::KVImplHelper(KVImplHelper& kh){
    /* Copy dataholder if used */
  }

  bool KVImplHelper::bind(string conn, string tablename){
    /* Connection to key value store */
    return false;
  }

  std::shared_ptr<KVData<string>> KVImplHelper::get(string const& key){
    std::shared_ptr<KVData<string>> ret = std::make_shared<KVData<string>>();
    /* Do get and update 'ret' */
    return ret;
  }

  std::shared_ptr<KVData<string>> KVImplHelper::put(string const& key,string const& val){
    std::shared_ptr<KVData<string>> ret = std::make_shared<KVData<string>>();
    /* Do put and update 'ret' */
    return ret;
  }

  std::shared_ptr<KVData<string>> KVImplHelper::del(string const& key){
    std::shared_ptr<KVData<string>> ret = std::make_shared<KVData<string>>();
    /* Do del and update 'ret' */
    return ret;
  }

  void KVImplHelper::async_get(string key, void (*fn)(std::shared_ptr<KVData<string>>,void *),void *data){
    std::shared_ptr<KVData<string>> ret = std::make_shared<KVData<string>>();
    /* Do async get and update 'ret' */
    fn(ret,data);
  }

  void KVImplHelper::async_put(string key,string val, void (*fn)(std::shared_ptr<KVData<string>>,void *),void *data){
    std::shared_ptr<KVData<string>> ret = std::make_shared<KVData<string>>();
    /* Do async put and update 'ret' */
    fn(ret,data);
  }

  void KVImplHelper::async_del(string key, void (*fn)(std::shared_ptr<KVData<string>>,void *),void *data){
    std::shared_ptr<KVData<string>> ret = std::make_shared<KVData<string>>();
    /* Do async del and update 'ret' */
    fn(ret,data);
  }

  bool KVImplHelper::clear(){
    /* Delete all from table */
    return false;
  };

  int KVImplHelper::mget(vector<string>& key, vector<string>& tablename, vector<std::shared_ptr<KVData<string>>>& ret){
    /* Do multiget and send the response via 'ret' vector */
    return 0;
  }

  int KVImplHelper::mput(vector<string>& key, vector<string>& val, vector<string>& tablename, vector<std::shared_ptr<KVData<string>>>& ret){
    /* Do multiput and send the response via 'ret' vector */
    return 0;
  }

  int KVImplHelper::mdel(vector<string>& key, vector<string>& tablename, vector<std::shared_ptr<KVData<string>>>& ret){
    /* Do multidel and send the response via 'ret' vector */
    return 0;
  }
}
