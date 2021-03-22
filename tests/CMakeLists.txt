enable_testing()

add_test(NAME Convert_PIE_2to3_exjeep COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/exjeep.pie out.pie)
add_test(NAME Convert_PIE_3to3_exjeep COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/exjeep3.pie out.pie)
