#include "targets/type_checker.h"
#include ".auto/all_nodes.h" // automatically generated
#include <cdk/types/primitive_type.h>
#include <string>

#define ASSERT_UNSPEC                                                          \
  {                                                                            \
    if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC))          \
      return;                                                                  \
  }

/**
 * Perform deep comparison of types.
 *
 * If relax is set to true it returns true for compatible types, which are int
 * and double, also for functional types with compatible arguments and/or return
 * values. If relax is set to false it does strict comparison of types
 */
bool til::type_checker::deep_compare_types(
    std::shared_ptr<cdk::basic_type> left,
    std::shared_ptr<cdk::basic_type> right, bool relax) {
  if (left->name() == cdk::TYPE_FUNCTIONAL) {
    if (right->name() != cdk::TYPE_FUNCTIONAL)
      return false;

    auto left_type = cdk::functional_type::cast(left);
    auto right_type = cdk::functional_type::cast(right);

    // Compare functions signatures
    if (left_type->output_length() != right_type->output_length() ||
        left_type->input_length() != right_type->input_length())
      return false;

    // Compare functions arguments
    for (size_t i = 0; i < left_type->input_length(); i++) {
      if (!deep_compare_types(left_type->input(i), right_type->input(i), relax))
        return false;
    }

    // Compare functions returns
    for (size_t i = 0; i < left_type->input_length(); i++) {
      if (!deep_compare_types(left_type->input(i), right_type->input(i), relax))
        return false;
    }

    return true;
  } else if (right->name() ==
             cdk::TYPE_FUNCTIONAL) { // left is not of functional type so right
                                     // shouldn't be
    return false;
  }

  // Compare pointer types
  if (left->name() == cdk::TYPE_POINTER) {
    if (right->name() != cdk::TYPE_POINTER)
      return false;

    auto left_type = cdk::reference_type::cast(left);
    auto right_type = cdk::reference_type::cast(right);

    // Compare referenced
    return deep_compare_types(left_type->referenced(), right_type->referenced(),
                              relax);
  } else if (right->name() ==
             cdk::TYPE_POINTER) { // left is not pointer so right shouldn't be
    return false;
  }

  // Compare remaining types
  if (relax)
    if (right->name() == cdk::TYPE_DOUBLE)
      return left->name() == cdk::TYPE_INT || left->name() == cdk::TYPE_DOUBLE;

  return left->name() == right->name();
}
//---------------------------------------------------------------------------

void til::type_checker::do_sequence_node(cdk::sequence_node *const node,
                                         int lvl) {
  for (auto &child : node->nodes()) {
    child->accept(this, lvl + 2);
  }
}

//---------------------------------------------------------------------------

void til::type_checker::processUnaryExpression(
    cdk::unary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT))
    throw std::string("wrong type in argument of unary expression");

  // in Simple, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void til::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}
void til::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void til::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void til::type_checker::do_integer_node(cdk::integer_node *const node,
                                        int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void til::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

void til::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void til::type_checker::do_nullptr_node(til::nullptr_node *const node,
                                        int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::reference_type::create(
      4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

//---------------------------------------------------------------------------

void til::type_checker::do_unary_minus_node(cdk::unary_minus_node *const node,
                                            int lvl) {
  processUnaryExpression(node, lvl);
}

void til::type_checker::do_unary_plus_node(cdk::unary_plus_node *const node,
                                           int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void til::type_checker::processBinaryExpression(
    cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT))
    throw std::string("wrong type in left argument of binary expression");

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT))
    throw std::string("wrong type in right argument of binary expression");

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void til::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void til::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void til::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void til::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void til::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void til::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void til::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void til::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void til::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void til::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void til::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void til::type_checker::do_variable_node(cdk::variable_node *const node,
                                         int lvl) {
  ASSERT_UNSPEC;
  auto symbol = _symtab.find(node->name());

  if (symbol == nullptr)
    throw std::string("undeclared variable '" + node->name() + "'");

  node->type(symbol->type());
}

void til::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl);
  node->type(node->lvalue()->type());
}

