#ifndef __TIL_AST_FUNCTION_NODE_H__
#define __TIL_AST_FUNCTION_NODE_H__

#include "block_node.h"
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/types/basic_type.h>
#include <cdk/types/functional_type.h>
#include <cdk/types/primitive_type.h>
#include <vector>

namespace til {

/**
 * Class for describing function definition nodes.
 */
class function_node : public cdk::expression_node {
    cdk::sequence_node *_arguments;
    til::block_node *_block;
    bool _main = false;

  public:
    function_node(int lineno, std::shared_ptr<cdk::basic_type> func_type,
                  cdk::sequence_node *arguments, til::block_node *block)
        : cdk::expression_node(lineno), _arguments(arguments), _block(block) {
        std::vector<std::shared_ptr<cdk::basic_type>> arg_types;
        for (size_t i; i < arguments->size(); i++) {
            arg_types.push_back(
                dynamic_cast<cdk::typed_node *>(arguments->node(i))->type());
        }

        type(cdk::functional_type::create(arg_types, func_type));
    }

    /**
     * Main function node constructor
     */
    function_node(int lineno, til::block_node *block)
        : cdk::expression_node(lineno),
          _arguments(new cdk::sequence_node(lineno)), _block(block),
          _main(true) {
        type(cdk::functional_type::create(
            cdk::primitive_type::create(4, cdk::TYPE_INT)));
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