#pragma once

#include <vector>
#include <utility>
#include <unordered_map>
#include "FlatCombiner.hpp"
#include "GlobalAllocator.hpp"
#include "ParallelLoop.hpp"
#include "Metrics.hpp"


//in SimpleMetric.hpp define global variable using grappa metric
GRAPPA_DECLARE_METRIC(SimpleMetric<size_t>, hashmap_insert_ops);
GRAPPA_DECLARE_METRIC(SimpleMetric<size_t>, hashmap_insert_msgs);
GRAPPA_DECLARE_METRIC(SimpleMetric<size_t>, hashmap_lookup_ops);
GRAPPA_DECLARE_METRIC(SimpleMetric<size_t>, hashmap_lookup_msgs);

namespace Grappa{

    template<typename K, typename V> class HashMap {

	public:
	    struct Entry {
		K key;
		V value;
		Entry(K ky) : key(ky), value() {}
		Entry(K ky, V val) : key(ky), value(val) {}
	    };

	    struct ResultEntry {
		bool isFound;
		ResultEntry * next;
		V val;
	    };

	    struct Cell {
		std::vector<Entry> entries;

		Cell(): entries() {
		    //just set capacity, not init memory
		    entries.reserve(10);
		}

		std::pair<bool, V> lookup(K ky) {
		    for(auto& e : this->entries) {
			if (e.key == ky) {
			    return std::pair<bool, K>(true, e.value);
			}
		    }
		    return std::pair<bool, K>(false, V());
		}
		//has update function as well
		void insert(const K& ky, const V& val) {
		    bool isFound = false;
		    for(auto& e : this->entries) {
			if(e.key == ky) {
			    e.value = val;
			    isFound = true;
			    break;
			}
		    }
		    if(!isFound) {
			entries.emplace_back(ky, val);
		    }
		}

		void clear(){
		    //Removes all elements from the vector
		    entries.clear();
		}
	    } GRAPPA_BLOCK_ALIGNED;

	    struct Proxy {
		//a type able to represent the size of any object in bytes
		static const size_t LOCAL_HASH_SIZE = 1<<10;
		HashMap* owner;

		//don't need to order, so use unordered_map (c++ 11)
		std::unordered_map<K, V> map;
		std::unordered_map<K, ResultEntry*> lookups;

		void clear() {
		    map.clear();
		    lookups.clear();
		}

		Proxy(HashMap* owner):owner(owner),map(LOCAL_HASH_SIZE),lookups(LOCAL_HASH_SIZE) {
		    clear();
		}

		Proxy* clone_fresh() {
		    //in LocalSharedMemory, allocate owner in the locale shared heap
		    return locale_new<Proxy>(owner);
		}

		bool is_full() {
		    return map.size() >= LOCAL_HASH_SIZE || lookups.size() >= LOCAL_HASH_SIZE;
		}

		void insert(const K& ky_new, const V& val_new) {
		    //returns 1 if an element with ky_new key exists in the container, and zero otherwise
		    if(map.count(ky_new) == 0) {
			map[ky_new] = val_new;
		    }
		}

		void sync() {
		    CompletionEvent ce(map.size()+lookups.size());
		    //Addressing.hpp, return a 2d global pointer to a local pointer on a particular core
		    auto ceg = make_global(&ce);

		    for (auto& e : map) {
			auto& k = e.first;
			auto& v = e.second;
			++hashmap_insert_msgs;
			//TODO:copy from grappa, y
			auto cell = owner->base+owner->computeIndex(k);
			//in SharedMessagePool.hpp, Message with payload, allocated on heap and immediately enqueued to be sent.
			send_heap_message(cell.core(),[cell,ceg,k,v]{
			    cell->insert(k,v);
			    complete(ceg);
			});
		    }

		    for (auto& e : lookups) {
			auto k = e.first;
			++hashmap_lookup_msgs;
			auto re = e.second;
			DVLOG(3) << "lookup " << k << " with re = " << re;
			auto cell = owner->base+owner->computeIndex(k);

			send_heap_message(cell.core(), [cell,k,ceg,re]{
			    Cell * c = cell.localize();
			    bool found = false;
			    V val;
			    for (auto& e : c->entries) if (e.key == k) {
				found = true;
				val = e.value;
				break;
			    }
			    send_heap_message(ceg.core(), [ceg,re,found,val]{
				ResultEntry* r =re;
				//nullptr is null pointer variable in c++11
				while(r != nullptr) {
				    r->isFound = found;
				    r->val = val;
				    r = r->next;
				}
				complete(ceg);
			    });
			});
		    }
		    ce.wait();
		}
	    };

	    GlobalAddress<HashMap> self;
	    GlobalAddress<Cell> base;
	    size_t capacity;

	    FlatCombiner<Proxy> proxy;

	    uint64_t computeIndex(K key) {
		static std::hash<K> hasher;
		return hasher(key) % capacity;
	    }

	    HashMap(GlobalAddress<HashMap> self, GlobalAddress<Cell> base, size_t capacity) : self(self), base(base), capacity(capacity), proxy(locale_new<Proxy>(this)) {
		CHECK_LT(sizeof(self) + sizeof(base) + sizeof(capacity) + sizeof(proxy), 2*block_size);
	    }

	public:
	    //static construction
	    HashMap(){};
	    static GlobalAddress<HashMap> create(size_t total_capacity) {
		//allocate memory globally for Cell
		auto base = global_alloc<Cell>(total_capacity);
		//allocate memory systemtrically for HashMap
		auto self = symmetric_global_alloc<HashMap>();
		//in Collective.hpp, call message (work that cannot block) on all cores, block until ack received from all. Like Grappa::on_all_cores() but does not spawn tasks on each core. Can safely be called concurrently with others.
		call_on_all_cores([self,base,total_capacity]{
		    new (self.localize()) HashMap(self,base,total_capacity);
		});
		forall(base, total_capacity, [](int64_t i, Cell& c){
		    new (&c) Cell();
		});
		return self;
	    }

	    GlobalAddress<Cell> begin() {
		return this->base;
	    }

	    size_t ncells() {
		return this->capacity;
	    }

	    void clear() {
		forall(base, capacity, [](Cell& c){
		    c.clear();
		});
	    }

	    void destroy() {
		auto self = this->self;
		forall(this->base, this->capacity, [](Cell& c){
		    c.~Cell();
		});
		global_free(this->base);
		call_on_all_cores([self]{
		    self->~HashMap();
		});
		global_free(self);
	    }

	    template<typename F> void forall_entries(F visit){
		forall(base, capacity, [visit](int64_t i, Cell& c){
		    if(c.entries == nullptr) return;
		    DVLOG(3) << "c<" << &c << "> entries:" << c.entries << " size: " << c.entries->size();
		    for (Entry& e : *c.entries) {
			visit(e.key, *e.val);
		    }
		});
	    }

	    bool lookup(K key,V* val) {
		if(FLAGS_flat_combining){
		    ResultEntry re{false,nullptr};
		    DVLOG(3) << "lookup[" << key << "] = " << &re;
		    proxy.combine([&re,key](Proxy& p){
			if (p.lookups.count(key) == 0) {
			    p.lookups[key] = nullptr;
			}
			re.next = p.lookups[key];
			p.lookups[key] = &re;
			DVLOG(3) << "p.lookups[" << key << "] = " << &re;
			//TODO: copy, need to figure out
			return FCStatus::BLOCKED;
		    });
		    *val = re.val;
		    return re.isFound;
		}
		else {
		    ++hashmap_lookup_msgs;
		    auto result = delegate::call(base+computeIndex(key), [key](Cell* c){
			return c->lookup(key);
		    });
		    *val = result.second;
		    return result.first;
		}
	    }

	    void insert(K key, V val) {
		++hashmap_insert_ops;
		if(FLAGS_flat_combining) {
		    proxy.combine([key,val](Proxy& p){
			p.map[key] = val;
			return FCStatus::BLOCKED;
		    });
		}
		else {
		    ++hashmap_insert_msgs;
		    delegate::call(base+computeIndex(key), [key,val](Cell* c) {
			c->insert(key, val);
		    });
		}
	    }
    } GRAPPA_BLOCK_ALIGNED;

    template<SyncMode S = SyncMode::Blocking, GlobalCompletionEvent* C = &impl::local_gce, typename K = nullptr_t, typename V = nullptr_t, typename F = nullptr_t> void insert(GlobalAddress<HashMap<K,V>> self, K key, F on_insert) {
	++hashmap_insert_msgs;
	delegate::call<S,C>(self->base+self->computeIndex(key), [=](typename HashMap<K,V>::Cell& c){
	    for(auto& e : c.entries) {
		if(e.key == key) {
		    on_insert(e.val);
		    return;
		}
	    }
	    c.entries.emplace_back(key);
	    on_insert(c.entries.back().val);
	});
    }

    template<GlobalCompletionEvent * GCE = &impl::local_gce, int64_t Threshold = impl::USE_LOOP_THRESHOLD_FLAG, typename T = decltype(nullptr), typename V = decltype(nullptr),typename F = decltype(nullptr)> void forall(GlobalAddress<HashMap<T,V>> self, F visit) {
	forall<GCE,Threshold>(self->begin(), self->ncells(), [visit](typename HashMap<T,V>::Cell& c){
	for (auto& e : c.entries) {
	    visit(e.key, e.val);
	}
    });
  }
}