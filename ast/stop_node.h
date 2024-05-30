#ifndef __TIL_AST_STOP_NODE_H__
#define __TIL_AST_STOP_NODE_H__

#include <cdk/ast/basic_node.h>

namespace til {

/**
 * Class to describe stop nodes.
 */
class stop_node : public cdk::basic_node {
    int _level;

  public:
    stop_node(int lineno, int level = 1)
        : cdk::basic_node(lineno), _level(level) {}

  public:
    inline int level() const { return _level; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_stop_node(this, level);
    }
};

} // namespace til

#endif