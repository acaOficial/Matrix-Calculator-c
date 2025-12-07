/*
	simple_calculator_v11.cpp - Simple calculator (elenventh version, proposed exercise 9)

  This program implements a basic expression calculator.
  Input from cin, output from cout.
  The gramman for input is:

  Statement:
    Help
    Constant
    Assign 
    Expression
    Print
    Precision
    SetPrecision
    Quit

  Print:
    ;
    
  Quit:
    quit

  Precision:
    precision

  Precision:
    set precision Number

  Constant:
    const Name = Expression

  Assign:
    Name = Expression

  Expression:
    Term
    Term + Expression
    Term - Expression

  Term:
    Primary
    Primary * Term 
    Primary / Term 
    Primary % Term 

  Primary:
    { List }
    { Columns }
    Function
    Number
    Name
    ( Expression )
    - Primary
    + Primary
    ~ Primary

  Columns:
    { List }
    { List } , Columns

  List:
    Expression 
    Expression , List

  Function:
    FunctionName ( Expression )
    FunctionName ( Expression , Expression )

  FunctionName:
    sin
    cos
    tan
    asin
    acos
    atan
    exp
    pow
    ln
    log10
    log2

  Number:
    floating-point-literal

  Name:
    a string of letters and numbers
  
  Input comes from cin through the Token_stream called ts.
*/

#include <iostream>
#include <string>
#include <stdexcept>
#include <queue>
#include <deque>
#include <cmath>
#include <sstream>
#include <map>
#include <iomanip>
#include <ios>
using namespace std;

#include "generic_value.hpp"
using gv=generic_value<double>;

#define DEBUG_FUNC false

#define PROGRAM_NAME "simple_calculator"
constexpr size_t version=13;

inline void error(const string& s)
{
	throw runtime_error(s);
}

inline void error(const string& s, const string& s2) { error(s+s2); }

inline void error(char c, const string& s2) 
{ ostringstream ostr; ostr<<c<<s2; error(ostr.str()); }

struct Token 
{
  enum id
  {
    none,
    quit,
    print,
    number,
    name_token,
    const_token,
    char_token,
    help_token,
    function_token,
    precision_token,
    set,

    // Añadido
    show_env_token,
    save_env_token,
    load_env_token,
    env_token
  };

  id kind;
  char symbol;
  typename gv::element_t value;
  string name;
  double (*function)(double);

  Token() 
  : kind(id::none), symbol(0), value(0), name(), function(nullptr)
  {}

  Token(id tk) 
  : kind(tk), symbol(0), value(0), name(), function(nullptr)
  {}

  Token(char ch) 
  : kind(id::char_token), symbol(ch), value(0), name(), function(nullptr)
  {}

  Token(typename gv::element_t val)
  : kind(id::number), symbol(0), value(val), name(), function(nullptr)
  {}

  Token(const string& str) 
  : kind(id::name_token), symbol(0), value(0), name(str), function(nullptr) 
  {}

  Token(const string& str, double (*the_function)(double)) 
  : kind(id::function_token), symbol(0), value(0), name(str), function(the_function) 
  {}

  bool is_symbol(char c) { return ((kind==id::char_token) && (symbol==c)); }
  bool is_number(typename gv::element_t v) { return ((kind==id::number) && (value==v)); }
  bool is_name(const string& str) { return ((kind==id::name_token) && (name==str)); }
  bool is_function() { return (kind==id::function_token); }
};

class Token_stream 
{ 
  private:
    //Añadido
    deque<Token> buffer; 
    
  public: 
    
    Token_stream() { } 
    Token get(); 
    void unget(Token t) { buffer.push_front(t); } 

    // Añadido
    void clear() { buffer.clear(); }
    deque<Token> save_buffer() { return buffer; }
    void restore_buffer(const deque<Token>& saved) { buffer = saved; }

    void ignore();
};

