#ifndef __TIL_AST_FUNCTION_NODE_H__
#define __TIL_AST_FUNCTION_NODE_H__

#include "block_node.h"
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace til {

/**
 * Class for describing function definition nodes.
 */
class function_node : public cdk::typed_node {
    cdk::sequence_node *_arguments;
    til::block_node *_block;

  public:
    function_node(int lineno,
                             std::shared_ptr<cdk::basic_type> func_type,
                             cdk::sequence_node *arguments,
                             til::block_node *block)
        : cdk::typed_node(lineno), _arguments(arguments), _block(block) {
        type(func_type);
    }

  public:
    cdk::sequence_node *arguments() { return _arguments; }
    til::block_node *block() { return _block; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_function_node(this, level);
    }
};

} // namespace til

#endif