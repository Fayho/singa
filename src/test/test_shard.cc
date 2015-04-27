#include <gtest/gtest.h>
#include <sys/stat.h>

#include "utils/shard.h"

std::string key[]={"firstkey","secondkey","3key", "key4", "key5"};
std::string tuple[]={"firsttuple","2th-tuple","thridtuple", "tuple4", "tuple5"};

using namespace singa;

TEST(ShardTest, CreateShard){
  std::string path="src/test/data/shard_test";
  mkdir(path.c_str(), 0755);
  Shard shard(path, Shard::kCreate, 50);
  shard.Insert(key[0], tuple[0]);
  shard.Insert(key[1], tuple[1]);
  shard.Insert(key[2], tuple[2]);
  shard.Flush();
}

TEST(ShardTest, AppendShard){
  std::string path="src/test/data/shard_test";
  Shard shard(path, Shard::kAppend, 50);
  shard.Insert(key[3], tuple[3]);
  shard.Insert(key[4], tuple[4]);
  shard.Flush();
}
TEST(ShardTest, CountShard){
  std::string path="src/test/data/shard_test";
  Shard shard(path, Shard::kRead, 50);
  int count=shard.Count();
  ASSERT_EQ(5, count);
}

TEST(ShardTest, ReadShard){
  std::string path="src/test/data/shard_test";
  Shard shard(path, Shard::kRead, 50);
  std::string k, t;
  ASSERT_TRUE(shard.Next(&k, &t));
  ASSERT_STREQ(key[0].c_str(), k.c_str());
  ASSERT_STREQ(tuple[0].c_str(), t.c_str());
  ASSERT_TRUE(shard.Next(&k, &t));
  ASSERT_STREQ(key[1].c_str(), k.c_str());
  ASSERT_STREQ(tuple[1].c_str(), t.c_str());
  ASSERT_TRUE(shard.Next(&k, &t));
  ASSERT_TRUE(shard.Next(&k, &t));
  ASSERT_TRUE(shard.Next(&k, &t));
  ASSERT_STREQ(key[4].c_str(), k.c_str());
  ASSERT_STREQ(tuple[4].c_str(), t.c_str());

  ASSERT_FALSE(shard.Next(&k, &t));
  shard.SeekToFirst();
  ASSERT_TRUE(shard.Next(&k, &t));
  ASSERT_STREQ(key[0].c_str(), k.c_str());
  ASSERT_STREQ(tuple[0].c_str(), t.c_str());
}
