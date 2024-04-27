#ifndef __TIL_AST_IDENTITY_NODE_H__
#define __TIL_AST_IDENTITY_NODE_H__

#include <cdk/ast/unary_operation_node.h>

namespace til {

/**
 * Class for describing the identity operator.
 */
class identity_node : public cdk::unary_operation_node {
  public:
    identity_node(int lineno, expression_node *arg)
        : cdk::unary_operation_node(lineno, arg) {}

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_identity_node(this, level);
    }
};

} // namespace til

#endif