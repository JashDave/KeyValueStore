/*
 To compile with:
 //leveldb
 g++ -std=c++11 -O3 AsynchronousFunction.cpp -lkvstore_v2 -lboost_serialization -pthread -lkvs_leveldb_v2

 //Redis
 g++ -std=c++11 -O3 AsynchronousFunction.cpp -lkvstore_v2 -lboost_serialization -pthread -lkvs_redis_v2

 //Memcached
 g++ -std=c++11 -O3 AsynchronousFunction.cpp -lkvstore_v2 -lboost_serialization -pthread -lkvs_memcached_v2 -lmemcached
*/

#include <iostream>
#include <mutex>
#include <kvstore/KVStoreHeader_v2.h>
using namespace std;
using namespace kvstore;

#define OPR_COUNT 10
mutex waitmtx; // Used to wait for all operations to get completed.

void myCallbackFunction(int count, KVData<string> ret){
 if(ret.ierr != 0){
   cout<<"Call at "<<count<<" index failed."<<endl;
   cout<<"Error:"<<ret.serr<<endl;
 }
 //Executed for last call.
 if(count == OPR_COUNT){
   cout<<"Done with all the calls."<<endl;
   //Releases the lock after all operations are done.
   waitmtx.unlock();
 }
}


int main(){
  KVStore<int, string> ks;

  /* Establish connection to key-value store */
  ks.bind("10.129.26.154:8090","MyTable");

  waitmtx.lock();
  for(int i = 1; i <= OPR_COUNT; i++){
    ks.async_put(i,"SomeValue",myCallbackFunction,i);
  }
  //Wait for all operations to get completed.
  waitmtx.lock();

  return 0;
}
