/*
 To compile with:
 //leveldb
 g++ -std=c++11 -O3 MyCustomType.cpp -lkvstore_v2 -lboost_serialization -pthread -lkvs_leveldb_v2

 //Redis
 g++ -std=c++11 -O3 MyCustomType.cpp -lkvstore_v2 -lboost_serialization -pthread -lkvs_redis_v2

 //Memcached
 g++ -std=c++11 -O3 MyCustomType.cpp -lkvstore_v2 -lboost_serialization -pthread -lkvs_memcached_v2 -lmemcached
*/

#include <iostream>
#include <kvstore/KVStoreHeader_v2.h>
using namespace std;
using namespace kvstore;


class MyCustomType{
  private:
    double mydouble;
    string mystring;
  public:
    /* Constructors */
    MyCustomType(){
    }

    MyCustomType(double md, string ms){
      mydouble = md;
      mystring = ms;
    }

    /* Setters and Getters */
    void setMyDouble(double val){
      mydouble = val;
    }
    double getMyDouble(){
      return mydouble;
    }
    void setMyString(string val){
      mystring = val;
    }
    string getMyString(){
      return mystring;
    }

    /* Function to be implemented for boost serialization */
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      /* Add all the variables you wanna save, add them
        to archive separated by & operator */
      ar & mydouble & mystring;
    }
};



int main(){
  /* Declare the KVStore object with KeyType and ValType */
  KVStore<double,MyCustomType> ks;

  /* Establish connection to key-value store*/
  bool succ = ks.bind("10.129.26.154:8090","MyTable");
  if(succ){
    cout<<"Connection successful."<<endl;
  } else {
    cout<<"Problem connecting."<<endl;
    return -1;
  }

  /* Storage for return value */
  KVData<MyCustomType> ret;

  /* Put a custom value on key-value store */
  MyCustomType mytype(1.1,"SomeString");
  ret = ks.put(3.14,mytype);
  if( ret.ierr != 0 ){
    cout<<"Error in put :"<<ret.serr<<endl;
  }

  /* Get the value from key-value store */
  MyCustomType retval;
  ret = ks.get(3.14);
  if( ret.ierr != 0 ){
    cout<<"Error in get :"<<ret.serr<<endl;
  } else {
    retval = ret.value;
    cout<<"Got mydouble : "<<retval.getMyDouble()<<endl;
    cout<<"Got mystring : "<<retval.getMyString()<<endl;
  }

  return 0;
}
