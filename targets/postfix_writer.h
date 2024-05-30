#ifndef __SIMPLE_TARGETS_POSTFIX_WRITER_H__
#define __SIMPLE_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <stack>
#include <cdk/emitters/basic_postfix_emitter.h>

namespace til {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<til::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;

    std::stack<std::string> _function_lbls;

    std::string _current_func_lbl;
    std::string _current_body_ret_lbl;

    std::vector<int> _loop_start_lbls;
    std::vector<int> _loop_end_lbls;

    int _offset = 0; // current frame pointer offset

    int _lbl;

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<til::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0) {
    }

  public:
    ~postfix_writer() {
      os().flush();
    }

    inline bool in_function() {
      return _function_lbls.size() > 0;
    }

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // til

#endif

#define THROW_ERROR(node, ...) \
  do { \
    std::cerr << "error: " << node->lineno() << ": "; \
    std::cerr << __VA_ARGS__; \
    std::cerr << std::endl; \
    return; \
  } while(0);
