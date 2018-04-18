#include <iostream>
#include "hopscotch_hash.hpp"

using namespace std;

#define DEBUG

#ifdef DEBUG
#define debug(x) cout << x
#else
#define debug(x)
#endif

template<typename K, typename V>
const K HopscotchHashTable<K,V>::emptyKey{};

template<typename K, typename V>
const V HopscotchHashTable<K,V>::emptyValue{};

template<typename K, typename V>
HopscotchHashTable<K,V>::HopscotchHashTable(){
  cout << "Empty Key is " << emptyKey << endl;
  cout << "Empty Value is " << emptyValue << endl;
  bucket_array = new hopscotch_bucket[MAX_BUCKETS];
  totalBuckets = MAX_BUCKETS;
  for(size_t i = 0; i < totalBuckets; i++){
    bucket_array[i] = {emptyKey, emptyValue,0}; 
  }
}

template<typename K, typename V>
HopscotchHashTable<K,V>::HopscotchHashTable(size_t bucketCount){
  //Round bucketCount to the nearest multiple of 2
  //Refer to Hacker's Delight
  
  cout << "Empty Key is " << emptyKey << endl;
  cout << "Empty Value is " << emptyValue << endl;
  bucketCount--;
  bucketCount |= bucketCount >> 1;
  bucketCount |= bucketCount >> 2;
  bucketCount |= bucketCount >> 4;
  bucketCount |= bucketCount >> 6;
  bucketCount |= bucketCount >> 8;
  bucketCount |= bucketCount >> 16;
  bucketCount++;
  cout << "Creating HashMap with " << bucketCount << " buckets!" << endl;
  bucket_array = new hopscotch_bucket[bucketCount];
  totalBuckets = bucketCount;
  for(size_t i = 0; i < totalBuckets; i++){
    bucket_array[i] = {emptyKey, emptyValue,0}; 
  }
}

template<typename K, typename V>
HopscotchHashTable<K,V>::~HopscotchHashTable(){
  cout << "Destroying HashMap." << endl;
  delete[] bucket_array;
}

template<typename K, typename V>
void HopscotchHashTable<K,V>::printHashTable(){
  for(int i = 0; i < totalBuckets; i++){
    if(!isBucketEmpty(i)){
      cout << "Bucket " << i << ": " << bucket_array[i].key <<  " -> "
	   << bucket_array[i].value
	   << "( " << bitset<HOP_RANGE>(bucket_array[i].hop_info) << ")" << endl;
    }
  }
}
template<typename K, typename V>
bool HopscotchHashTable<K,V>::isBucketEmpty(size_t bucketNum){
  hopscotch_bucket b = bucket_array[bucketNum];
  if(b.key == emptyKey &&
     b.value == emptyValue){
    return true;
  }
  return false;
}


template<typename K, typename V>
bool HopscotchHashTable<K,V>::
findEmptyBucketAndSwap(size_t &emptyBucket){
  bool foundEmptyBucket = false;
  for(size_t distance = HOP_RANGE - 1; distance > 0; distance--){
    size_t startBucket = emptyBucket - distance;
    size_t startHopInfo = bucket_array[startBucket].hop_info;
    size_t swapDistance = ffs(startHopInfo);
    if(swapDistance == 0){
      continue;
    }

    /**
     * While setting the hop_info, we set the LSB to 1 when the hop distance
     * is zero. The function ffs returns the index of the least significant 
     * bit that is set and the index starts from 1 and not 0. So decrement 
     * swapDistance by 1;
     **/
    swapDistance--;
    size_t swapBucket = startBucket + swapDistance;
    bucket_array[emptyBucket].key = bucket_array[swapBucket].key;
    bucket_array[emptyBucket].value = bucket_array[swapBucket].value;
    bucket_array[swapBucket].key = emptyKey;
    bucket_array[swapBucket].value = emptyValue;

    bucket_array[startBucket].hop_info |= (1 << distance);
    bucket_array[startBucket].hop_info &= ~(1 << swapDistance );
    emptyBucket = swapBucket;
    foundEmptyBucket = true;
    break;
  }
  return foundEmptyBucket;
}


template<typename K, typename V>
bool HopscotchHashTable<K,V>::exists(K key){
  size_t hashValue = hash<K>{}(key);
  hashValue = hashValue % totalBuckets;

  size_t hopInfo = bucket_array[hashValue].hop_info;
  for(int distance = 0; distance < HOP_RANGE; distance++){
    if (1 << distance & hopInfo) {
      hopscotch_bucket bucket = bucket_array[hashValue + distance];
      if(bucket.key == key){
	return true;
      }
    }
  }
  return false;
}