void til::type_checker::do_assignment_node(cdk::assignment_node *const node,
                                           int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl + 2);
  node->rvalue()->accept(this, lvl + 2);
  // (var x (read))
  if (node->lvalue()->is_typed(cdk::TYPE_UNSPEC) &&
      node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
    node->lvalue()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->rvalue()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    return;
  }

  if (node->lvalue()->is_typed(cdk::TYPE_UNSPEC)) {
    node->lvalue()->type(node->rvalue()->type());
  } else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
    node->rvalue()->type(node->lvalue()->type());
  }

  node->type(node->lvalue()->type());
}

void til::type_checker::do_index_node(til::index_node *const node, int lvl) {
  node->base()->accept(this, lvl + 2);

  if (!node->base()->is_typed(cdk::TYPE_POINTER)) {
    throw std::string("expected pointer type in index operator base");
  }

  node->index()->accept(this, lvl + 2);
  // unspec, assume it's a read node, type infer it to int
  if (node->index()->is_typed(cdk::TYPE_UNSPEC)) {
    node->index()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->index()->is_typed(cdk::TYPE_INT)) {
    throw std::string("expected integer type in index operator index");
  }

  // type cast for reference_type information
  auto basetype = cdk::reference_type::cast(node->base()->type());

  // unspec, assume pointer to int
  if (basetype->referenced()->name() == cdk::TYPE_UNSPEC) {
    basetype = cdk::reference_type::create(
        4, cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->base()->type(basetype);
  }

  // node type is pointer of the same type of base child
  node->type(basetype->referenced());
}

//---------------------------------------------------------------------------

void til::type_checker::do_program_node(til::program_node *const node,
                                        int lvl) {
  node->block()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void til::type_checker::do_function_node(til::function_node *const node,
                                         int lvl) {
  // TODO
}

void til::type_checker::do_return_node(til::return_node *const node, int lvl) {
  auto symbol = _symtab.find("@");
  if (symbol == nullptr) {
    // probably inside program func
    auto prog_symbol = _symtab.find("_main");

    if (prog_symbol == nullptr)
      throw std::string("return statement outside function definition");

    if (!node->ret_val())
      throw std::string("wrong type of return value in main (int expected)");
    
    node->ret_val()->accept(this, lvl + 2);

    if (!node->ret_val()->is_typed(cdk::TYPE_INT))
      throw std::string("wrong type of return value in main (int expected)");

    return;
  }

  std::shared_ptr<cdk::functional_type> func_type = cdk::functional_type::cast(symbol->type());

  if (node->ret_val() == nullptr) {
    if (func_type->output(0)->name() == cdk::TYPE_VOID) {
      throw std::string("return value missing in non-void function");
    }
    return;
  }

  if (func_type->output(0)->name() == cdk::TYPE_VOID) {
    throw std::string("non void return value in void function");
  }

  node->ret_val()->accept(this, lvl + 2);

  if (!deep_compare_types(node->ret_val()->type(), func_type->output(0), true))
    throw std::string("incompatible return value for funciton");
}

//---------------------------------------------------------------------------
void til::type_checker::do_evaluation_node(til::evaluation_node *const node,
                                           int lvl) {
  // evaluate the node by type checking it's expression
  node->argument()->accept(this, lvl + 2);
  // if unspec, assume it's a read node, type infer it to int
  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->argument()->is_typed(cdk::TYPE_POINTER)) {
    auto ref = cdk::reference_type::cast(node->argument()->type());

    if (ref != nullptr && ref->referenced()->name() == cdk::TYPE_UNSPEC) {
      // (double !p (objects 5)) this is where update the referenced type to
      // double
      node->argument()->type(cdk::reference_type::create(
          4, cdk::primitive_type::create(4, cdk::TYPE_INT)));
    }
  }
}

void til::type_checker::do_print_node(til::print_node *const node, int lvl) {
  node->arguments()->accept(this, lvl + 2);

  for (auto &arg : node->arguments()->nodes()) {
    // typecast to typed node (only need type information)
    cdk::typed_node *typed_node = dynamic_cast<cdk::typed_node *>(arg);
    if (typed_node == nullptr) { // not an expression node
      throw std::string("invalid argument type for print instruction");
    }

    // if unspec, assume it's a read node, type infer it to int
    if (typed_node->is_typed(cdk::TYPE_UNSPEC)) {
      typed_node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (!typed_node->is_typed(cdk::TYPE_INT) &&
               !typed_node->is_typed(cdk::TYPE_STRING) &&
               !typed_node->is_typed(cdk::TYPE_DOUBLE)) {
      throw std::string("invalid argument type for print instruction");
    }
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_read_node(til::read_node *const node, int lvl) {
  ASSERT_UNSPEC;
  // gets type infered by parent nodes
  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void til::type_checker::do_address_of_node(til::address_of_node *const node,
                                           int lvl) {
  ASSERT_UNSPEC
  node->lvalue()->accept(this, lvl + 2);
  if (node->lvalue()->is_typed(cdk::TYPE_POINTER)) {
    auto ref = cdk::reference_type::cast(
        node->lvalue()->type()); // cast for pointer information
    if (ref->referenced()->name() ==
        cdk::TYPE_VOID) // if lval is void pointer aka generic pointer
      node->type(node->lvalue()->type()); // it's address is also a void pointer
                                          // (!void == !!void == ...)
  }
  node->type(cdk::reference_type::create(4, node->lvalue()->type()));
}

void til::type_checker::do_stack_alloc_node(til::stack_alloc_node *const node,
                                            int lvl) {
  ASSERT_UNSPEC
  node->argument()->accept(this, lvl + 2);

  // if unspec, assume it's a read node, type infer it to int
  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string(
        "expected integer type in stack allocation operator argument");
  }

  node->type(cdk::reference_type::create(
      4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

//---------------------------------------------------------------------------

void til::type_checker::do_loop_node(til::loop_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);

  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {
    node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->condition()->is_typed(cdk::TYPE_INT)) {
    throw std::string("expected integer type in loop instruction condition");
  }

  node->instruction()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void til::type_checker::do_block_node(til::block_node *const node, int lvl) {
  // EMPTY
}

void til::type_checker::do_stop_node(til::stop_node *const node, int lvl) {
  // EMPTY
}

void til::type_checker::do_next_node(til::next_node *const node, int lvl) {
  // EMPTY
}

void til::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}

void til::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_if_node(til::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);

  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {
    node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->condition()->is_typed(cdk::TYPE_INT)) {
    throw std::string(
        "expected integer type in conditional instruction condition");
  }

  node->block()->accept(this, lvl + 2);
}

void til::type_checker::do_if_else_node(til::if_else_node *const node,
                                        int lvl) {
  node->condition()->accept(this, lvl + 2);

  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {
    node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->condition()->is_typed(cdk::TYPE_INT)) {
    throw std::string(
        "expected integer type in conditoinal instruction condition");
  }

  node->thenblock()->accept(this, lvl + 2);
  node->elseblock()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void til::type_checker::do_function_call_node(
    til::function_call_node *const node, int lvl) {
  ASSERT_UNSPEC
  std::shared_ptr<cdk::functional_type> func_type;
  if (node->func()) { // regular call
    node->func()->accept(this, lvl);

    if (!node->func()->is_typed(cdk::TYPE_FUNCTIONAL)) {
      throw std::string("wrong type in function call");
    }

    func_type = cdk::functional_type::cast(node->func()->type());

    if (func_type->input()->length() != node->arguments()->size()) {
      throw std::string("wrong number of arguments in function call");
    }

    node->type(func_type->output(0));
  } else {
    throw std::string("not implemented");
  }

  for (size_t i = 0; i < node->arguments()->size(); i++) {
    auto arg = dynamic_cast<cdk::expression_node *>(node->arguments()->node(i));
    arg->accept(this, lvl);

    auto param_type = func_type->input(i);
    if (arg->is_typed(cdk::TYPE_UNSPEC)) {
      if (param_type->name() == cdk::TYPE_INT) {
        arg->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      } else if (param_type->name() == cdk::TYPE_DOUBLE) {
        arg->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
      } else {
        throw std::string("wrong argument type provided in function call");
      }
    } else if (arg->is_typed(cdk::TYPE_POINTER) && // pointer logic comparison here
               param_type->name() == cdk::TYPE_POINTER) {
      auto param_ref = cdk::reference_type::cast(param_type);
      auto arg_ref = cdk::reference_type::cast(arg->type());

      if (arg_ref->referenced()->name() == cdk::TYPE_UNSPEC ||
          arg_ref->referenced()->name() ==
              cdk::TYPE_VOID // void pointer magic happens here
          || param_ref->referenced()->name() == cdk::TYPE_VOID) {
        arg->type(param_type);
      }
    }

    if (!deep_compare_types(param_type, arg->type(), true)) {
      throw std::string("wrong argument type provided in function call");
    }
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_declaration_node(til::declaration_node *const node,
                                            int lvl) {
  if (node->type() == nullptr) { // var
    node->initializer()->accept(this, lvl + 2);
    if (node->initializer()->is_typed(cdk::TYPE_VOID)) {
      throw std::string("cannot declare variable of type void");
    } else if (node->initializer()->is_typed(
                   cdk::TYPE_UNSPEC)) { // (var x (read))
      node->initializer()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (node->initializer()->is_typed(cdk::TYPE_POINTER)) {
      auto ref = cdk::reference_type::cast(node->initializer()->type());
      if (ref->referenced()->name() ==
          cdk::TYPE_UNSPEC) { // (var x (objects 5))
        node->initializer()->type(cdk::reference_type::create(
            4, cdk::primitive_type::create(4, cdk::TYPE_INT)));
      }
    }
    node->type(node->initializer()->type());
  } else { // static type
    if (node->initializer() != nullptr) {
      node->initializer()->accept(this, lvl + 2);

      if (node->initializer()->is_typed(cdk::TYPE_UNSPEC)) { // (read)
        if (node->is_typed(cdk::TYPE_DOUBLE)) {
          node->initializer()->type(
              cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
        } else if (node->is_typed(cdk::TYPE_INT)) {
          node->initializer()->type(
              cdk::primitive_type::create(4, cdk::TYPE_INT));
        } else {
          throw std::string(
              "conflicting initializer expression type for variable'" +
              node->identifier() + "'");
        }
      } else if (node->initializer()->is_typed(cdk::TYPE_POINTER) &&
                 node->is_typed(cdk::TYPE_POINTER)) {
        auto initref = cdk::reference_type::cast(node->initializer()->type());
        if (initref->referenced()->name() == cdk::TYPE_UNSPEC ||
            initref->referenced()->name() == cdk::TYPE_VOID) {
          // int! p (objects 5) -> type cast expression (objects 5) to
          // appropriate pointer type int! p (void!) -> implicit typecast à lá C
          // (int *p = m*lloc(...)), cant use func name...
          node->initializer()->type(node->type());
        }
      }

      if (!deep_compare_types(node->type(), node->initializer()->type(),
                              true)) {
        throw std::string(
            "'conflicting initializer expression type for variable'" +
            node->identifier() + "'");
      }
    }
  }

  auto new_symbol =
      til::make_symbol(node->identifier(), node->type(), node->qualifier());
  if (!_symtab.insert(new_symbol->name(), new_symbol)) {
    // Redeclaration of variable
    const auto previous_symbol = _symtab.find(node->identifier());
    if (deep_compare_types(previous_symbol->type(), new_symbol->type(),
                           false)) {
      throw std::string("redeclaration of variable '" + node->identifier() +
                        "' with conflicting type");
    }
    _symtab.replace(new_symbol->name(), new_symbol);
    _parent->set_new_symbol(new_symbol);
    return;
  }
  _parent->set_new_symbol(new_symbol);
}

//---------------------------------------------------------------------------

void til::type_checker::do_sizeof_node(til::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);

  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }
