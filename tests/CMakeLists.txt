enable_testing()

### Simple model
add_test(NAME Convert_PIE2_to_PIE_simple COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/exjeep.pie out_exjeep.pie)
add_test(NAME Convert_PIE2_to_OBJ_simple COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/exjeep.pie out_exjeep.obj)
add_test(NAME Convert_PIE3_to_PIE_simple COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/exjeep3.pie out_exjeep3.pie)
add_test(NAME Convert_PIE3_to_OBJ_simple COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/exjeep3.pie out_exjeep3.obj)
# This will test that PIE3 to PIE is identical
add_test(NAME Compare_PIE3_to_PIE_simple COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/exjeep3.pie out_exjeep3.pie)

### Test that effects flags are preserved
add_test(NAME Convert_PIE3_to_PIE_effect_flags COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/fxmflare3.pie out_fxmflare3.pie)
add_test(NAME Compare_PIE3_to_PIE_effect_flags COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/fxmflare3.pie out_fxmflare3.pie)

### Test that INTERPOLATE 0 flag is preserved
add_test(NAME Convert_PIE3_to_PIE_animation_wo_interpolation
    COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/cybd_run_no_interpolation.pie out_cybd_run_no_interpolation.pie)
add_test(NAME Compare_PIE3_to_PIE_animation_wo_interpolation
    COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/cybd_run_no_interpolation.pie out_cybd_run_no_interpolation.pie)

### PIE 4
add_test(NAME Convert_PIE4_to_PIE4_simple COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/pie4_simple.pie out_pie4_simple.pie)
add_test(NAME Compare_PIE4_to_PIE4_simple COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/pie4_simple.pie out_pie4_simple.pie)

### Test that an explicit TCMASK page is preserved
add_test(NAME Convert_PIE4_to_PIE4_tcmask COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/pie4_tcmask.pie out_pie4_tcmask.pie)
add_test(NAME Compare_PIE4_to_PIE4_tcmask COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/pie4_tcmask.pie out_pie4_tcmask.pie)

### Test that texture overrides for the urban and rockies tilesets are preserved
add_test(NAME Convert_PIE4_to_PIE4_tilesets COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/pie4_tilesets.pie out_pie4_tilesets.pie)
add_test(NAME Compare_PIE4_to_PIE4_tilesets COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/pie4_tilesets.pie out_pie4_tilesets.pie)

### Test that comments survive a round trip
add_test(NAME Convert_PIE4_comments COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/pie4_comments.pie out_pie4_comments.pie)
add_test(NAME Compare_PIE4_comments COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/pie4_comments.pie out_pie4_comments.pie)

### Test that comments survive in every position they can appear in
add_test(NAME Convert_PIE4_comments_full COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/pie4_comments_full.pie out_pie4_comments_full.pie)
add_test(NAME Compare_PIE4_comments_full COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/pie4_comments_full.pie out_pie4_comments_full.pie)

### Test that Windows line endings survive a round trip
add_test(NAME Convert_PIE4_crlf COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/pie4_crlf.pie out_pie4_crlf.pie)
add_test(NAME Compare_PIE4_crlf COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/pie4_crlf.pie out_pie4_crlf.pie)

### Test that a PIE 4 model can be written back out as PIE 3
add_test(NAME Convert_PIE4_to_PIE3 COMMAND wmit --pie-version=3 ${PROJECT_SOURCE_DIR}/tests/pie/pie4_simple.pie out_pie4_as_pie3.pie)
add_test(NAME Compare_PIE4_to_PIE3 COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/exjeep3.pie out_pie4_as_pie3.pie)

### Test that a PIE 3 model can be written back out as PIE 4
add_test(NAME Convert_PIE3_to_PIE4 COMMAND wmit --pie-version=4 ${PROJECT_SOURCE_DIR}/tests/pie/exjeep3.pie out_pie3_as_pie4.pie)
add_test(NAME Compare_PIE3_to_PIE4 COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/pie4_simple.pie out_pie3_as_pie4.pie)

### Test that settings a level overrides for itself are preserved
add_test(NAME Convert_PIE4_level_overrides COMMAND wmit ${PROJECT_SOURCE_DIR}/tests/pie/pie4_level_overrides.pie out_pie4_level_overrides.pie)
add_test(NAME Compare_PIE4_level_overrides COMMAND diff ${PROJECT_SOURCE_DIR}/tests/pie/pie4_level_overrides.pie out_pie4_level_overrides.pie)
