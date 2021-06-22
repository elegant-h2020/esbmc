/*******************************************************************\

Module: Loop IDs

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_CBMC_LOOP_IDS_H
#define CPROVER_CBMC_LOOP_IDS_H

#include <goto-programs/goto_functions.h>

void show_loop_numbers(
  const goto_functionst &goto_functions,
  const messaget &msg);

void show_loop_numbers(const goto_programt &goto_program, const messaget &msg);

#endif
