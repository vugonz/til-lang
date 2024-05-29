#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated

//---------------------------------------------------------------------------

void til::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void til::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}

void til::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  // EMPTY
}
void til::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  // EMPTY
}
void til::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if (in_function()) {
    _pf.INT(node->value()); // push an integer
  } else {
    _pf.SINT(node->value());
  }
}

void til::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  if (in_function()) {
    _pf.DOUBLE(node->value());
  } else {
    _pf.SDOUBLE(node->value());
  }
}

void til::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl = ++_lbl;

  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl)); // give the string a name
  _pf.SSTRING(node->value()); // output string characters

  if (!in_function()) {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl));
  } else {
    /* put the address in the stack */
    _pf.TEXT(_functionLabels.top());
    _pf.ADDR(mklbl(lbl));
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_unary_minus_node(cdk::unary_minus_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  _pf.NEG(); // 2-complement
}

void til::postfix_writer::do_unary_plus_node(cdk::unary_plus_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.ADD();
}
void til::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.SUB();
}
void til::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MUL();
}
void til::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.DIV();
}
void til::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}
void til::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.LT();
}
void til::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.LE();
}
void til::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.GE();
}
void til::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.GT();
}
void til::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.NE();
}
void til::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.EQ();
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // simplified generation: all variables are global
  _pf.ADDR(node->name());
}

void til::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDDOUBLE();
  } else {
    _pf.LDINT(); // ints, strings and pointers
  }
}

void til::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->rvalue()->accept(this, lvl); // determine the new value
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT))
      _pf.I2D();
    _pf.DUP64();
  } else {
    _pf.DUP32();
  }

  node->lvalue()->accept(this, lvl); // where to store the value
  if (node->lvalue()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.STDOUBLE();
  } else {
    _pf.STINT();
  }
}

void til::postfix_writer::do_index_node(til::index_node *const node, int lvl) {
  // TODO
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_program_node(til::program_node * const node, int lvl) {
  // generate the main function (RTS mandates that its name be "_main")
  _pf.TEXT();
  _pf.ALIGN();
  _pf.GLOBAL("_main", _pf.FUNC());
  _pf.LABEL("_main");

  _functionLabels.push("_main");

  frame_size_calculator fsc(_compiler, _symtab);
  node->accept(&fsc, lvl);
  std::cout << fsc.localsize() << std::endl;
  _pf.ENTER(fsc.localsize());

  auto retLabel = mklbl(++_lbl);

  _offset = 0;
  node->block()->accept(this, lvl + 2);

  // end the main function
  _pf.INT(0);
  _pf.STFVAL32();

  _pf.ALIGN();
  _pf.LABEL(retLabel);
  _pf.LEAVE();
  _pf.RET();

  // these are just a few library function imports
  _pf.EXTERN("readi");
  _pf.EXTERN("printi");
  _pf.EXTERN("prints");
  _pf.EXTERN("println");
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_function_node(til::function_node *const node, int lvl) {
  // TODO
}

void til::postfix_writer::do_return_node(til::return_node *const node, int lvl) {
  // TODO
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_evaluation_node(til::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  _pf.TRASH(node->argument()->type()->size());
}

void til::postfix_writer::do_block_node(til::block_node *const node, int lvl) {
  _symtab.push();
  node->declarations()->accept(this, lvl);
  node->instructions()->accept(this, lvl);
  _symtab.pop();
}

void til::postfix_writer::do_print_node(til::print_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto args_vec = node->arguments()->nodes();
  for (auto it = args_vec.rbegin(); it != args_vec.rend(); ++it) {
    auto expr_node = dynamic_cast<cdk::expression_node *> (*it);

    expr_node->accept(this, lvl); // determine the value to print
    if (expr_node->is_typed(cdk::TYPE_INT)) {
      _pf.CALL("printi");
      _pf.TRASH(4); // delete the printed value
    } else if (expr_node->is_typed(cdk::TYPE_STRING)) {
      _pf.CALL("prints");
      _pf.TRASH(4); // delete the printed value's address
    } else if (expr_node->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.CALL("prints");
      _pf.TRASH(4); // delete the printed value's address
    } else {
      std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
      exit(1);
    }
  }

  if (node->newline()) {
    _pf.CALL("println"); // print a newline
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_read_node(til::read_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (node->is_typed(cdk::TYPE_INT)) {
    _pf.CALL("readi");
    _pf.LDFVAL32();
  } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else {
    std::cerr << "FATAL: " << node->lineno() << ": cannot read type" << std::endl;
    return;
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_address_of_node(til::address_of_node *const node, int lvl) {
  // TODO
}

void til::postfix_writer::do_stack_alloc_node(til::stack_alloc_node *const node, int lvl) {
  // TODO
}

void til::postfix_writer::do_nullptr_node(til::nullptr_node *const node, int lvl) {
  // TODO
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_loop_node(til::loop_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  _pf.LABEL(mklbl(lbl1 = ++_lbl));
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl2 = ++_lbl));
  node->instruction()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl1));
  _pf.LABEL(mklbl(lbl2));
}

void til::postfix_writer::do_stop_node(til::stop_node *const node, int lvl) {
  // TODO
}

void til::postfix_writer::do_next_node(til::next_node *const node, int lvl) {
  // TODO
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_if_node(til::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1 = ++_lbl;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_if_else_node(til::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl2));
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_function_call_node(til::function_call_node *const node, int lvl) {
  // TODO
}

//---------------------------------------------------------------------------
void til::postfix_writer::do_declaration_node(til::declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  
  int typesize = node->type()->size();
  int offset = 0;
  if (in_function()) {
    _offset -= typesize;
    offset = _offset;
  }

  auto symbol = new_symbol();
  if (symbol) {
    symbol->offset(offset);
    reset_new_symbol();
  }

  /* Private declaration */
  if (in_function()) {
    if (node->initializer() == nullptr) {
      return;
    }

    node->initializer()->accept(this, lvl);
    if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING) || node->is_typed(cdk::TYPE_POINTER)) {
      _pf.LOCAL(symbol->offset());
      _pf.STINT();
    } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.LOCAL(symbol->offset());
      _pf.STDOUBLE();
    } else {
      std::cerr << node->lineno() << ": failed initializing" << std::endl;
    }

    return;
  }

  /* Global declaration */

  // Unitialized declaration
  if (node->initializer() == nullptr) {
    _pf.BSS();
    _pf.ALIGN();
    std::cout << symbol->name() << std::endl;
    _pf.LABEL(symbol->name());
    _pf.SALLOC(typesize);

    return;
  }

  _pf.DATA();
  _pf.ALIGN();
  _pf.LABEL(symbol->name());

  if (node->is_typed(cdk::TYPE_DOUBLE) && node->initializer()->is_typed(cdk::TYPE_INT)) {
    cdk::integer_node *int_node = dynamic_cast<cdk::integer_node *>(node->initializer());
    _pf.SDOUBLE(int_node->value());
  } else {
    node->initializer()->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_sizeof_node(til::sizeof_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  _pf.INT(node->argument()->type()->size());
}