Token Token_stream::get()
{
  if(!buffer.empty()) 
  { 
    auto t=buffer.front(); 
    buffer.pop_front(); 
    return t; 
  }

  char ch;
  //cin >> ch;
  do { cin.get(ch); } while(isspace(ch));

  switch (ch) 
  {
    case '(': case ')': 
    case '{': case '}':
    case '+': case '-': 
    case '*': case '/': 
    case '%': case '~': 
    case '=': case ',': 
      return Token(ch);

    case ';':
      return Token(Token::id::print);

    case '.': case '0': case '1': case '2': 
    case '3': case '4': case '5': case '6': 
    case '7': case '8': case '9':
    {	
      cin.unget();

      double val;
      cin>>val;
      if(!cin) { cin.clear(); error("Bad number"); }

      return Token(val);
    }
    default:
    	if (isalpha(ch) || ch == '_') 
      {
        string s;
        s+=ch;
        while(cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s+=ch;
        cin.unget();

        if(s=="quit") return Token(Token::id::quit);
        if(s=="const") return Token(Token::id::const_token);
        if(s=="help") return Token(Token::id::help_token);
        if(s=="set") return Token(Token::id::set);
        if(s=="precision") return Token(Token::id::precision_token);

        // Añadido
        if(s=="show") return Token(Token::id::show_env_token);
        if(s=="save") return Token(Token::id::save_env_token);
        if(s=="load") return Token(Token::id::load_env_token);
        if(s=="env") return Token(Token::id::env_token);

        if(s=="sin") return Token(s,sin);
        if(s=="cos") return Token(s,cos);
        if(s=="tan") return Token(s,tan);
        if(s=="asin") return Token(s,asin);
        if(s=="acos") return Token(s,acos);
        if(s=="atan") return Token(s,atan);
        if(s=="exp") return Token(s,exp);

        if(s=="pow") return Token(s,nullptr); // WARNING: this is the exception, pow has two arguments
                                            //
        if(s=="ln") return Token(s,log);
        if(s=="log10") return Token(s,log10);
        if(s=="log2") return Token(s,log2);

        return Token(s);
    	}
    	error("Bad token");
  }
}

void Token_stream::ignore()
{
  while(!buffer.empty())
  {
    auto t=buffer.front(); buffer.pop_front();
    if(t.kind==Token::id::quit) return;
  }

  char ch;
  while (cin>>ch)
    if (ch==';') return;
}

struct Value 
{
  string name;
  gv value;
  bool is_const;
  string expr;  // Expression string

  Value() :name{}, value{double(0)}, is_const{false}, expr{} {}

  Value(const string& n, const gv& v, bool is_constant=false, const string& expression="") 
    :name(n), value(v), is_const(is_constant), expr(expression) 
  {}
};

// Añadido
struct function {
  vector<string> args;
  vector<Token> body;
};

map<string, function> functions;

// Añadido
class ScopeGuard {
private:
  map<string, Value>& names_ref;
  map<string, Value> backup;
  deque<Token> saved_buffer;
  Token_stream& ts_ref;
  
public:
  ScopeGuard(map<string, Value>& names, Token_stream& ts)
    : names_ref(names), backup(names), ts_ref(ts)
  {
    saved_buffer = ts_ref.save_buffer();
    ts_ref.clear();
  }
  
  ~ScopeGuard() {
    ts_ref.restore_buffer(saved_buffer);
    names_ref = backup;
  }
};

map<string,Value> names;

gv get_value(const string& s)
{
  auto iter=names.find(s);
  if(iter!=names.end()) return iter->second.value;

  error("get: undefined name ",s);
}

void set_value(const string& s,const gv& v)
{
  auto iter=names.find(s);
  if(iter!=names.end())
  {
    if(iter->second.is_const) error("set: const name ",s);
    iter->second.value=v;
    return;
  }

  error("set: undefined name ",s);
}

bool is_constant(const string& s)
{
  auto iter=names.find(s);
  return (
    (iter!=names.end()) && 
    (iter->second.is_const)
  ); 
}

bool is_declared(const string& s) { return (names.find(s)!=names.end()); }

void define_name(const string& s, const gv& d, bool constant=false, const string& expr="")
{ names[s]=Value(s,d,constant,expr); }

Token_stream ts;

constexpr int default_precision=6;
int precision=default_precision;

gv expression();

// Añadido
void show_env();
void save_env_to_file(const string& fname);
void load_env_from_file(const string& fname);
string capture_expression_string();
string tokens_to_string(const vector<Token>& tokens);


string read_filename()
{
  string filename;
  char ch;
  
  while (cin.get(ch) && isspace(ch));
  
  if (!cin) error("filename expected");
  
  do {
    filename += ch;
  } while (cin.get(ch) && ch != ';' && !isspace(ch));
  
  if (ch == ';') cin.unget();
  
  return filename;
}

// Añadido
string token_to_string(const Token& t) {
  ostringstream oss;
  
  switch(t.kind) {
    case Token::id::number:
      oss << t.value;
      break;
    case Token::id::name_token:
      oss << t.name;
      break;
    case Token::id::char_token:
      oss << t.symbol;
      break;
    case Token::id::function_token:
      oss << t.name;
      break;
    case Token::id::print:
      break;
    default:
      break;
  }
  
  return oss.str();
}

// Añadido
string tokens_to_string(const vector<Token>& tokens) {
  ostringstream oss;
  for (size_t i = 0; i < tokens.size(); i++) {
    if (tokens[i].kind == Token::id::print) break;
    string s = token_to_string(tokens[i]);
    if (!s.empty()) {
      oss << s;
      if (i + 1 < tokens.size() && tokens[i+1].kind != Token::id::char_token) {
        if (tokens[i].kind == Token::id::name_token || 
            tokens[i].kind == Token::id::number) {
          oss << " ";
        }
      }
    }
  }
  return oss.str();
}

// Añadido
string capture_expression_string() {
  vector<Token> expr_tokens;
  Token t;
  int paren_depth = 0;
  int brace_depth = 0;
  
  while (true) {
    t = ts.get();
    
    if (t.is_symbol('(')) paren_depth++;
    else if (t.is_symbol(')')) paren_depth--;
    else if (t.is_symbol('{')) brace_depth++;
    else if (t.is_symbol('}')) brace_depth--;
    
    if (t.kind == Token::id::print && paren_depth == 0 && brace_depth == 0) {
      ts.unget(t);
      break;
    }
    
    expr_tokens.push_back(t);
  }
  
  for (auto it = expr_tokens.rbegin(); it != expr_tokens.rend(); ++it) {
    ts.unget(*it);
  }
  
  return tokens_to_string(expr_tokens);
}

// Añadido
vector<gv> parse_arguments() {
  vector<gv> args;
  
  Token t = ts.get();
  if (!t.is_symbol(')')) {
    ts.unget(t);
    args.push_back(expression());
    
    while (true) {
      Token comma = ts.get();
      if (comma.is_symbol(')')) break;
      if (!comma.is_symbol(',')) error("',' expected in argument list");
      args.push_back(expression());
    }
  }
  
  return args;
}

// Añadido
vector<string> parse_parameters() {
  vector<string> params;
  
  Token t = ts.get();
  if (!t.is_symbol(')')) {
    if (t.kind != Token::id::name_token) error("parameter name expected");
    params.push_back(t.name);
    
    while (true) {
      Token comma = ts.get();
      if (comma.is_symbol(')')) break;
      if (!comma.is_symbol(',')) error("',' expected in parameter list");
      
      Token p = ts.get();
      if (p.kind != Token::id::name_token) error("parameter name expected");
      params.push_back(p.name);
    }
  }
  
  return params;
}

gv function_name()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(!t.is_function()) error("function name expected");

  Token tt=ts.get();
  if(!tt.is_symbol('(')) error("'(' expected");
  gv v=expression();
  tt=ts.get();
  if(tt.is_symbol(')')) 
  {
    if(t.function) return v.call_function(t.function);
    else error(t.name," needs two arguments");
  }
  else if(!tt.is_symbol(',')) error("')' expected");
  {
    gv vv=expression();
    tt=ts.get();
    if(tt.is_symbol(')')) 
    {
      if(t.name=="pow") return v.call_function(pow,vv); 
      else error(t.name," needs only one argument");
    }
    else error("')' expected");
  }
}

