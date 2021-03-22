enable_testing()

add_test(NAME Convert_PIE2_to_PIE3_exjeep COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/exjeep.pie out_exjeep.pie)
add_test(NAME Convert_PIE2_to_OBJ_exjeep COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/exjeep.pie out_exjeep.obj)
add_test(NAME Convert_PIE3_to_PIE3_exjeep COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/exjeep3.pie out_exjeep3.pie)
add_test(NAME Convert_PIE3_to_OBJ_exjeep COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/exjeep3.pie out_exjeep3.obj)
add_test(NAME Compare_PIE3_to_PIE3_exjeep COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/exjeep3.pie out_exjeep3.pie)
