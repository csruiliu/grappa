///////////////////////////////
// tutorial/key_value.cpp
///////////////////////////////
#include "Grappa.hpp"
#include "ParallelLoop.hpp"
#include "GlobalAllocator.hpp"
#include "Delegate.hpp"
#include "GlobalHashMap.hpp"
#include "Metrics.hpp"

using namespace Grappa;

int main(int argc, char *argv[]) {
  Grappa::init(&argc, &argv);
  std::cout << "Key Value Store Test!\n";
  static const size_t FLAGS_global_hash_size = 1<<10;  
  auto hm = GlobalHashMap<long,long>::create(FLAGS_global_hash_size); 
  Grappa::run([]{   
    hm->insert(1,2);
    long val;
    hm->lookup(1, &val);
    std::cout << val << std::endl;
    //auto hm = GlobalHashMap<long,long>::GlobalHashMap();
  });

  Grappa::finalize();
}
