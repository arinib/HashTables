#ifndef __HOPSCOTCH_HASH_
#define __HOPSCOTCH_HASH_

template<typename K, typename V>
class HopscotchHashTable{
private:
  static const size_t MAX_BUCKETS = 1024;
  static const size_t HOP_RANGE = 3;
  static const K emptyKey;
  static const V emptyValue;
  struct hopscotch_bucket{
    K key;
    V value;
    /**
     * Bitmap that stores which buckets in the next HOP_RANGE - 1 buckets
     * contain keys hashed to this bucket index i.
     **/
    unsigned int hop_info;
  };
  //array of hopscotch_buckets
  hopscotch_bucket* bucket_array;
  size_t totalBuckets;
  bool getBucketNumber(size_t& bucketNumber, K key);
  bool isBucketEmpty(size_t bucketNum);
  bool findEmptyBucketAndSwap(size_t &emptyBucket);
public:
  HopscotchHashTable();
  HopscotchHashTable(size_t bucketCount);
  void printHashTable();
  ~HopscotchHashTable();
  bool exists(K key);
  bool add(K key, V value);
  V get(K key);
  void remove(K key);
};

#endif