vector<typename gv::matrix_t::value_t::element_t> list()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  vector<typename gv::matrix_t::value_t::element_t> row; 
  Token t;
  do
  {
    auto v=expression();
    row.push_back(v.get<typename gv::scalar_t>());
    t=ts.get();
  } while(t.is_symbol(','));
  ts.unget(t);
  return row;
}

gv columns()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
           
  vector<
    vector<typename gv::matrix_t::value_t::element_t> 
  > rows;

  Token t,tt;
  do
  {
    t=ts.get();
    if(!t.is_symbol('{')) error("'{' expected");
    
    tt=ts.get();
    if(tt.is_symbol('}')) 
      rows.push_back(vector<typename gv::matrix_t::value_t::element_t>()); 
    else 
    {
      ts.unget(tt);

      rows.push_back(list());
      t=ts.get();
      if(!t.is_symbol('}')) error("'}' expected");
    }
    t=ts.get();
  } while(t.is_symbol(','));
  ts.unget(t);

  return gv(typename gv::matrix_t::value_t(rows));
}

// Añadido
gv evaluate_function(const string& fname, const vector<gv>& args) 
{
  auto it = functions.find(fname);
  if (it == functions.end())
    error("Undefined function: ", fname);

  const function& fun = it->second;

  if (args.size() != fun.args.size())
    error("Wrong number of arguments in call to ", fname);

  ScopeGuard guard(names, ts);

  for (size_t i = 0; i < args.size(); i++) {
    define_name(fun.args[i], args[i], false);
  }

  for (auto it = fun.body.rbegin(); it != fun.body.rend(); ++it) {
    ts.unget(*it);
  }

  return expression();
}

