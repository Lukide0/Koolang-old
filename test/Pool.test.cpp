#include "air/Pool.h"
#include "test.h"

using namespace air;

TEST_CASE("Pool - Caching")
{
    Pool pool;

    const auto keyA = PoolKey::CreateIntVal(0, 0);
    const auto keyB = PoolKey::CreateIntVal(1, 0);
    const auto keyC = PoolKey::CreateIntVal(2, 0);

    const auto keyAIndex = pool.Put(keyA);
    const auto keyBIndex = pool.Put(keyB);
    const auto keyCIndex = pool.Put(keyC);

    CHECK_EQ(keyAIndex, pool.Get(keyA));
    CHECK_EQ(keyBIndex, pool.Get(keyB));
    CHECK_EQ(keyCIndex, pool.Get(keyC));

    // keyAIndex != keyBIndex != keyCIndex
    CHECK_NE(keyAIndex, keyBIndex);
    CHECK_NE(keyBIndex, keyCIndex);
}
