%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                                           i;           /* integer value */
  double                                        d;           /* double value */
  std::string                                   *s;          /* symbol name or string literal */
  cdk::basic_node                               *node;       /* node pointer */
  cdk::sequence_node                            *sequence;
  cdk::expression_node                          *expression; /* expression nodes */
  cdk::lvalue_node                              *lvalue;
  til::program_node                             *program;
  til::block_node                               *block;      /* block node */
  std::vector<std::shared_ptr<cdk::basic_type>> *type_vec;
};

%token <i> tINTEGER
%token <d> tDOUBLE
%token <s> tIDENTIFIER tSTRING
%token tTYPE_INT tTYPE_DOUBLE tTYPE_STRING tTYPE_VOID
%token tEXTERNAL tFORWARD tPUBLIC tVAR tPRIVATE
%token tLOOP tSTOP tNEXT tRETURN
%token tIF
%token tREAD tNULL tSIZEOF tINDEX tOBJECTS
%token tPROGRAM
%token tBLOCK
%token tPRINT tPRINTLN
%token tFUNCTION

%right tSET
%left tOR 
%left tAND 
%nonassoc '~'
%left tEQ tNE 
%left tGE tLE '>' '<'
%left '+' '-'
%left '*' '/' '%'

%type <sequence> file gdecls exprs decls instrs
%type <node> gdecl instr decl
%type <type> type data_type func_type ref_type
%type <type_vec> types
%type <block> block
%type <expression> expr integer double function
%type <lvalue> lval
%type<s> string
%type <program> program

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}

%%

file : gdecls program { compiler->ast(new cdk::sequence_node(LINE, $2, $1)); }
     | gdecls         { compiler->ast($1); }
     |        program { compiler->ast(new cdk::sequence_node(LINE, $1)); }
     | /* empty */    { compiler->ast(new cdk::sequence_node(LINE)); }
     ;

gdecls : gdecls gdecl { $$ = new cdk::sequence_node(LINE, $2, $1);}
       | gdecl        { $$ = new cdk::sequence_node(LINE, $1); }
       ;

gdecl : '(' tEXTERNAL     func_type tIDENTIFIER ')'      { $$ = new til::declaration_node(LINE, tEXTERNAL, $3,      *$4, nullptr); }
      | '(' tFORWARD      type      tIDENTIFIER ')'      { $$ = new til::declaration_node(LINE, tFORWARD,  $3,      *$4, nullptr); }
      | '(' tPUBLIC       type      tIDENTIFIER ')'      { $$ = new til::declaration_node(LINE, tPUBLIC,   $3,      *$4, nullptr); }
      | '(' tPUBLIC       type      tIDENTIFIER expr ')' { $$ = new til::declaration_node(LINE, tPUBLIC,   $3,      *$4, $5); }
      | '(' tPUBLIC tVAR            tIDENTIFIER expr ')' { $$ = new til::declaration_node(LINE, tPUBLIC,   nullptr, *$4, $5); }
      | '(' tPUBLIC                 tIDENTIFIER expr ')' { $$ = new til::declaration_node(LINE, tPUBLIC,   nullptr, *$3, $4); }
      |  decl                                            { $$ = $1; } // private by default
      ;

decls : decls decl { $$ = new cdk::sequence_node(LINE, $2, $1); }
      | decl       { $$ = new cdk::sequence_node(LINE, $1); }
      ;

decl : '(' type tIDENTIFIER ')'      { $$ = new til::declaration_node(LINE, tPRIVATE, $2,      *$3, nullptr); }
     | '(' type tIDENTIFIER expr ')' { $$ = new til::declaration_node(LINE, tPRIVATE, $2,      *$3, $4); }
     | '(' tVAR tIDENTIFIER expr ')' { $$ = new til::declaration_node(LINE, tPRIVATE, nullptr, *$3, $4); }
     ;

program : '(' tPROGRAM block ')' { $$ = new til::program_node(LINE, $3); }
        ;

function : '(' tFUNCTION '(' type ')' block ')'       { $$ = new til::function_node(LINE, $4, new cdk::sequence_node(LINE), $6); }
         | '(' tFUNCTION '(' type decls ')' block ')' { $$ = new til::function_node(LINE, $4, $5, $7); }
         ;

type : data_type { $$ = $1; }
     | ref_type  { $$ = $1; }
     | func_type { $$ = $1; }
     ;

data_type : tTYPE_INT    { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT); }
          | tTYPE_DOUBLE { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE); }
          | tTYPE_STRING { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING); }
          | tTYPE_VOID   { $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID); }
          ;

ref_type :  type '!'  { $$ = cdk::reference_type::create(4, $1); };
         ;

func_type : '(' type ')'               { $$ = cdk::functional_type::create($2); }
          | '(' type '(' types ')' ')' { $$ = cdk::functional_type::create(*$4, $2); }
          ;

types : type       { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(); $$->push_back($1); }
      | types type { $$ = $1; $$->push_back($2); }
      ;

block : decls instrs  { $$ = new til::block_node(LINE, $1, $2); }
      |       instrs  { $$ = new til::block_node(LINE, new cdk::sequence_node(LINE), $1); }
      | decls         { $$ = new til::block_node(LINE, $1, new cdk::sequence_node(LINE)); }
      | /* empty */   { $$ = new til::block_node(LINE, new cdk::sequence_node(LINE), new cdk::sequence_node(LINE)); }
      ;