gv primary()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();

  if(t.is_function()) { ts.unget(t); return function_name(); }
  else if(t.kind==Token::id::char_token)
  {
    if(t.is_symbol('('))
    {
      gv v=expression();
      t=ts.get();
      if(!t.is_symbol(')')) error("')' expected");
      return v;
    }
    else if(t.is_symbol('{'))
    {
      Token tt=ts.get();
      if(tt.is_symbol('{')) 
      { 
        ts.unget(tt); 
        gv v=columns();
        t=ts.get();
        if(!t.is_symbol('}')) error("'}' expected");  
        return v; 
      }
      else if(tt.is_symbol('}')) return gv{typename gv::matrix_t::value_t()};  
      { 
        ts.unget(tt);
        gv v{typename gv::matrix_t::value_t(list())};
        t=ts.get();
        if(!t.is_symbol('}')) error("'}' expected");  
        return v;
      }
    }
    else if(t.is_symbol('-')) return -primary();
    else if(t.is_symbol('+')) return primary();
    else if(t.is_symbol('~')) return ~primary();
  }
  else if(t.kind==Token::id::number) return gv(t.value);

  // Añadido
  else if (t.kind == Token::id::name_token) {
    string fname = t.name;
    Token next = ts.get();

    if (next.is_symbol('(')) {
      vector<gv> args = parse_arguments();
      
      if (functions.find(fname) == functions.end())
        error("Undefined function: ", fname);

      return evaluate_function(fname, args);
    }

    ts.unget(next);
    return get_value(fname);
  }


  error("primary expected");
}

gv term()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  gv left = primary();
  while(true) 
  {
    Token t = ts.get();

    if(t.is_symbol('*')) left=left*primary();
    else if(t.is_symbol('/')) left=left/primary();
    else if(t.is_symbol('%')) left=left%primary();
    else { ts.unget(t); return left; }
  }
}

gv expression()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  gv left = term();
  while(true) 
  {
    Token t = ts.get();

    if(t.is_symbol('+')) left=left+term();
    else if(t.is_symbol('-')) left=left-term();
    else { ts.unget(t); return left; }
  }
}


gv assign()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(t.kind!=Token::id::name_token) error ("name expected in assign");
  string name = t.name;
  if (is_constant(name)) error(name," constant cannot be modified"); 
  t=ts.get();

  if(!t.is_symbol('=')) error("= missing in assign of " ,name);

  string expr_str = capture_expression_string();
  gv v=expression();

  if(is_declared(name)) 
    set_value(name,v);
  else
    define_name(name,v,false,expr_str);

  return v;
}

gv constant_assign()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(t.kind!=Token::id::name_token) error("name expected in const assign");
  string name = t.name;
  if(is_declared(name)) error(name," has already been defined"); 
  t=ts.get();

  if(!t.is_symbol('=')) error("= missing in assign of " ,name);

  string expr_str = capture_expression_string();
  gv v=expression();

  define_name(name,v,true,expr_str);

  return v;
}

// Añadido
void define_function()
{
  Token t = ts.get();
  if (t.kind != Token::id::name_token) error("function name expected");
  string fname = t.name;

  Token lp = ts.get();
  if (!lp.is_symbol('(')) error("'(' expected in function definition");

  vector<string> params = parse_parameters();

  Token eq = ts.get();
  if (!eq.is_symbol('=')) error("'=' expected in function definition");

  // Tokenizar el cuerpo de la función
  vector<Token> body_tokens;
  Token tok = ts.get();
  
  while (tok.kind != Token::id::print) {
    body_tokens.push_back(tok);
    tok = ts.get();
  }
  
  body_tokens.push_back(tok);

  functions[fname] = function{params, body_tokens};
}


