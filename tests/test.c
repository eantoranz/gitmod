/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <CUnit/Basic.h>
#include "gitmod.h"
#include "suites.h"

int main()
{
	CU_pSuite pSuite1 = NULL, pSuite2 = NULL, pSuiteKim = NULL, pSuiteKim2 = NULL;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* add a suite to the registry */
	pSuite1 = suite1_setup();
	pSuite2 = suite2_setup();
	pSuiteKim = suitekim_setup();
	pSuiteKim2 = suitekim2_setup();
	if (!(pSuite1 && pSuite2 && pSuiteKim && pSuiteKim2)) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
