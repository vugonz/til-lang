#ifndef __TIL_AST_SIZEOF_NODE_H__
#define __TIL_AST_SIZEOF_NODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/unary_operation_node.h>

namespace til {

/**
 * Class for describing the sizeof operator.
 */
class sizeof_node : public cdk::unary_operation_node {
    cdk::expression_node *_argument;

  public:
    sizeof_node(int lineno, cdk::expression_node *argument)
        : cdk::unary_operation_node(lineno, argument) {}

  public:
    void accept(basic_ast_visitor *sp, int level) {
        sp->do_sizeof_node(this, level);
    }
};

} // namespace til

#endif