gv statement()
{
  
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  switch(t.kind)
  {
    case Token::id::const_token:
      return constant_assign();
      break;

    case Token::id::name_token:
    {
      Token name = t;
      Token next = ts.get();

      // definición de función
      if (next.is_symbol('(')) {
        vector<Token> lookahead;
        lookahead.push_back(next);
        
        int level = 1;
        while (level > 0) {
          Token x = ts.get();
          lookahead.push_back(x);
          if (x.is_symbol('(')) level++;
          else if (x.is_symbol(')')) level--;
        }
        
        Token after = ts.get();
        
        if (after.is_symbol('=')) {
          ts.unget(after);
          for (auto it = lookahead.rbegin(); it != lookahead.rend(); ++it) 
            ts.unget(*it);
          ts.unget(name);
          define_function();
          return gv(0.0);
        }
        
        ts.unget(after);
        for (auto it = lookahead.rbegin(); it != lookahead.rend(); ++it) 
          ts.unget(*it);
        ts.unget(name);
        return expression();
      }

      // Cuando asigno
      if (next.is_symbol('=')) {
        ts.unget(next);
        ts.unget(name);
        return assign();
      }

      //  Cuando hay una expresión simple
      ts.unget(next);
      ts.unget(name);
      return expression();
    }
    break;

    // Añadido
    case Token::id::show_env_token:
    {
      Token next = ts.get();
      if (next.kind != Token::id::env_token)
        error("expected 'env' after show");

      show_env();
      return gv(0.0);
    }
    break;

    case Token::id::save_env_token:
    {
      Token next = ts.get();
      if (next.kind != Token::id::env_token)
        error("expected 'env' after save");

      string filename = read_filename();
      if (filename.empty())
        error("file name expected");

      save_env_to_file(filename);
      return gv(0.0);
    }
    break;

    case Token::id::load_env_token:
    {
      Token next = ts.get();
      if (next.kind != Token::id::env_token)
        error("expected 'env' after load");

      string filename = read_filename();
      if (filename.empty())
        error("file name expected");

      load_env_from_file(filename);
      return gv(0.0);
    }
    break;

    default:
    { ts.unget(t); return expression(); }
  }
}

void clean_up_mess()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
	ts.ignore();
}

void help()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
           
  cout
    <<" "<<PROGRAM_NAME<<" - v"<<version<<" - MUSIANI - Programación y Prototipado"
    <<"\n"
    <<"\n This  is  a simple  calculator  which  accepts  aritmetic"
    <<"\n expressions  with  numeric  literals and simbolic  values"
    <<"\n like constants and variables."
    <<"\n"
    <<"\n Allows also the use of the following fundamental functions:"
    <<"\n"
    <<"\n   sin, cos, tan, asin, acos, atan"
    <<"\n   exp, pow, ln, log10, log2"
    <<"\n"
    <<"\nIt is a matricial calculator, and accepts matrix literals with the following"
    <<"\nnotation (spaces, line feeds and carry returns are skipped):"
    <<"\n"
    <<"\n   {{1,2},{-2,-1}};"
    <<"\n   or"
    <<"\n   {"
    <<"\n     { 122.000000, 244.000000, 366.000000 },"
    <<"\n     { 244.000000, 488.000000, 732.000000 },"
    <<"\n     { 366.000000, 732.000000, 1098.000000 }"
    <<"\n   }"
    <<"\n   or"
    <<"\n   { 0.540302, 0.004426, 0.004426, -0.999961, -0.013277 }"
    <<"\n"
    <<"\n Notation is very simple, as shown in the following examples:"
    <<"\n "
    <<"\n   2+3*7-(8-3.2)-1/3; (aritmetic expression)"
    <<"\n   a=3; (assign of a value to a variable)"
    <<"\n   const pi=3.141592; (assign of a value to a constant)"
    <<"\n   2*pi-a; (arithmetic expression with variables and constants)"
    <<"\n   sin(2*pi/4); (arithmetic expression with functions)"
    <<"\n   const e=exp(1); (arithmetic expression with functions)" 
    <<"\n   ln(2*e/pi); (arithmetic expression with functions)"
    <<"\n   pow(3,2); (arithmetic expression with functions)"
    <<"\n   v={1,1,2,3,5,8,13,21,34} (assign a line vector to a varible)"
    <<"\n   exp(v) (arithmetic expression with functions and a line vector)"
    <<"\n   a={"
    <<"\n     { 122.000000, 244.000000, 366.000000 },"
    <<"\n     { 244.000000, 488.000000, 732.000000 },"
    <<"\n     { 366.000000, 732.000000, 1098.000000 }"
    <<"\n   } (a literal matrix assigned to a varible)"<<"\n"
    <<"\n"
    <<"\n Matrix operations:"
    <<"\n"
    <<"\n   ~a; (calculates the inverse of matrix a)"
    <<"\n   ~{{1,2},{3,4}}; (inverse of a matrix literal)"
    <<"\n"
    <<"\n User-defined functions:"
    <<"\n"
    <<"\n   f(x) = x^2 + 2*x + 1; (define a function with one parameter)"
    <<"\n   g(x, y) = x*y + sin(x); (define a function with multiple parameters)"
    <<"\n   f(5); (call a user-defined function)"
    <<"\n"
    <<"\n Mind that all expressions should be finished with a symbol ';'."
    <<"\n For finishing the execution type \"quit\"."
    <<"\n"
    <<"\n Additional commands: "
    <<"\n"
    <<"\n   precision; (shows how many fractional digits are used for showing calculator's results)"
    <<"\n   set precision <numeric_expression>; (changes calculator's precision)"
    <<"\n   show env; (displays all variables and user-defined functions with their expressions)"
    <<"\n   save env <filename>; (saves the current environment to a file)"
    <<"\n   load env <filename>; (loads an environment from a file)"
    <<"\n"
  ; 
}

