package main

import (
  "os"
  "fmt"
  "net"
  "strconv"
  "time"
	"sync"
  // "strings"
  //kvstore "levelmemdb/lmemdb_kvstore"
)

func temp_includes(){
  time.Now();
}

/*
type KVStoreClient struct {
var conn net.Conn
}
*/

var datastore map[string]string
var mtx sync.Mutex

type Client struct {
	tablename string
}


func itoba(i int) []byte {
    slen := i
    bytearr := make([]byte,4)
    idx:=0
    bytearr[idx] = byte((slen >> 24) & 255)
    idx++
    bytearr[idx] = byte((slen >> 16) & 255)
    idx++
    bytearr[idx] = byte((slen >> 8) & 255)
    idx++
    bytearr[idx] = byte((slen) & 255)
    idx++
    return bytearr
}

func batoi(bytearr []byte) int{
  idx:=4
  x:=int(bytearr[idx-4])<<24 + int(bytearr[idx-3])<<16 + int(bytearr[idx-2])<<8 + int(bytearr[idx-1])
  return x
}

func (cl *Client) performTask(strarr []string) ([]string,int){
  used:=1
  switch strarr[0] {
  case "CreateTable":
		cl.tablename = strarr[1]
    used++;
    return []string{"true"},used
  case "Get":
    used++;
		mtx.Lock()
		val,ok := datastore[cl.tablename+strarr[1]]
		mtx.Unlock()
		if ok {
	    return []string{val,"",strconv.Itoa(0)},used
		} else {
			return []string{"","Value doesn't exists.",strconv.Itoa(-1)},used
		}
  case "Put":
    used+=2;
    //fmt.Println("Putting","key",strarr[1],"Val",strarr[2]);
		mtx.Lock()
		datastore[cl.tablename+strarr[1]] = strarr[2]
		mtx.Unlock()
    return []string{"","",strconv.Itoa(0)},used
  case "Del":
		mtx.Lock()
		delete(datastore,cl.tablename+strarr[1])
		mtx.Unlock()
    used++;
    return []string{"","",strconv.Itoa(0)},used
  case "Clear":
    return []string{"false"},used
  }
  return []string{"Incorrect command from client"},used
}

func (cl *Client) performMultipleTasks(strarr []string) []string{
  l := len(strarr)
  ret := make([]string,0)
  idx := 0
  for idx!=l {
    s,n := cl.performTask(strarr[idx:])
    idx+=n
    ret = append(ret,s...)
  }
  //fmt.Println("Return ",ret);
  return ret
}

func (cl *Client) doTask(strarr []string,conn net.Conn){
  //fmt.Println("Strings\n",strarr)
  //fmt.Println("Request arrived",strarr)
  if strarr[0]=="Multiple" {
    ret := cl.performMultipleTasks(strarr[1:])
    writeStrings(ret,conn)
  }else{
    // t1:=time.Now().Nanosecond()
    ret,_ := cl.performTask(strarr)
    // t2:=time.Now().Nanosecond()
    // fmt.Println(strarr[0],"in",t2-t1,"nanosecond")
    writeStrings(ret,conn)

  }
  //time.Sleep(5000)
  //os.Exit(0)
}

func writeStrings(strarr []string,conn net.Conn){
  alen := len(strarr)
  sum := 1
  for i:=0;i<alen;i++ {
    sum += (len(strarr[i])+4)
  }
  //fmt.Println("Sum",sum)
  bytearr := make([]byte,sum)
  idx := 0
  bytearr[0] = byte(alen)
  idx++
  for i:=0;i<alen;i++ {
    slen := len(strarr[i])
    bytearr[idx] = byte((slen >> 24) & 255)
    idx++
    bytearr[idx] = byte((slen >> 16) & 255)
    idx++
    bytearr[idx] = byte((slen >> 8) & 255)
    idx++
    bytearr[idx] = byte((slen) & 255)
    idx++

    copy(bytearr[idx:],[]byte(strarr[i]))
    idx+=slen

  }
  n,err := conn.Write(bytearr) //? err
  if n<sum {
    fmt.Println("Write error",err);
  }
}

func readString(conn net.Conn) string{
  lenbuf := make([]byte,4)
  /*n,err := */conn.Read(lenbuf) //? err
  var slen int
  slen = int(lenbuf[0])<<24 + int(lenbuf[1])<<16 + int(lenbuf[2])<<8 + int(lenbuf[3])
  //fmt.Println("Slen:",slen)
  strbuf := make([]byte,slen)
  dsum:=0;
  for dsum!=slen{
    n,_ := conn.Read(strbuf[dsum:]) //? err
    dsum+=n
  }
  return string(strbuf)
}

func readStrings(conn net.Conn) ([]string, error) {
  no_of_string := make([]byte,1)
  _,err := conn.Read(no_of_string) //? err
  if(err!=nil){
    return nil,err
  }
  strarr := make([]string,no_of_string[0])
  for i:=byte(0);i<no_of_string[0];i++ {
    strarr[i] = readString(conn)
  }
  return strarr,nil
}

func handleClient(conn net.Conn) {
  var cl Client
  //i := 0
  for {
    strarr,err := readStrings(conn)
      //i++
      //fmt.Println("Iter:",i,"Strings",strarr)
    if err!=nil {
      //fmt.Println("returned")
      return
    }
    cl.doTask(strarr,conn)
  }
}

func main() {
  socket := os.Args[1] //Read form command line arugment
  ln, err := net.Listen("tcp",socket)
  if err != nil {
    panic(err)
    return
    } else {
			datastore = make(map[string]string)
      fmt.Println("Server started at ",os.Args[1])
      for{
        conn, err := ln.Accept()
        if err != nil {
          continue
        }
        go handleClient(conn)
      }
    }
  }
