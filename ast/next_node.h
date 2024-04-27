#ifndef __TIL_AST_NEXT_NODE_H__
#define __TIL_AST_NEXT_NODE_H__

#include <cdk/ast/basic_node.h>

namespace til {

/**
 * Class for describing stop nodes.
 */
class next_node : public cdk::basic_node {
    int _iteration_no;

  public:
    next_node(int lineno, int iteration_no)
        : cdk::basic_node(lineno), _iteration_no(iteration_no) {}

  public:
    inline int iteration_no() const { return _iteration_no; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_next_node(this, level);
    }
};

} // namespace til

#endif