#ifndef __TIL_AST_RETURN_NODE_H__
#define __TIL_AST_RETURN_NODE_H__

#include <cdk/ast/basic_node.h>
#include <cdk/ast/expression_node.h>

namespace til {

class return_node : public cdk::basic_node {
    cdk::expression_node *_ret_val;

  public:
    return_node(int lineno, cdk::expression_node *ret_val)
        : cdk::basic_node(lineno), _ret_val(ret_val) {}

  public:
    cdk::expression_node *ret_val() { return _ret_val; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_return_node(this, level);
    }
};

} // namespace til

#endif