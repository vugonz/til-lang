#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void til::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (auto &child : node->nodes()) {
    child->accept(this, lvl + 2);
  }
}

//---------------------------------------------------------------------------

void til::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in argument of unary expression");

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

void til::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
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

void til::type_checker::do_nullptr_node(til::nullptr_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

//---------------------------------------------------------------------------

void til::type_checker::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void til::type_checker::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void til::type_checker::processBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) 
    throw std::string("wrong type in left argument of binary expression");

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of binary expression");

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

void til::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<til::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id;
  }
}

void til::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void til::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl + 2);
  node->rvalue()->accept(this, lvl + 2);

  if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
    node->rvalue()->type(node->lvalue()->type());
  }


  /*
  try {
    node->lvalue()->accept(this, lvl);
  } catch (const std::string &id) {
    auto symbol = make_symbol(id, cdk::primitive_type::create(4, cdk::TYPE_INT));
    _symtab.insert(id, symbol);
    _parent->set_new_symbol(symbol);  // advise parent that a symbol has been inserted
    node->lvalue()->accept(this, lvl);  //DAVID: bah!
  }

  if (!node->lvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of assignment expression");

  node->rvalue()->accept(this, lvl + 2);
  if (!node->rvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of assignment expression");

  // in Simple, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  */
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
    basetype = cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->base()->type(basetype);
  }

  // node type is pointer of the same type of base child
  node->type(basetype->referenced());
}

//---------------------------------------------------------------------------

void til::type_checker::do_program_node(til::program_node *const node, int lvl) {
  node->block()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void til::type_checker::do_function_node(til::function_node *const node, int lvl) {
  // TODO
}

void til::type_checker::do_return_node(til::return_node *const node, int lvl) {
  // TODO
}

//---------------------------------------------------------------------------
void til::type_checker::do_evaluation_node(til::evaluation_node *const node, int lvl) {
  // evaluate the node by type checking it's expression
  node->argument()->accept(this, lvl + 2);


  // if unspec, assume it's a read node, type infer it to int
  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->argument()->is_typed(cdk::TYPE_POINTER)) {
    auto ref = cdk::reference_type::cast(node->argument()->type());

    // default to int pointer if unspec pointer
    if (ref != nullptr && ref->referenced()->name() == cdk::TYPE_UNSPEC) {
      node->argument()->type(cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT)));
    }
  }
}


void til::type_checker::do_print_node(til::print_node *const node, int lvl) {
  node->arguments()->accept(this, lvl + 2);

  for (auto &arg : node->arguments()->nodes()) {
    // typecast to typed node (only need type information)
    cdk::typed_node *typed_node = dynamic_cast<cdk::typed_node*>(arg);
    if (typed_node == nullptr) { // not an expression node
        throw std::string("invalid argument type for print instruction");
    }

    // if unspec, assume it's a read node, type infer it to int
    if (typed_node->is_typed(cdk::TYPE_UNSPEC)) {
      typed_node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (!typed_node->is_typed(cdk::TYPE_INT) && !typed_node->is_typed(cdk::TYPE_STRING) && !typed_node->is_typed(cdk::TYPE_DOUBLE)) {
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

void til::type_checker::do_address_of_node(til::address_of_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->lvalue()->accept(this, lvl + 2);
  node->type(cdk::reference_type::create(4, node->lvalue()->type()));
}

void til::type_checker::do_stack_alloc_node(til::stack_alloc_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->argument()->accept(this, lvl + 2);

  // if unspec, assume it's a read node, type infer it to int
  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string("expected integer type in stack allocation operator argument");
  }

  // pointer to unspec type, primitive type depends on context
  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
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
    throw std::string("expected integer type in conditional instruction condition");
  }

  node->block()->accept(this, lvl + 2);
}

void til::type_checker::do_if_else_node(til::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);

  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {
    node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->condition()->is_typed(cdk::TYPE_INT)) {
    throw std::string("expected integer type in conditoinal instruction condition");
  }

  node->thenblock()->accept(this, lvl + 2);
  node->elseblock()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void til::type_checker::do_function_call_node(til::function_call_node *const node, int lvl) {
  // TODO
}

//---------------------------------------------------------------------------

void til::type_checker::do_declaration_node(til::declaration_node *const node, int lvl) {
  // if we have initializer expression
  if (node->initializer() != nullptr) {
    node->initializer()->accept(this, lvl + 2);

    // static typed declaration
    if (node->type() != nullptr) {
      if (node->initializer()->is_typed(cdk::TYPE_UNSPEC)) {
        node->initializer()->type(node->type());
      } else if (!node->initializer()->is_typed(node->type()->name())) {
        throw std::string("wrong type in initializer for variable '" + node->identifier() + "'");
      }
    } else { // var 
      node->type(node->initializer()->type());
    }

    // cant be void type declaration
    if (node->is_typed(cdk::TYPE_VOID)) {
      throw std::string("cannot declare variable of type void");
    }
  }

  auto symbol = til::make_symbol(node->identifier(), node->type(), node->qualifier());

  if (_symtab.insert(symbol->name(), symbol)) {
    _parent->set_new_symbol(symbol);
    return;
  }
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