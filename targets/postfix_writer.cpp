#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated

#include "til_parser.tab.h"

//---------------------------------------------------------------------------

void til::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void til::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}

void til::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->argument()->accept(this, lvl + 2);
  _pf.INT(0);
  _pf.EQ();
}

void til::postfix_writer::do_unary_minus_node(cdk::unary_minus_node* const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->argument()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_INT)) {
    _pf.NEG();
  } else {
    _pf.DNEG();
  }
}

void til::postfix_writer::do_unary_plus_node(cdk::unary_plus_node* const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->argument()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  auto lbl = mklbl(++_lbl);
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JZ(lbl);
  node->right()->accept(this, lvl + 2);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(lbl);
}

void til::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto lbl = mklbl(++_lbl);
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JNZ(lbl);
  node->right()->accept(this, lvl + 2);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(lbl);
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
    _pf.TEXT(_function_lbls.top());
    _pf.ADDR(mklbl(lbl));
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::pre_process_int_double_pointer_binary_expr(
    cdk::binary_operation_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) &&
      !node->left()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();
  else if (node->is_typed(cdk::TYPE_POINTER) &&
           !node->left()->is_typed(cdk::TYPE_POINTER)) {
    const auto ref_right =
        cdk::reference_type::cast(node->right()->type())->referenced();
    _pf.INT(std::max(
        1, static_cast<int>(ref_right->size())));
    _pf.MUL();
  }

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) &&
      !node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();
  else if (node->is_typed(cdk::TYPE_POINTER) &&
           !node->right()->is_typed(cdk::TYPE_POINTER)) {
    const auto ref_left =
        cdk::reference_type::cast(node->left()->type())->referenced();
    _pf.INT(std::max(
        1, static_cast<int>(ref_left->size())));
    _pf.MUL();
  }
}
void til::postfix_writer::do_add_node(cdk::add_node *const node, int lvl) {
  pre_process_int_double_pointer_binary_expr(node, lvl);

  if (!node->is_typed(cdk::TYPE_DOUBLE))
    _pf.ADD();
  else
    _pf.DADD();
}

void til::postfix_writer::do_sub_node(cdk::sub_node *const node, int lvl) {
  pre_process_int_double_pointer_binary_expr(node, lvl);

  if (!node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.SUB();
    // pointer - pointer requires a special treatment
    if ((node->left()->is_typed(cdk::TYPE_POINTER) &&
         node->right()->is_typed(cdk::TYPE_POINTER)) &&
        cdk::reference_type::cast(node->left()->type())->referenced()->name() !=
            cdk::TYPE_VOID) {
      _pf.INT(cdk::reference_type::cast(node->left()->type())
                  ->referenced()
                  ->size());
      _pf.DIV();
    }
  } else {
    _pf.DSUB();
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::pre_process_int_double_binary_expr(
    cdk::binary_operation_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) &&
      !node->left()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) &&
      !node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();
}

void til::postfix_writer::do_mul_node(cdk::mul_node *const node, int lvl) {
  pre_process_int_double_binary_expr(node, lvl);

  if (!node->is_typed(cdk::TYPE_DOUBLE))
    _pf.MUL();
  else
    _pf.DMUL();
}

void til::postfix_writer::do_div_node(cdk::div_node *const node, int lvl) {
  pre_process_int_double_binary_expr(node, lvl);

  if (!node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DIV();
  else
    _pf.DDIV();
}

void til::postfix_writer::do_mod_node(cdk::mod_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}

//---------------------------------------------------------------------------

void til::postfix_writer::pre_process_logical_binary_expr(
    cdk::binary_operation_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_DOUBLE) &&
      node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_DOUBLE) &&
      node->left()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) ||
      node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DCMP();
    _pf.INT(0);
  }
}

void til::postfix_writer::do_lt_node(cdk::lt_node *const node, int lvl) {
  pre_process_logical_binary_expr(node, lvl);
  _pf.LT();
}

void til::postfix_writer::do_le_node(cdk::le_node *const node, int lvl) {
  pre_process_logical_binary_expr(node, lvl);
  _pf.LE();
}

void til::postfix_writer::do_ge_node(cdk::ge_node *const node, int lvl) {
  pre_process_logical_binary_expr(node, lvl);
  _pf.GE();
}

void til::postfix_writer::do_gt_node(cdk::gt_node *const node, int lvl) {
  pre_process_logical_binary_expr(node, lvl);
  _pf.GT();
}

void til::postfix_writer::do_ne_node(cdk::ne_node *const node, int lvl) {
  pre_process_logical_binary_expr(node, lvl);
  _pf.NE();
}

void til::postfix_writer::do_eq_node(cdk::eq_node *const node, int lvl) {
  pre_process_logical_binary_expr(node, lvl);
  _pf.EQ();
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  const std::string &id = node->name();
  auto symbol = _symtab.find(id);
  if (symbol->is_global()) {
    _pf.ADDR(symbol->name());
  } else {
    _pf.LOCAL(symbol->offset());
  }
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

  _function_lbls.push("_main");
  // treated as just line any other function
  auto prog_symbol = til::make_symbol("@", cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_INT)), tPRIVATE);
  _symtab.insert("@", prog_symbol);

  frame_size_calculator fsc(_compiler, _symtab);
  node->accept(&fsc, lvl);
  _pf.ENTER(fsc.localsize());

  auto ret_lbl = mklbl(++_lbl);
  _current_function_ret_lbl = ret_lbl;

  _offset = 0;
  node->block()->accept(this, lvl + 2);

  // end the main function
  _pf.ALIGN();
  _pf.LABEL(ret_lbl);
  _pf.LEAVE();
  _pf.RET();

  // declare the extern functions 
  for (const auto &ext_func : _external_funcs) {
    std::cerr << ext_func << std::endl;
    _pf.EXTERN(ext_func);
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_function_node(til::function_node *const node, int lvl) {
  auto func_lbl = mklbl(++_lbl);
  _function_lbls.push(func_lbl);

  // create function symbol in this context
  auto function_sym = til::make_symbol("@", node->type(), tPRIVATE);
  if (!_symtab.insert(function_sym->name(), function_sym)) {
    _symtab.replace(function_sym->name(), function_sym);
  }
  
  _functions.push(function_sym);

  /** Argument handling */
  const int prev_offset = _offset;
  _offset = 8; // argument variables

  _func_args_decl = true;
  if (node->arguments()) 
    node->arguments()->accept(this, lvl + 2);
  _func_args_decl = false;

  _pf.TEXT();
  _pf.ALIGN();
  _pf.LABEL(func_lbl);

  auto ret_lbl = mklbl(++_lbl);
  _current_function_ret_lbl = ret_lbl;
  /** Local variables handling */
  frame_size_calculator fsc(_compiler, _symtab);
  node->block()->accept(&fsc, lvl);
  _pf.ENTER(fsc.localsize());

  _offset = 0; // local variables
  node->block()->accept(this, lvl + 2);
  _offset = prev_offset; // reset offset

  /** Return handling */
  _pf.ALIGN();
  _pf.LABEL(ret_lbl);
  _pf.LEAVE();
  _pf.RET();

  _function_lbls.pop();
  _functions.pop();
  _current_function_ret_lbl = nullptr;
}

void til::postfix_writer::do_return_node(til::return_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  auto symbol = _symtab.find("@"); // type checker ensures it exists 

  if (!symbol->is_typed(cdk::TYPE_VOID)) {
    node->ret_val()->accept(this, lvl + 2);
    if (!symbol->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.STFVAL32();
    } else {
      _pf.I2D();
      _pf.STFVAL64();
    }
  }

  _pf.JMP(_current_function_ret_lbl);
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
      _external_funcs.insert("printi");
      _pf.CALL("printi");
      _pf.TRASH(4); // delete the printed value
    } else if (expr_node->is_typed(cdk::TYPE_STRING)) {
      _external_funcs.insert("prints");
      _pf.CALL("prints");
      _pf.TRASH(4); // delete the printed value's address
    } else if (expr_node->is_typed(cdk::TYPE_DOUBLE)) {
      _external_funcs.insert("printd");
      _pf.CALL("printd");
      _pf.TRASH(8); // delete the printed value's address
    }  
  }

  if (node->newline()) {
    _external_funcs.insert("println");
    _pf.CALL("println"); // print a newline
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_read_node(til::read_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (node->is_typed(cdk::TYPE_INT)) {
    _external_funcs.insert("readi");
    _pf.CALL("readi");
    _pf.LDFVAL32();
  } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _external_funcs.insert("readd");
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else {
    THROW_ERROR(node, "cannot read type");
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
  int loop_start_lbl = ++_lbl;
  int loop_end_lbl = ++_lbl;

  _loop_start_lbls.push_back(loop_start_lbl);
  _loop_end_lbls.push_back(loop_end_lbl);
  _symtab.push();

  _pf.LABEL(mklbl(loop_start_lbl));

  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(loop_end_lbl));
  node->instruction()->accept(this, lvl + 2);
  _pf.JMP(mklbl(loop_start_lbl));
  _pf.LABEL(mklbl(loop_end_lbl));

  _symtab.pop();
  _loop_start_lbls.pop_back();
  _loop_end_lbls.pop_back();
}

void til::postfix_writer::do_stop_node(til::stop_node *const node, int lvl) {
  auto loop_lbls_count = _loop_start_lbls.size();

  if (loop_lbls_count == 0)
    THROW_ERROR(node, "stop instruction oustide loop");
  
  if ((size_t) node->level() > loop_lbls_count)
    THROW_ERROR(node, "invalid stop level ", std::to_string(node->level()));

  _pf.JMP(mklbl(_loop_end_lbls[loop_lbls_count - node->level()]));
}

void til::postfix_writer::do_next_node(til::next_node *const node, int lvl) {
  auto loop_lbls_count = _loop_start_lbls.size();

  if (loop_lbls_count == 0)
    THROW_ERROR(node, "next instruction oustide loop");

  if ((size_t) node->level() > loop_lbls_count)
    THROW_ERROR(node, "invalid next level ", std::to_string(node->level()));

  _pf.JMP(mklbl(_loop_start_lbls[loop_lbls_count - node->level()]));
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
  ASSERT_SAFE_EXPRESSIONS
  auto func_type = node->func()->type();
  std::vector<std::shared_ptr<cdk::basic_type>> arg_types;
  if (node->func()) { // normal function call
    arg_types = 
        cdk::functional_type::cast(func_type)->input()->components();
  }

  size_t args_size = 0;
  for (int i = node->arguments()->size() - 1; i >= 0; --i) {
    auto arg = dynamic_cast<cdk::expression_node *>(node->arguments()->node(i));
    args_size += arg->type()->size();
    // accept covariant arguments
    arg->accept(this, lvl + 2);
    if (arg_types[i]->name() == cdk::TYPE_DOUBLE &&
            arg->type()->name() == cdk::TYPE_INT) {
        args_size += 4;
        _pf.I2D();
      }
  }

  if (node->func()) {
    node->func()->accept(this, lvl); // call func expr
    _pf.BRANCH(); // because functions are just variables with addresses
  }

  if (args_size > 0) {
    _pf.TRASH(args_size);
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();
  } else if (!node->is_typed(cdk::TYPE_VOID)) {
    _pf.LDFVAL32();
  }
}

//---------------------------------------------------------------------------
void til::postfix_writer::do_declaration_node(til::declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  
  int typesize = node->type()->size();
  int offset = 0;

  if (_func_args_decl) {
    offset = _offset;      // func args start 8 and go up (_offset is 8 if here)
    _offset += typesize;
  } else if (in_function()) {
    _offset -= typesize;   // local variables start at 0 and go down 
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
      THROW_ERROR(node, "failed initialization");
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