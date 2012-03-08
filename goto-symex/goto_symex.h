/*******************************************************************\

Module: Symbolic Execution

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_GOTO_SYMEX_GOTO_SYMEX_H
#define CPROVER_GOTO_SYMEX_GOTO_SYMEX_H

#include <map>
#include <std_types.h>
#include <i2string.h>
#include <hash_cont.h>
#include <options.h>

#include <goto-programs/goto_functions.h>

#include "goto_symex_state.h"
#include "symex_target.h"

class reachability_treet; // Forward dec
class execution_statet; // Forward dec

/**
 *  Primay symbolic execution class.
 *  Contains very little state data, instead implements a large number of
 *  methods that actually perform the symbolic execution and keep data
 *  elsewhere. Interprets actual GOTO functions. Fixes up assignments and
 *  formats operations into something suitable for being fed to the symex
 *  target object. Also maintains renaming hierarchies, call stacks, pointer
 *  tracking goo.
 */

class goto_symext
{
public:
  /**
   *  Default constructor. Performs base initialization, storing namespace
   *  and the symex target we'll be assigning at.
   *  @param _ns Namespace we'll be working in.
   *  @param _new_context Context we'll be working in.
   *  @param _target Symex target that actions will be recorded into.
   *  @param opts Options we'll be running with.
   */
  goto_symext(const namespacet &_ns, contextt &_new_context,
              const goto_functionst &goto_functions,
              symex_targett *_target, const optionst &opts);
  goto_symext(const goto_symext &sym);
  goto_symext& operator=(const goto_symext &sym);

  // Types

public:
  friend class symex_dereference_statet;
  friend class bmct;

  typedef goto_symex_statet statet;

  /**
   *  Class recording the outcome of symbolic execution.
   *  Contains the things that are of interest to the BMC class: The object
   *  containing the symex equation (or otherwise, the symex target), as well
   *  as the list of claims that have been recorded (and how many are already
   *  satisfied).
   */
  class symex_resultt {
  public:
    symex_resultt(symex_targett *t, unsigned int claims, unsigned int remain) :
      target(t), total_claims(claims), remaining_claims(remain) { };

    symex_targett *target;
    unsigned int total_claims;
    unsigned int remaining_claims;
  };

  // Macros
  //
  /**
   *  Return identifier for goto guards.
   *  These guards are symbolic names for the truth of a guard on a GOTO jump.
   *  Assertions and other activity during the course of symbolic execution
   *  encode these execution guard in them.
   *  @return Name of the guard
   */
  irep_idt guard_identifier(void)
  {
    return irep_idt(id2string(guard_identifier_s) + "!" + i2string(cur_state->top().level1._thread_id));
  };

  // Methods

  /**
   *  Create a symex result for this run.
   */
  symex_resultt *get_symex_result(void);

  /**
   *  Symbolically execute one instruction.
   *  Essentially a despatcher. Performs some renaming and dereferencing, then
   *  hands off an expression to be claimed, or assigned, or possibly for some
   *  control flow beating to occur (goto, func call, return). Threading
   *  specific operations are handled by execution_statet, which overrides
   *  this.
   *  @param art Reachability tree we're working with.
   */
  virtual void symex_step(reachability_treet & art);

protected:
  /**
   *  Perform simplification on an expression.
   *  Essentially is just a call to simplify, but is guarded by the
   *  --no-simplify option being turned off.
   *  @param expr Expression to simplify, in place.
   */
  virtual void do_simplify(exprt &expr);

  /**
   *  Dereference an expression.
   *  Finds dereference expressions within expr, takes the set of things that
   *  it might point at, according to value set tracking, and builds an
   *  if-then-else list of concrete references that it might point at.
   *  @param expr Expression to eliminate dereferences from.
   *  @param write Whether or not we're writing into this object.
   */
  void dereference(exprt &expr, const bool write);

  /**
   *  Recursive implementation of dereference method.
   *  @param expr Expression to eliminate dereferences from.
   *  @param guard Some guard (defunct?).
   *  @param dereference Dereferencet object to operate with.
   *  @param write Whether or not we're writing to this object.
   */
  void dereference_rec(
    exprt &expr,
    guardt &guard,
    class dereferencet &dereference,
    const bool write);

  // symex

  /**
   *  Perform GOTO jump using current instruction.
   *  Handle a GOTO jump between locations. This isn't just the factor of there
   *  being jumps where the guards are nondeterministic, it's that we have to
   *  handle editing the unwind bound when these things occur, and set up state
   *  merges in the future to handle each path thats taken. A precise
   *  description of how this is implemented... can go somewhere else.
   *  @param old_guard Renamed guard on this jump occuring.
   */
  virtual void symex_goto(const exprt &old_guard);

