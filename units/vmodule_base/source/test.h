#pragma once

#ifdef GTEST_CASE
#include <gtest/gtest.h>
#else
// disable GTEST stuff
#define FRIEND_TEST(test_case_name, test_name) \
	friend class test_case_name##_##test_name##_Test
#endif
