#include "Grappa.hpp"
#include "ParallelLoop.hpp"
#include "GlobalAllocator.hpp"
#include "Delegate.hpp"
#include "GlobalHashMap.hpp"
//#include "hashmap.hpp"
//#include "DHT.hpp"
#include "Metrics.hpp"
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <iostream>
using namespace Grappa;

long count=0;
int pp = 0;

int main(int argc, char *argv[]){
    if(argc != 3) {
	std::cerr << "Usage: " << argv[0]
	     << " [total_cores_num] [read/write]"
	     << std::endl;
	std::cerr << "   eg: " << argv[0]
	     << " 16 0.5" << std::endl;
	return 0;
    }
    Grappa::init(&argc, &argv);

    Grappa::run([&argv]{
	int cores = std::stoi(argv[1]);
	pp = std::stoi(argv[2]);
	std::cout<<"Key Value Store Test!" << std::endl;
	static const size_t global_hash_size = 1 << 15;
	auto hm = GlobalHashMap<long, uint64_t>::create(global_hash_size);

	forall(0, cores, [hm](int64_t i){
	    std::ifstream kv;
	    std::string str_kv = "/data/zhanghao/zipf/" + std::to_string(i) + ".dat";
	    std::cout << "Read file " << str_kv << " from Core " << Grappa::mycore() << " of " << Grappa::cores() << " (Node " <<     Grappa::mylocale() << ")"<< "\n";
	    kv.open(str_kv.c_str(),std::ios::in);
	    std::string key;
	    while(true) {
		std::getline(kv,key);
		if (kv.eof()) {
		    break;
		}
		++count;
		long key_l = std::stol(key);
		std::stringstream stream;
		std::string str = "1234567890";
		uint64_t ins;
		stream << str;
		stream >> ins;
		hm->insert(key_l,ins);
	    }
	    kv.close();
	});

	double start = walltime();
	forall(0, cores, [hm](int64_t i){
	    std::ifstream kv;
	    std::string str_kv = "/data/zhanghao/zipf/" + std::to_string(i) + ".dat";
	    kv.open(str_kv.c_str(),std::ios::in);
	    std::string key;
	    while(true) {
		std::getline(kv,key);
		if (kv.eof()) {
		    break;
		}
		long key_l = std::stol(key);
		std::stringstream stream;
		std::string str = "1234567890";
		srand((unsigned)time(NULL));
		if((rand() / double(RAND_MAX)) >= pp) {
		    uint64_t ins;
		    stream << str;
		    stream >> ins;
		    hm->insert(key_l,ins);
		}
		else{
		    uint64_t val;
		    std::string val_s;
		    hm->lookup(key_l,&val);
		    stream << val;
		    stream >> val_s;
		}
	    }
	    kv.close();
	});


	/*
	forall(0, 1000000000000000, [hm](int64_t i){
	    std::stringstream stream;
	    std::string str = "1234567890";
	    uint64_t ins;
	    //uint64_t val;
	    //hm->lookup(i, &val);
	    stream << str;
	    stream >> ins;
	    hm->insert(i,ins);
	    //streamk << val;
	    //streamk >> strk;
	    //std::cout << hostName << std::endl;
	    //hm->insert(i, val+1);
	});
	*/

	double end = walltime();
	double duration = end - start;
	/*
	forall(0, 200, [hm](int64_t i){
	    long val;
	    hm->lookup(i, &val);
	    std::cout << i << ":" << val << std::endl;
	    std::cout << "[" << i << ": core " << i.core() << "] ";
	});
	*/
	std::cout << "Ops_Per_Core: " << count << std::endl;
	std::cout << "Duration: " << duration << std::endl;
	std::cout << "Throught: " << count * cores / duration / 1000 << std::endl;
	//std::cout << "Throught:" << duration << std::endl;
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
    exit(0);
}