  /**
   *  Perform interpretation of RETURN instruction.
   */
  void symex_return(void);

  /**
   *  Interpret an OTHER instruction.
   *  These can take many forms; memory management functions are OTHERs for
   *  example (ideally they should be intrinsics...), but also printf and
   *  variable declarations are handled here.
   */
  void symex_other(void);

  /**
   *  Perform an assertion.
   *  Encodes an assertion that the expression claimed is always true. This
   *  adds the requirement that the current state guard is true as well.
   *  @param expr Expression that must always be true.
   *  @param msg Textual message explaining assertion.
   */
  virtual void claim(
    const exprt &expr,
    const std::string &msg);

  /**
   *  Perform an assumption.
   *  Adds to target an assumption that must always be true.
   *  @param assumption Assumption that must always be true.
   */
  virtual void assume(const exprt &assumption);

  // gotos
  /**
   *  Merge converging states into current state.
   *  Jumps forwards are handled by recording a merge of the current state in
   *  the future; then when we hit that future state, a phi function is
   *  performed that joins the states converging at this point, according to
   *  the truth of their guards.
   */
  void merge_gotos(void);

  /**
   *  Merge pointer tracking value sets in a phi function.
   *  See merge_gotos - when we're merging states together due to previous
   *  jumps, this function implements the merging of pointer tracking data.
   *  @param goto_state Previously executed goto state to be merged in.
   *  @param dest Thread state for previous jump to be merged into.
   */
  void merge_value_sets(const statet::goto_statet &goto_state);

  /**
   *  Join together a previous jump state into thread state.
   *  This combines together two thread states by using if-then-elses to decide
   *  the new value of a variable, according to the truth of the guards of the
   *  states being joined. 
   *  @param goto_state The previous jumps state to be merged into the current
   */
  void phi_function(const statet::goto_statet &goto_state);

  /**
   *  Test whether unwinding bound has been exceeded.
   *  This looks up a look number, checks the limit on unwindings against the
   *  given number of unwinds already performed, and returns whether that limit
   *  has been exceeded.
   *  @param source Program location to check for loops against.
   *  @param unwind Number of unwinds that have already occured.
   *  @return True if we've unwound past the unwinding limit.
   */
  bool get_unwind(
    const symex_targett::sourcet &source,
    unsigned unwind);

  /**
   *  Encode unwinding assertions and assumption.
   *  If unwinding assertions are on, assert that the unwinding bound is not
   *  exceeded. If partial loops are off, assume that the unwinding bound was
   *  not exceeded. Otherwise, just continue execution.
   *  @param guard Current state guard.
   */
  void loop_bound_exceeded(const exprt &guard);

  // function calls

  /**
   *  Pop a stack frame.
   *  This frees/removes the top stack frame, and removes any relevant local
   *  variables from the l2 renaming, and value set tracking.
   */
  void pop_frame(void);

  /**
   *  Create assignment for return statement.
   *  Generate an assignment to the return variable from this return statement.
   *  @param assign Assignment expression. Output.
   *  @param code The return statement we're interpreting.
   *  @return True if a return assignment was generated.
   */
  bool make_return_assignment(code_assignt &assign, const code_returnt &code);

  /** 
   *  Perform function call.
   *  Handles all kinds of function call instructions, symbols or function
   *  pointers.
   *  @param call Function call we're working on.
   */
  void symex_function_call(const code_function_callt &call);

  /**
   *  End a functions interpretation.
   *  This routine pops a stack frame, and returns control to the caller;
   *  except in the case of function pointer interpretation, where we instead
   *  switch to interpreting the next pointed to function.
   */
  void symex_end_of_function(void);

  /**
   *  Handle a call to a named function.
   *  @param call Function call we're performing.
   */
  void symex_function_call_symbol(const code_function_callt &call);

  /**
   *  Handle an indirect function call, to a pointer.
   *  Finds all potential targets, and sets up calls to them with the
   *  appropriate guards and targets. They are then put in a list, the first
   *  one run, then at the end of each of these function calls we switch to
   *  the next in the list. Finally, when the insn after the func ptr call is
   *  run, all func ptr call states are merged in.
   *  @param call Function call to interpret.
   */
  virtual void symex_function_call_deref(const code_function_callt &call);

  /**
   *  Handle function call to fixed function
   *  Like symex_function_call_code, but minus an assertion and location
   *  recording.
   *  @param code Function code to actually call
   */
  virtual void symex_function_call_code(const code_function_callt &call);

