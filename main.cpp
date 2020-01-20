#include "Scanner.cpp"
#include <queue>
#include <stack>
#include <cassert>

using namespace std;

class Calculator {

private:
  
  Scanner m_scanner;
  stack<Token> m_stack;
  queue<Token> m_queue;
  double result;

public: 
  Calculator(): m_stack(), m_queue() {}
  Calculator(Token t);

  int  getPrecedence(Token t);
  void inToPost();
  void evalPostfix();
  bool isNumber(Token t);
  bool isOperator(Token t);
  bool isLeftParen(Token t);
  bool isRightParen(Token t);
  void handleOperator(Token t);
  void handleParens();
  void unloadStack();
  void printQueue();
  void readPostfix();
  double evalExpression(double t1, double t2, Token op);
};

int Calculator::getPrecedence(Token t) {

  switch(*t.c_str()) {
  case '+' : case '-':
    return 2;
    break;
  case '*': case '/':
    return 3;
    break;
  case '^':
    return 4;
    break;
  default:
    return 0;
  }


}

bool Calculator::isNumber(Token t) {
  return (t.type() == Token::NUMBER);
}

bool Calculator::isOperator(Token t) {
  return (t.type() == Token::SYMBOL && *t.c_str() != ')' && *t.c_str() != '(');
}

bool Calculator::isLeftParen(Token t) {
  return (t.type() == Token::SYMBOL && *t.c_str() == '(');
}

bool Calculator::isRightParen(Token t) {
  return (t.type() == Token::SYMBOL && *t.c_str() == ')');
}

void Calculator::handleOperator(Token t) {
  while(!m_stack.empty() && isOperator(m_stack.top()) && getPrecedence(m_stack.top()) >= getPrecedence(t))
    {
      Token stack_top = m_stack.top();
      m_queue.push(stack_top);
      m_stack.pop();
    }
  m_stack.push(t);
}

void Calculator::handleParens() {
  bool left_paren = false;
  while(!m_stack.empty() && !left_paren ) {
    Token top = m_stack.top();
    if (*top.c_str() == '(') {
      left_paren = true;
      m_stack.pop();     //discard the parentheses       
    }      
    else {
      m_queue.push(top);
      m_stack.pop();	
    }
  }
}

void Calculator::unloadStack(){
  while( !(m_stack.empty())) {
    Token t = m_stack.top();
    m_queue.push(t);
    m_stack.pop();
  }
}

void Calculator::inToPost() {
  Token t = m_scanner.next();
  t = m_scanner.next(); // hack for now -- need this so that token type isn't END_OF_LINE
  
  while(t.type() != Token::END_OF_LINE) {
    if (isNumber(t))
      m_queue.push(t);

    if(isOperator(t)) 
      handleOperator(t);
    
    if (isLeftParen(t)) 
      m_stack.push(t);
      
    if (isRightParen(t)) 
      handleParens();

    t = m_scanner.next();
  }  
  
  unloadStack();
  
}
/*Need to distinguish between SYMBOLS and NUMBERS because we can't print c_strings of NUMBERS */
void Calculator::printQueue() {
  while(!m_queue.empty()) {
    Token t = m_queue.front();
    if(t.type() == Token::SYMBOL) {
      cout << *t.c_str();
    }
    else {
      cout << t.number();
    }
    m_queue.pop();
    if(m_queue.empty()) cout << '\n';
  }

}

double Calculator::evalExpression(double t1, double t2, Token op) {
  switch(*op.c_str()) {
  case '+':
    return t1 + t2;
    break;

  case '-':
    return t1 - t2;
    break;

  case '*':
    return t1 * t2;
    break;

  case '/':
    return t1 / t2;
    break;

  default:
    return 0;
    break;
  }
}

void Calculator::readPostfix() {
  Token t = m_scanner.next();
  t = m_scanner.next();
  while(t.type() != Token::END_OF_LINE) {
    m_queue.push(t);
    t = m_scanner.next();
  }
}

void Calculator::evalPostfix() {
  if(m_queue.empty()) 
    readPostfix();
	
  stack<double> evalStack;
  while(!m_queue.empty()) {
    Token t = m_queue.front();
    if(isNumber(t)) {
      evalStack.push(t.number());      
    }
    else if(isOperator(t)) {
      double operand1 = evalStack.top();
      evalStack.pop();
      double operand2 = evalStack.top();
      evalStack.pop();
      result = evalExpression(operand2, operand1, t);
      evalStack.push(result);
    }
    m_queue.pop();

  }
  while(!evalStack.empty()) {
    cout << "Result: " << evalStack.top() << '\n';
    evalStack.pop();
  }
}

void promptUser() {
  
  std::cout << "infix, postfix, or quit?" << std::endl;
  string answer;
  std::cin >> answer; 
  
 
  if (answer == "infix") {
    Calculator myCalc;
    myCalc.inToPost();
    myCalc.evalPostfix();
    promptUser();
  }

  else if (answer == "postfix") {
    Calculator myCalc;
    myCalc.evalPostfix();
    promptUser();
  }

  else if (answer == "quit" ) 
    return;
    

}

  
int main() {
  
  promptUser();
 
  /*  

    I think this works. Need to clean up evalPostFix, make comments, catch errors, and try harder tests
    
    Still some issues with subtraction vs negation based on how expressions are inputted.

   */
  
    
  return 0;

}
