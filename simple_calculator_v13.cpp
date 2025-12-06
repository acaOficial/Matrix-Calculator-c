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
    set
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

    deque<Token> buffer; 
    
  public: 
    
    Token_stream() { } 
    Token get(); 
    void unget(Token t) { buffer.push_front(t); } 
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
  cout << "[DEBUG] get(): tokenizing char = '" << ch << "'\n";

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
    	if (isalpha(ch)) 
      {
        string s;
        s+=ch;
        while(cin.get(ch) && (isalpha(ch) || isdigit(ch))) s+=ch;
        cin.unget();

        if(s=="quit") return Token(Token::id::quit);
        if(s=="const") return Token(Token::id::const_token);
        if(s=="help") return Token(Token::id::help_token);
        if(s=="set") return Token(Token::id::set);
        if(s=="precision") return Token(Token::id::precision_token);

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

  Value() :name{}, value{double(0)}, is_const{false} {}

  Value(const string& n, const gv& v, bool is_constant=false) 
    :name(n), value(v), is_const(is_constant) 
  {}
};

// Añadido
struct UserFunction {
  vector<string> args;   // nombres de argumentos
  string body;           // expresión en texto crudo
};

map<string, UserFunction> user_functions;



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

void define_name(const string& s, const gv& d, bool constant=false)
{ names[s]=Value(s,d,constant); }

Token_stream ts;

constexpr int default_precision=6;
int precision=default_precision;

gv expression();

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
gv evaluate_user_function_call(const string& fname, const vector<gv>& args) 
{
    cout << "[DEBUG] evaluate_user_function_call(): fname=" << fname 
         << ", args=" << args.size() << endl;

    auto it = user_functions.find(fname);
    if (it == user_functions.end())
        error("Undefined function: ", fname);

    const UserFunction& fun = it->second;

    if (args.size() != fun.args.size())
        error("Wrong number of arguments in call to ", fname);

    // Guardar entorno actual
    map<string, Value> backup = names;
    
    // Guardar buffer de tokens
    auto saved_buffer = ts.save_buffer();

    // Crear variables locales para los parámetros
    for (size_t i = 0; i < args.size(); i++) {
        cout << "[DEBUG] evaluate_user_function_call(): setting param " << fun.args[i] 
             << " = " << args[i] << endl;
        define_name(fun.args[i], args[i], false);
    }

    // Preparar un stream con el cuerpo y guardar el buffer original de cin
    stringstream ss(fun.body);
    auto* old_buf = cin.rdbuf(ss.rdbuf());

    cout << "[DEBUG] evaluate_user_function_call(): about to evaluate body: '" << fun.body << "'" << endl;
    ts.clear();

    // Evaluar la expresión
    gv result;
    try {
        result = expression();
        cout << "[DEBUG] evaluate_user_function_call(): result = " << result << endl;
    } catch (...) {
        // Asegurar que restauramos cin incluso si hay error
        cin.rdbuf(old_buf);
        ts.restore_buffer(saved_buffer);
        names = backup;
        throw;
    }

    // Restaurar el buffer original de cin
    cin.rdbuf(old_buf);
    
    // Restaurar buffer de tokens (elimina tokens del cuerpo de la función y recupera los originales)
    ts.restore_buffer(saved_buffer);

    // Restaurar entorno
    names = backup;

    return result;
}

