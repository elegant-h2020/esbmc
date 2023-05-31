#include <goto-programs/goto_functions.h>

// add an assert(0)
// - at the beginning of each GOTO program
// - at the beginning of each branch body
// - before each END_FUNCTION statement
void add_false_asserts(goto_functionst &goto_functions);

// insert an assert(0) BEFORE the instruciton
void insert_false_assert(
  goto_programt &goto_program,
  goto_programt::targett &it);

// convert every assertion to an assert(0)
void make_asserts_false(goto_functionst &goto_functions);
