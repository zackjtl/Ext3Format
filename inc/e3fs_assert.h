#ifndef e3fs_assertH
#define e3fs_assertH

#include "Tester.h"

#define fs_assert_eq(x, y) (assert_equal(x, y, __LINE__, __FILE__, __FUNCTION__))
#define fs_assert_neq(x, y) (assert_non_equal(x, y, __LINE__, __FILE__, __FUNCTION__))
#define fs_assert_true(x) (assert_condition_true(x ? true : false, __LINE__, __FILE__, __FUNCTION__))
#define fs_assert_false(x) (assert_condition_true(x ? false : true, __LINE__, __FILE__, __FUNCTION__))

#endif