void precision_statement()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
           
  cout
    <<" precision digits: "<<precision<<"\n"
  ;
}

void set_precision()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(t.kind!=Token::id::precision_token) error("precision keyword expected");

  gv d=expression();
  precision=d.get<gv::scalar_t>();

  cout
    <<" precision set to "<<precision<<" digits\n"
  ;
}


// Añadido
void show_env() 
{
  cout << "Environment variables:\n";
  for (const auto& kv : names) {
    cout << "  " << kv.first;
    if (!kv.second.expr.empty()) {
      cout << " = " << kv.second.expr;
    }
    cout << "\n" << kv.second.value;
    if (kv.second.is_const) cout << " (const)";
    cout << "\n";
  }
  
  if (!functions.empty()) {
    cout << "\nUser-defined functions:\n";
    for (const auto& kv : functions) {
      cout << "  " << kv.first << "(";
      for (size_t i = 0; i < kv.second.args.size(); i++) {
        if (i > 0) cout << ", ";
        cout << kv.second.args[i];
      }
      cout << ")";
      if (!kv.second.body.empty()) {
        cout << " = " << tokens_to_string(kv.second.body);
      }
      cout << "\n";
    }
  }
}


// Añadido
void save_env_to_file(const string& fname)
{
  ofstream out(fname);
  if (!out) error("Cannot open file for saving: ", fname);

  out << "VARS " << names.size() << "\n";
  
  for (const auto& kv : names) {
    out << "VAR " << kv.first << " " << kv.second.is_const << "\n";
    
    gv temp_value = kv.second.value;
    
    try {
      double scalar = temp_value.get<gv::scalar_t>();
      out << "SCALAR " << scalar << "\n";
    }
    catch (...) {
      try {
        auto matrix = temp_value.get<gv::matrix_t>();
        out << "MATRIX " << matrix.rows() << " " << matrix.columns() << "\n";
        
        for (size_t i = 0; i < matrix.rows(); i++) {
          for (size_t j = 0; j < matrix.columns(); j++) {
            out << matrix[i][j];
            if (j < matrix.columns() - 1) out << " ";
          }
          out << "\n";
        }
      }
      catch (...) {
        error("Cannot serialize value for: ", kv.first);
      }
    }
  }
  
  out << "FUNCS " << functions.size() << "\n";
  
  for (const auto& kv : functions) {
    out << "FNAME " << kv.first << "\n";
    out << "ARGS " << kv.second.args.size();
    for (const auto& arg : kv.second.args) {
      out << " " << arg;
    }
    out << "\n";
    
    out << "BODY " << kv.second.body.size() << "\n";
    for (const auto& token : kv.second.body) {
      out << static_cast<int>(token.kind) << " ";
      
      switch (token.kind) {
        case Token::id::char_token:
          out << token.symbol;
          break;
        case Token::id::number:
          out << token.value;
          break;
        case Token::id::name_token:
        case Token::id::function_token:
          out << token.name;
          break;
        default:
          out << "0";
          break;
      }
      out << "\n";
    }
  }
}