instrs : instr         { $$ = new cdk::sequence_node(LINE, $1); }
       | instrs instr  { $$ = new cdk::sequence_node(LINE, $2, $1); }
       ;

instr :  expr                              { $$ = new til::evaluation_node(LINE, $1); }
      | '(' tPRINT exprs ')'               { $$ = new til::print_node(LINE, $3, false); }
      | '(' tPRINTLN exprs ')'             { $$ = new til::print_node(LINE, $3, true); }
      | '(' tSTOP tINTEGER ')'             { $$ = new til::stop_node(LINE, $3); }
      | '(' tSTOP ')'                      { $$ = new til::stop_node(LINE, 0); }
      | '(' tNEXT tINTEGER ')'             { $$ = new til::stop_node(LINE, $3); }
      | '(' tNEXT ')'                      { $$ = new til::stop_node(LINE, 0); }
      | '(' tRETURN expr ')'               { $$ = new til::return_node(LINE, $3); }
      | '(' tRETURN ')'                    { $$ = new til::return_node(LINE, nullptr); }
      | '(' tLOOP expr instr ')'           { $$ = new til::loop_node(LINE, $3, $4); }
      | '(' tIF expr instr ')'             { $$ = new til::if_node(LINE, $3, $4); }
      | '(' tIF expr instr instr ')'       { $$ = new til::if_else_node(LINE, $3, $4, $5); }
      | '(' tBLOCK block ')'               { $$ = $3; }
      ;

expr : '(' '-' expr ')'              { $$ = new cdk::unary_minus_node(LINE, $3); }
     | '(' '+' expr ')'              { $$ = new cdk::unary_plus_node(LINE, $3); }
     | '(' '~' expr ')'              { $$ = new cdk::not_node(LINE, $3); }
     | '(' '+' expr expr ')'         { $$ = new cdk::add_node(LINE, $3, $4); }
     | '(' '-' expr expr ')'         { $$ = new cdk::sub_node(LINE, $3, $4); }
     | '(' '*' expr expr ')'         { $$ = new cdk::mul_node(LINE, $3, $4); }
     | '(' '/' expr expr ')'         { $$ = new cdk::div_node(LINE, $3, $4); }
     | '(' '%' expr expr ')'         { $$ = new cdk::mod_node(LINE, $3, $4); }
     | '(' '<' expr expr ')'         { $$ = new cdk::lt_node(LINE, $3, $4); }
     | '(' '>' expr expr ')'         { $$ = new cdk::gt_node(LINE, $3, $4); }
     | '(' '?' lval ')'              { $$ = new til::address_of_node(LINE, $3); }
     | '(' tGE  expr expr ')'        { $$ = new cdk::ge_node(LINE, $3, $4); }
     | '(' tLE  expr expr ')'        { $$ = new cdk::le_node(LINE, $3, $4); }
     | '(' tNE  expr expr ')'        { $$ = new cdk::ne_node(LINE, $3, $4); }
     | '(' tEQ  expr expr ')'        { $$ = new cdk::eq_node(LINE, $3, $4); }
     | '(' tAND expr expr ')'        { $$ = new cdk::and_node(LINE, $3, $4); }
     | '(' tOR  expr expr ')'        { $$ = new cdk::or_node(LINE, $3, $4); }
     | '(' tSET lval expr ')'        { $$ = new cdk::assignment_node(LINE, $3, $4); }
     | '(' tOBJECTS expr ')'         { $$ = new til::stack_alloc_node(LINE, $3); } 
     | '(' tSIZEOF expr ')'          { $$ = new til::sizeof_node(LINE, $3); }
     | '(' expr exprs ')'            { $$ = new til::function_call_node(LINE, $2, $3); }
     | '(' expr ')'                  { $$ = new til::function_call_node(LINE, $2, new cdk::sequence_node(LINE)); }
     | '(' '@' exprs ')'             { $$ = new til::function_call_node(LINE, nullptr, $3); }
     | '(' '@' ')'                   { $$ = new til::function_call_node(LINE, nullptr, new cdk::sequence_node(LINE)); }
     | '(' tREAD ')'                 { $$ = new til::read_node(LINE); }
     | integer                       { $$ = $1; }
     | double                        { $$ = $1; }
     | string                        { $$ = new cdk::string_node(LINE, $1); }
     | tNULL                         { $$ = new til::nullptr_node(LINE); }
     | function                      { $$ = $1; }
     | lval                          { $$ = new cdk::rvalue_node(LINE, $1); }
     ;

exprs : expr exprs { $$ = new cdk::sequence_node(LINE, $1, $2); }
      | expr       { $$ = new cdk::sequence_node(LINE, $1); }
      ;

lval : tIDENTIFIER              { $$ = new cdk::variable_node(LINE, $1); }
     | '(' tINDEX expr expr ')' { $$ = new til::index_node(LINE, $3, $4); } 
     ;

integer  : tINTEGER       { $$ = new cdk::integer_node(LINE, $1); }

double   : tDOUBLE        { $$ = new cdk::double_node(LINE, $1); }

string   : tSTRING        { $$ = new std::string(*$1); delete $1; }
         ;
 
%%
