#include "../GroundTruthRegistration/src/GroundTruthRegistration.h"
#include <iostream>
#include "gtest/gtest.h"

TEST(GroundTruthRegistration, BasicAssert){
	ASSERT_EQ(1,1);
}

int main(int argc, char *argv[]){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