  /**
   *  Discover whether recursion bound has been exceeded.
   *  @see get_unwind
   *  @param identifier Name of function to consider recursion of.
   *  @param unwind Number of times its been unwound already.
   *  @return True if unwind recursion has been exceeded.
   */
  bool get_unwind_recursion(
    const irep_idt &identifier,
    unsigned unwind);

  /**
   *  Join up function arguments.
   *  Assigns the value of arguments to a function to the actual argument
   *  variables of the function being called.
   *  @param function_type type containing argument types of func call.
   *  @param arguments The arguments to assign to function arg variables.
   */
  void argument_assignments(
    const code_typet &function_type,
    const exprt::operandst &arguments);

  /**
   *  Fill goto_symex_statet::framet with renamed local variable names.
   *  These names are all the names of local variables, renamed to level 1, so
   *  that we have a list of all variables that are in fact local to this
   *  particular function call.
   *  @param frame_counter The function frame invocation number.
   *  @param goto_function The function we're working upon.
   */
  void locality(unsigned frame_counter,
    const goto_functionst::goto_functiont &goto_function);

  /**
   *  Setup next function in a chain of func ptr calls.
   *  @see symex_function_call_deref
   *  @param first Whether this is the first func ptr invocation.
   *  @return True if a function pointer invocation was set up.
   */
  bool run_next_function_ptr_target(bool first);

  /**
   *  Run an intrinsic, something prefixed with __ESBMC.
   *  This looks through a set of intrinsic functions that are implemented in
   *  ESBMC, and calls the appropriate one. Examples include starting a thread,
   *  ending a thread, switching to another thread.
   *  @param call Function call being performed.
   *  @param art Reachability tree we're operating on.
   *  @param symname Name of intrinsic we're calling.
   */
  void run_intrinsic(code_function_callt &call, reachability_treet &art,
                     const std::string symname);

  /** Perform yield; forces a context switch point. */
  void intrinsic_yield(reachability_treet &arg);
  /** Perform switch_to; switches control to explicit thread ID. */
  void intrinsic_switch_to(code_function_callt &call, reachability_treet &art);
  /** Yield, always switching away from this thread */
  void intrinsic_switch_from(reachability_treet &arg);
  /** Perform get_thread_id; return the current thread identifier. */
  void intrinsic_get_thread_id(code_function_callt &call,
                                reachability_treet &art);
  /** Perform set_thread_state; store thread startup information. */
  void intrinsic_set_thread_data(code_function_callt &call,
                                reachability_treet &art);
  /** Perform get_thread_data; get thread startup information. */
  void intrinsic_get_thread_data(code_function_callt &call,
                                reachability_treet &art);
  /** Perform spawn_thread; Generates a new thread at a named function. */
  void intrinsic_spawn_thread(code_function_callt &call, reachability_treet &art);
  /** Perform terminate_thread; Record thread as terminated. */
  void intrinsic_terminate_thread(reachability_treet &art);
  /** Perform get_thead_state... defunct. */
  void intrinsic_get_thread_state(code_function_callt &call, reachability_treet &art);

  // dynamic stuff
  /**
   *  Replace ireps regarding dynamic allocations with code.
   *  Things like "invalid-object" and suchlike are replaced here with
   *  references to array members, or more elaborate expressions, representing
   *  how that information is actually stored in the resulting SMT. In the past
   *  this has been done in the solver backend, but that seems slightly
   *  the wrong place.
   *  @param expr Expression we're replacing the contents of.
   */
  void replace_dynamic_allocation(exprt &expr);

  /**
   *  Decide if symbol is valid or not.
   *  i.e., whether it's live or not. Not very well understood.
   *  @return True if symbol is valid.
   */
  bool is_valid_object(const symbolt &symbol);

  /**
   *  Make symbolic assignment.
   *  Renames things; records assignment in symex target, and all the relevant
   *  renaming and value set tracking objects. The primary task of this routine
   *  is to rewrite assignments to arrays, structs, and byte_selects into the
   *  equivalent uses of WITH, or byte_update, and so forth. The end result is
   *  a single new value to be bound to a new symbol.
   *  @param code Code to assign; with lhs and rhs.
   */
  virtual void symex_assign(const codet &code);

  /** Recursively perform symex assign. @see symex_assign */
  void symex_assign_rec(const exprt &lhs, exprt &rhs, guardt &guard);

  /**
   *  Perform assignment to a symbol.
   *  Renames further, performs goto_symex_statet::assignment and symex target
   *  assignments.
   *  @param lhs Symbol to assign to
   *  @param rhs Value to assign to symbol
   *  @param guard Guard; intent unknown
   */
  void symex_assign_symbol(const exprt &lhs, exprt &rhs, guardt &guard);