gv primary()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  cout << "[DEBUG] primary(): got token kind=" << t.kind << " symbol='" << t.symbol << "' name='" << t.name << "'" << endl;

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
    cout << "[DEBUG] primary(): name_token '" << fname << "', next kind=" << next.kind << " symbol='" << next.symbol << "'" << endl; // Añadido

    // ------------------------------
    // LLAMADA A FUNCIÓN DEFINIDA POR USUARIO
    // ------------------------------
    if (next.is_symbol('(')) {
      cout << "[DEBUG] primary(): detected function call for '" << fname << "'" << endl;
      vector<gv> args;

      Token x = ts.get();
      cout << "[DEBUG] primary(): reading first arg token: kind=" << x.kind << " symbol='" << x.symbol << "' name='" << x.name << "'" << endl;
      
      if (!x.is_symbol(')')) {
        cout << "[DEBUG] primary(): not ')', ungetting and calling expression()" << endl;
        ts.unget(x);
        
        cout << "[DEBUG] primary(): about to call expression() for first arg" << endl;
        args.push_back(expression());
        cout << "[DEBUG] primary(): first arg evaluated, result=" << args[0] << endl;

        while (true) {
          Token comma = ts.get();
          cout << "[DEBUG] primary(): after arg, got token: kind=" << comma.kind << " symbol='" << comma.symbol << "'" << endl;
          
          if (comma.is_symbol(')')) {
            cout << "[DEBUG] primary(): found ')', ending arg list" << endl;
            break;
          }
          
          if (!comma.is_symbol(',')) 
              error("',' expected");
          
          cout << "[DEBUG] primary(): found ',', reading next arg" << endl;
          args.push_back(expression());
          cout << "[DEBUG] primary(): arg evaluated, result=" << args.back() << endl;
        }
      }

      cout << "[DEBUG] primary(): all args read, total=" << args.size() << endl;
      
      auto it = user_functions.find(fname);
      if (it == user_functions.end())
          error("Undefined function: ", fname);

      cout << "[DEBUG] primary(): calling evaluate_user_function_call()" << endl;
      return evaluate_user_function_call(fname, args);
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


// Añadido

bool is_user_function_definition(Token t)
{
  if (t.kind != Token::id::name_token) return false;

  Token next = ts.get();
  if (!next.is_symbol('(')) { 
      ts.unget(next);
      return false; 
  }

  ts.unget(next);
  ts.unget(t);
  return true;
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

  gv v=expression();

  if(is_declared(name)) 
    set_value(name,v);
  else
    define_name(name,v);

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

  gv v=expression();

  define_name(name,v,true);

  return v;
}

// Añadido

void define_user_function()
{
  cout << "[DEBUG] define_user_function() called" << endl; // Añadido
  Token t = ts.get();
  cout << "[DEBUG] define_user_function(): got token kind=" << t.kind << " name='" << t.name << "'" << endl; // Añadido
  string fname = t.name;

  Token lp = ts.get();
  cout << "[DEBUG] define_user_function(): expecting '(', got kind=" << lp.kind << " symbol='" << lp.symbol << "'" << endl; // Añadido
  if (!lp.is_symbol('(')) error("'(' expected in function definition");

  vector<string> params;
  Token p = ts.get();
  cout << "[DEBUG] define_user_function(): checking params, got kind=" << p.kind << " name='" << p.name << "'" << endl; // Añadido

  if (!p.is_symbol(')')) {
    if (p.kind != Token::id::name_token) error("parameter name expected");
    cout << "[DEBUG] define_user_function(): adding param '" << p.name << "'" << endl; // Añadido
    params.push_back(p.name);

    while (true) {
      Token comma = ts.get();
      if (comma.is_symbol(')')) break;
      if (!comma.is_symbol(',')) error("',' expected");

      Token p2 = ts.get();
      if (p2.kind != Token::id::name_token) error("parameter name expected");
      params.push_back(p2.name);
    }
  }

  Token eq = ts.get();
  cout << "[DEBUG] define_user_function(): expecting '=', got kind=" << eq.kind << " symbol='" << eq.symbol << "'" << endl; // Añadido
  if (!eq.is_symbol('=')) error("'=' expected in function definition");

    string body;
    getline(cin, body, ';');
    body += ";";
  cout << "[DEBUG] define_user_function(): body = '" << body << "'" << endl; // Añadido

  user_functions[fname] = UserFunction{params, body};
  cout << "[DEBUG] define_user_function(): function '" << fname << "' defined with " << params.size() << " params" << endl; // Añadido
}


gv statement()
{
  
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();

  cout << "[DEBUG] statement(): token.kind=" << t.kind 
    << " name=" << t.name 
    << " symbol=" << t.symbol << endl;

  switch(t.kind)
  {
    case Token::id::const_token:
      return constant_assign();
      break;

    case Token::id::name_token:
    {
      Token tt = ts.get();
      cout << "[DEBUG] statement(): after name_token '" << t.name << "', got kind=" << tt.kind << " symbol='" << tt.symbol << "'" << endl; // Añadido

      // DETECCIÓN DE DEFINICIÓN DE FUNCIÓN vs LLAMADA
      if (tt.is_symbol('(')) {
        cout << "[DEBUG] statement(): detected '(' after name, checking if definition or call..." << endl;
        
        // Necesitamos ver qué hay después del paréntesis de cierre para distinguir
        // Guardamos tokens temporalmente para inspeccionar
        vector<Token> temp_tokens;
        temp_tokens.push_back(tt); // el '('
        
        cout << "[DEBUG] reading tokens until ')' to check what follows..." << endl;
        
        // Leer hasta encontrar el ')' correspondiente
        int paren_count = 1;
        while (paren_count > 0) {
          Token next = ts.get();
          cout << "[DEBUG] read token: kind=" << next.kind << " symbol='" << next.symbol << "' name='" << next.name << "'" << endl;
          temp_tokens.push_back(next);
          if (next.is_symbol('(')) paren_count++;
          else if (next.is_symbol(')')) paren_count--;
        }
        
        // Ahora miramos qué hay después del ')'
        Token after_paren = ts.get();
        cout << "[DEBUG] statement(): after closing ')', got token: kind=" << after_paren.kind << " symbol='" << after_paren.symbol << "'" << endl;
        
        // Si hay '=' después del ')', es DEFINICIÓN
        if (after_paren.is_symbol('=')) {
          cout << "[DEBUG] statement(): it's a function DEFINITION" << endl;
          temp_tokens.push_back(after_paren); // Incluir el '=' en el buffer
          
          cout << "[DEBUG] Now ungetting all tokens. temp_tokens.size()=" << temp_tokens.size() << endl;
          
          // Con deque y push_front, necesitamos hacer unget en ORDEN INVERSO
          // para que salgan en el orden correcto
          
          // Primero devolver los tokens en orden inverso
          for (auto it = temp_tokens.rbegin(); it != temp_tokens.rend(); ++it) {
            cout << "[DEBUG] ungetting: kind=" << it->kind << " symbol='" << it->symbol << "' name='" << it->name << "'" << endl;
            ts.unget(*it);
          }
          
          // Luego el nombre (para que salga primero)
          cout << "[DEBUG] ungetting name token: '" << t.name << "'" << endl;
          ts.unget(t);
          
          define_user_function();
          return gv(0.0);
        }
        // Si no hay '=', es una LLAMADA (expresión)
        else {
          cout << "[DEBUG] statement(): it's a function CALL (expression)" << endl;
          
          cout << "[DEBUG] Now ungetting all tokens. temp_tokens.size()=" << temp_tokens.size() << endl;
          
          // Con deque y push_front, hacer unget en orden inverso
          
          // Primero el after_paren
          cout << "[DEBUG] ungetting after_paren: kind=" << after_paren.kind << " symbol='" << after_paren.symbol << "'" << endl;
          ts.unget(after_paren);
          
          // Luego los tokens en orden inverso
          for (auto it = temp_tokens.rbegin(); it != temp_tokens.rend(); ++it) {
            cout << "[DEBUG] ungetting: kind=" << it->kind << " symbol='" << it->symbol << "' name='" << it->name << "'" << endl;
            ts.unget(*it);
          }
          
          // Finalmente el nombre (para que salga primero)
          cout << "[DEBUG] ungetting name token: '" << t.name << "'" << endl;
          ts.unget(t);
          
          return expression();
        }
      }

      // ASIGNACIÓN NORMAL
      if (tt.is_symbol('=')) {
        ts.unget(tt);
        ts.unget(t);
        return assign();
      }

      // CUALQUIER OTRA COSA → EXPRESIÓN
      ts.unget(tt);
      ts.unget(t);
      return expression();
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
    <<"\n Mind that all expressions should be finished with a symbol ';'."
    <<"\n For finishing the execution type \"quit\"."
    <<"\n"
    <<"\n Additional commands: "
    <<"\n"
    <<"\n   precision; (shows how many fractional digits are used for showing calculator's results)"
    <<"\n   set precision <numeric_expression>; (changes calculator's precision)"
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
