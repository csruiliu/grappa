#include "Delegate.hpp"
#include "GlobalAllocator.hpp"
#include "Grappa.hpp"
#include "Metrics.hpp"
#include "ParallelLoop.hpp"
#include "hashmap.hpp"
#include <sstream>
#include <string>
#include <unistd.h>
using namespace Grappa;

int main(int argc, char *argv[]) {
  Grappa::init(&argc, &argv);
  // std::cout<<"Key Value Store Test!";
  // static const size_t global_hash_size = 1<<10;
  // auto hm = GlobalHashMap<long,long>::create(global_hash_size);
  /*
  forall(10, 20, [hm](int64_t i){
      hm->insert(i, i+20);
    });

    forall(10, 20, [hm](int64_t i){
      long val;
      hm->lookup(i, &val);
      std::cout << val;
    });
  */
  // std::cout << val << std::endl;

  Grappa::run([] {
    std::cout << "Key Value Store Test!";
    static const size_t global_hash_size = 1 << 20;
    auto hm = HashMap<long, uint64_t>::create(global_hash_size);
    std::stringstream stream;
    std::string str = "1234567890";
    uint64_t ins;
    stream << str;
    stream >> ins;
    // char hostName[100];
    // int result = gethostname(hostName, 100);
    // unsigned long = "sdsdsd"
    // double t = walltime();
    // std::string ins = "ssssssssss";
    // char ins[100] = "ssssssss";
    // char * ins = "ssssssssss";
    for (int i = 0; i < 10000000; i++) {
      hm->insert(i, ins);
    }
    // long val;
    /*
    for (int i = 0; i < 20; ++i) {
      hm->lookup(i, &val);
      /std::cout << val << std::endl;
    }
    */
    double t = walltime();
    /*
    on_all_cores([hm]{
     for (int i = 0; i < 1000000; i++) {
       //char hostName[100];
       //int result = gethostname(hostName,100);
       //std::cout << hostName << std::endl;
       uint64_t  val;
       hm->lookup(i, &val);
       std::stringstream streamk;
       std::string strk;
       streamk << val;
       streamk >> strk;
       //hm->insert(i,val+"dddddddd");
       //std::cout << i << ":" << strk << std::endl;
     }
    });
    */
    // t = walltime() - t;

    forall(0, 10000000, [hm](int64_t i) {
      char hostName[100];
      int result = gethostname(hostName, 100);
      uint64_t val;
      hm->lookup(i, &val);
      std::stringstream streamk;
      std::string strk;
      streamk << val;
      streamk >> strk;
      // std::cout << hostName << std::endl;
      // hm->insert(i, val+1);
    });
    t = walltime() - t;
    /*
    forall(0, 200, [hm](int64_t i){
      long val;
      hm->lookup(i, &val);
      std::cout << i << ":" << val << std::endl;
      std::cout << "[" << i << ": core " << i.core() << "] ";
    });
    */

    std::cout << "Time:" << t << std::endl;

    /*
    on_all_cores([hm]{
     for (int i=0; i<20; i++) {
       char hostName[100];
       int result = gethostname(hostName,100);
       std::cout << hostName << std::endl;
       hm->insert(i,i+1);
     }
    });
    */
  });
  Grappa::finalize();
}
