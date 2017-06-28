//---------------------------------------------------------------------------
#ifndef TesterH
#define TesterH
//---------------------------------------------------------------------------
#include <string>
#include "BaseTypes.h"
#include "TypeConv.h"
#include "BaseError.h"

typedef void (*test_func)(void);

typedef struct _test_case
{
public:
	test_func  		fn;
	std::wstring  name;
} test_case;

static void assert_equal(uint32 Lhs, uint32 Rhs, int Line, const string& File, const string& Function)
{
	if (Lhs != Rhs) {
		wstring info = L", Line =" + IntegerToWideString(Line) +
									 L", File =" + ToWideString(File) +
									 L", Func =" + ToWideString(Function) +
									 L"\r\n Lhs = " + IntegerToWideString(Lhs) +
									 L" (" + IntegerToHexWideString(Lhs, 8) + L")" +
									 L"\r\n Rhs = " + IntegerToWideString(Rhs) +
									 L" (" + IntegerToHexWideString(Rhs, 8) + L")";

		throw CError(L"Assert value equal failed. " + info);
	}
}

static void assert_non_equal(uint32 Lhs, uint32 Rhs, int Line, const string& File, const string& Function)
{
	if (Lhs == Rhs) {
		wstring info = L", Line =" + IntegerToWideString(Line) +
									 L", File =" + ToWideString(File) +
									 L", Func =" + ToWideString(Function) +
									 L"\r\n Lhs = " + IntegerToWideString(Lhs) +
									 L" (" + IntegerToHexWideString(Lhs, 8) + L")" +
									 L"\r\n Rhs = " + IntegerToWideString(Rhs) +
									 L" (" + IntegerToHexWideString(Rhs, 8) + L")";

		throw CError(L"Assert value non-equal failed. " + info);
	}
}

static void assert_condition_true(bool Condition, int Line, const string& File, const string& Function)
{
	if (!Condition) {
		wstring info = L", Line =" + IntegerToWideString(Line) +
									 L", File =" + ToWideString(File) +
									 L", Func =" + ToWideString(Function);

		throw CError(L"Assert value non-equal failed. " + info);
	}
}


#define assert_eq(x, y) (assert_equal(x, y, __LINE__, __FILE__, __FUNCTION__))
#define assert_neq(x, y) (assert_non_equal(x, y, __LINE__, __FILE__, __FUNCTION__))
//---------------------------------------------------------------------------
#endif