  /**
   *  Perform assignment to a typecast irep.
   *  This just ends up moving the typecast from the lhs to the rhs.
   *  @param lhs Typecast to assign to
   *  @param rhs Value to assign to lhs
   *  @param guard Guard; intent unknown
   */
  void symex_assign_typecast(const exprt &lhs, exprt &rhs, guardt &guard);

  /**
   *  Perform assignment to an array.
   *  lhs transformed to the container of the array, or the symbol for its
   *  destination. rhs converted to a WITH statement, updating the contents of
   *  the original array with the value of the original rhs.
   *  @param lhs Array to assign to
   *  @param rhs Value to assign to symbol
   *  @param guard Guard; intent unknown
   */
  void symex_assign_array(const exprt &lhs, exprt &rhs, guardt &guard);

  /**
   *  Perform assignment to a struct.
   *  Exactly like with arrays, but with structs and members.
   *  @see symex_assign_array
   *  @param lhs Struct to assign to
   *  @param rhs Value to assign to lhs
   *  @param guard Guard; intent unknown
   */
  void symex_assign_member(const exprt &lhs, exprt &rhs, guardt &guard);

  /**
   *  Perform assignment to an "if".
   *  This ends up being two assignments, one to one branch of the if, the
   *  other to the other. The appropriate guard is executed in either case.
   *  Possibly defunct; I'm not aware of C supporting nondeterministic
   *  left hand side expressions.
   *  @param lhs "If" to assign to
   *  @param rhs Value to assign to lhs
   *  @param guard Guard; intent unknown
   */
  void symex_assign_if(const exprt &lhs, exprt &rhs, guardt &guard);

  /**
   *  Perform assignment to a byte extract.
   *  Results in a byte update of the relevant part of the lhs with the
   *  right hand side at the appropriate position. Currently a problem , as
   *  assignments of something that's bigger than a byte fails.
   *  @param lhs Byte extract to assign to
   *  @param rhs Value to assign to lhs
   *  @param guard Guard; intent unknown
   */
  void symex_assign_byte_extract(const exprt &lhs, exprt &rhs, guardt &guard);

  /** Symbolic implementation of malloc. */
  void symex_malloc(const exprt &lhs, const side_effect_exprt &code);
  /** Symbolic implementation of c++'s delete. */
  void symex_cpp_delete(const codet &code);
  /** Symbolic implementation of c++'s new. */
  void symex_cpp_new(const exprt &lhs, const side_effect_exprt &code);
  /** Symbolic implementation of printf */
  void symex_printf(const exprt &lhs, const exprt &code);

  /**
   *  Replace nondet func calls with nondeterminism.
   *  Creates a new nondeterministic symbol, with a globally unique counter
   *  encoded into its name. Is left as a free variable.
   *  @param expr Expr to search for nondet symbols.
   */
  void replace_nondet(exprt &expr);

  /**
   *  Fetch reference to global dynamic object counter.
   *  @return Reference to global dynamic object counter.
   */
  virtual unsigned int &get_dynamic_counter(void) = 0;
  /**
   *  Fetch reference to global nondet object counter.
   *  @return Reference to global nondet object counter.
   */
  virtual unsigned int &get_nondet_counter(void) = 0;

  // Members

  /**
   *  Symbol prefix for guards.
   *  These guards are the symbolic names for the truth of whether a particular
   *  branch has been taken during symbolic execution.
   *  @see guard_identifier
   */
  irep_idt guard_identifier_s;

  /** Number of assertions executed. */
  unsigned total_claims;
  /** Number of assertions remaining to be discharged. */
  unsigned remaining_claims;
  /** Reachability tree we're working with. */
  reachability_treet *art1;
  /** Names of functions that we've complained about missing bodies of. */
  hash_set_cont<irep_idt, irep_id_hash> body_warnings;
  /** Unwind bounds, loop number -> max unwinds. */
  std::map<unsigned, long> unwind_set;
  /** Global maximum number of unwinds. */
  unsigned int max_unwind;
  /** Whether constant propagation is to be enabled. */
  bool constant_propagation;
  /** Namespace we're working in. */
  const namespacet &ns;
  /** Options we're working with */
  const optionst &options;
  /** Context we're working with */
  contextt &new_context;
  /** GOTO functions that we're operating over. */
  const goto_functionst &goto_functions;
  /** Target listening to the execution trace */
  symex_targett *target;
  /** Target thread we're currently operating upon */
  goto_symex_statet *cur_state;
};

#endif
