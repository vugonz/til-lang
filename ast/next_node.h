#ifndef __TIL_AST_NEXT_NODE_H__
#define __TIL_AST_NEXT_NODE_H__

#include <cdk/ast/basic_node.h>

namespace til {

/**
 * Class for describing stop nodes.
 */
class next_node : public cdk::basic_node {
    int _level;

  public:
    next_node(int lineno, int level = 1)
        : cdk::basic_node(lineno), _level(level) {}

  public:
    inline int level() const { return _level; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_next_node(this, level);
    }
};

} // namespace til

#endif