template<typename K, typename V>
bool HopscotchHashTable<K,V>::add(K key, V value) {
  size_t hashValue = hash<K>{}(key);
  hashValue = hashValue % totalBuckets;
  size_t emptyBucket  = 0;
  bool foundEmptyBucket = false;
  
  cout << "HashValue is " << hashValue << endl;

  if(exists(key)){
    return true;
  }
  
  /** Find a bucket within the hop range and copy the key and value 
   * into the first empty bucket within the hop range
   * and update the hop_info in the original hashBucket
   **/
  size_t distanceFromHashBucket = 0;
  size_t startBucket = hashValue;
  size_t endBucket = min((size_t)hashValue + HOP_RANGE - 1, totalBuckets - 1);
  int i = startBucket;
  for(;i <= endBucket; i++, distanceFromHashBucket++){
    if(isBucketEmpty(i)) {
      foundEmptyBucket = true;
      bucket_array[i].key = key;
      bucket_array[i].value = value;
      bucket_array[hashValue].hop_info |= ( 1 << distanceFromHashBucket );
      return foundEmptyBucket;
    }
  }
  /**
   * If no empty bucket is found in the HOP_RANGE, find the first empty bucket
   * and try to move the entries around to make space.
   **/
  for(; i < totalBuckets; i++){
    if(isBucketEmpty(i)){
      emptyBucket = i;
      foundEmptyBucket = true;
      break;
    }
  }

  while(foundEmptyBucket && emptyBucket > endBucket){
    foundEmptyBucket = findEmptyBucketAndSwap(emptyBucket);
  }

  if(!foundEmptyBucket) {
    return false;
  }

  bucket_array[emptyBucket].key = key;
  bucket_array[emptyBucket].value = value;
  bucket_array[hashValue].hop_info |= (1 << (emptyBucket - hashValue));
  return true;
}

template<typename K, typename V>
bool HopscotchHashTable<K,V>::getBucketNumber(size_t& bucketNum, K key){
  size_t hashValue = hash<K>{}(key);
  size_t bucket = hashValue % totalBuckets;
  size_t hopInfo = bucket_array[bucket].hop_info;
  for(size_t distance = 0; distance < HOP_RANGE; distance++){
    if(1 << distance & hopInfo) {
      bucketNum = bucket + distance;
      if(bucket_array[bucketNum].key == key){
	return true;
      }
    }
  }
  return false;
}

template<typename K, typename V>
V HopscotchHashTable<K,V>::get(K key){
  size_t bucketNum = 0;
  if(getBucketNumber(bucketNum, key)){
    return bucket_array[bucketNum].value;
  }
  return emptyValue;
}


template<typename K, typename V>
void HopscotchHashTable<K,V>::remove(K key){
  size_t bucketNum = 0;
  if(getBucketNumber(bucketNum, key)){
    size_t hashBucket = hash<K>{}(key) % totalBuckets;
    size_t distance = bucketNum - hashBucket;

    bucket_array[hashBucket].hop_info &= ~(1 << distance);
    bucket_array[bucketNum].value = emptyValue;
    bucket_array[bucketNum].key = emptyKey;
  }
  return;
}
		   

int main(){
  HopscotchHashTable<string, string> stringHashTable(10);
  stringHashTable.printHashTable();
  stringHashTable.add("ABC","DEF");
  stringHashTable.printHashTable();
  stringHashTable.add("XYZ","ABC");
  stringHashTable.printHashTable();
  stringHashTable.add("Hello", "World");
  stringHashTable.printHashTable();
  stringHashTable.add("HELLO","WORLD");
  stringHashTable.printHashTable();
  stringHashTable.add("HEllo","World");
  stringHashTable.printHashTable();
  stringHashTable.add("abc", "def");
  stringHashTable.printHashTable();

  cout << "Removing the key \"abc\" " << endl;
  stringHashTable.remove("abc");
  stringHashTable.printHashTable();

  stringHashTable.add("Hello", "World");
  stringHashTable.printHashTable();
  HopscotchHashTable<int, int> intHashTable;
  intHashTable.add(1,100);
  intHashTable.printHashTable();
  return 0;
}

