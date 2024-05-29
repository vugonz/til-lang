#ifndef __OG_TARGET_FRAME_SIZE_CALCULATOR_H__
#define __OG_TARGET_FRAME_SIZE_CALCULATOR_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <stack>

namespace til {

class frame_size_calculator : public basic_ast_visitor {
  cdk::symbol_table<til::symbol> &_symtab;
  std::shared_ptr<til::symbol> _function;

  size_t _localsize;

public:
  frame_size_calculator(std::shared_ptr<cdk::compiler> compiler,
                        cdk::symbol_table<til::symbol> &symtab)
      : basic_ast_visitor(compiler), _symtab(symtab),
        _localsize(0) {}

public:
  ~frame_size_calculator();

public:
  size_t localsize() const { return _localsize; }

public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h" // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end
};

} // namespace til

#endif