// Añadido
void load_env_from_file(const string& fname)
{
  ifstream in(fname);
  if (!in) error("Cannot open file for loading: ", fname);

  string keyword;
  
  // Cargar variables
  in >> keyword;
  if (keyword != "VARS") error("Invalid file format: expected VARS");
  
  size_t num_vars;
  in >> num_vars;
  
  for (size_t i = 0; i < num_vars; i++) {
    in >> keyword;
    if (keyword != "VAR") error("Invalid file format: expected VAR");
    
    string var_name;
    int is_const;
    in >> var_name >> is_const;
    
    in >> keyword;
    if (keyword == "SCALAR") {
      double val;
      in >> val;
      define_name(var_name, gv(val), is_const != 0);
    }
    else if (keyword == "MATRIX") {
      size_t rows, cols;
      in >> rows >> cols;
      
      vector<vector<double>> matrix_data(rows, vector<double>(cols));
      for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < cols; c++) {
          in >> matrix_data[r][c];
        }
      }
      
      define_name(var_name, gv(typename gv::matrix_t::value_t(matrix_data)), is_const != 0);
    }
    else {
      error("Invalid file format: expected SCALAR or MATRIX");
    }
  }
  
  // Cargar funciones
  in >> keyword;
  if (keyword != "FUNCS") error("Invalid file format: expected FUNCS");
  
  size_t num_funcs;
  in >> num_funcs;
  
  for (size_t i = 0; i < num_funcs; i++) {
    in >> keyword;
    if (keyword != "FNAME") error("Invalid file format: expected FNAME");
    
    string func_name;
    in >> func_name;
    
    in >> keyword;
    if (keyword != "ARGS") error("Invalid file format: expected ARGS");
    
    size_t num_args;
    in >> num_args;
    
    vector<string> args;
    for (size_t j = 0; j < num_args; j++) {
      string arg;
      in >> arg;
      args.push_back(arg);
    }
    
    in >> keyword;
    if (keyword != "BODY") error("Invalid file format: expected BODY");
    
    size_t num_tokens;
    in >> num_tokens;
    
    vector<Token> body;
    for (size_t j = 0; j < num_tokens; j++) {
      int kind;
      in >> kind;
      
      Token tok;
      tok.kind = static_cast<Token::id>(kind);
      
      switch (tok.kind) {
        case Token::id::char_token:
        {
          char sym;
          in >> sym;
          tok.symbol = sym;
          break;
        }
        case Token::id::number:
        {
          double val;
          in >> val;
          tok.value = val;
          break;
        }
        case Token::id::name_token:
        case Token::id::function_token:
        {
          string name;
          in >> name;
          tok.name = name;
          break;
        }
        default:
        {
          string dummy;
          in >> dummy;
          break;
        }
      }
      
      body.push_back(tok);
    }
    
    functions[func_name] = function{args, body};
  }
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  while(true) 
  try 
  {
    cout<<prompt;
    Token t=ts.get();
    while (t.kind==Token::id::print) t=ts.get();
    if(t.kind==Token::id::quit) return;
    if(t.kind==Token::help_token) { help(); continue; }
    if(t.kind==Token::precision_token) { precision_statement(); continue; }
    if(t.kind==Token::set) { set_precision(); continue; }

    ts.unget(t);
    auto the_result=statement();
    cout<<fixed<<setprecision(precision)<<result<<the_result<<endl;
  }
  catch(runtime_error& e) 
  {
    cerr<<e.what()<< endl;
    clean_up_mess();
  }
  catch(logic_error& e) 
  {
    cerr<<e.what()<< endl;
    clean_up_mess();
  }
}

int main()
try 
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
  help();

  calculate();
  return 0;
}
catch (exception& e) {
  cerr<<"exception: "<<e.what()<<endl;
  char c;
  while((cin>>c) && (c!=';')) ;
  return 1;
}
catch (...) {
  cerr << "exception\n";
  char c;
  while((cin>>c) && (c!=';')) ;
  return 2;
}
