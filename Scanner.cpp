// CS136 support code
// By Morgan McGuire morgan@cs.williams.edu and Sam Donow
// Williams College Computer Science Department, 2014
// Released into the Public Domain

#include "Scanner.h"
#include <cassert>
#include <stdlib.h>
#include <iostream>

Token::Token(Type t, const SourceLocation& location) :
  m_type(t), m_number(0), m_boolean(false), m_location(location) {
  assert((t == END_OF_FILE) || (t == END_OF_LINE));
}


Token::Token(Type t, double n, const SourceLocation& location) :
  m_type(t), m_string(std::to_string(n)), m_number(n), m_boolean(false), m_location(location) {
  assert(t == NUMBER);
}


Token::Token(Type t, bool b, const SourceLocation& location) : 
  m_type(t), m_string(b ? "true" : "false"), m_number(0), m_boolean(b), m_location(location) {
  assert(t == BOOLEAN);
}


Token::Token(Type t, const std::string& s, const SourceLocation& location) :
  m_type(t), m_string(s), m_number(0), m_boolean(false), m_location(location) {
  assert((t == STRING) || (t == SYMBOL));
}


Token::Token(Type t, const char* s, const SourceLocation& location) :
  m_type(t), m_string(s), m_number(0), m_boolean(false), m_location(location) {
  assert(s != NULL);
  assert((t == STRING) || (t == SYMBOL));
}
    

Token::Type Token::type() const {
  return m_type;
}

 
const std::string& Token::string() const {
  assert(type() == STRING);
  return m_string;
}


const std::string& Token::symbol() const {
  assert(type() == SYMBOL);
  return m_string;
}


double Token::number() const {
  assert(type() == NUMBER);
  return m_number;
}


bool Token::boolean() const {
  assert(type() == BOOLEAN);
  return m_boolean;
}


const char* Token::c_str() const {
  assert(!(type() == END_OF_FILE || type() == END_OF_LINE));
  return m_string.c_str();
}


const SourceLocation& Token::sourceLocation() const {
  return m_location;
}

///////////////////////////////////////////////////////////////////////////////////////////////

static std::string readline(FILE* file) {
  char* line = NULL;
  size_t length = 0;
    
  // http://linuxmanpages.com/man3/getline.3.php
  const ssize_t numChars = getline(&line, &length, file);
  (void)numChars;
    
  const std::string& s = line;

  free(line);
  line = NULL;

  return s;
}

/** Space, Tab, Newline, or EOF */
static bool isWhitespace(const char c) {
  return isspace(c) || c == EOF;
}

/** \n, \r, and \r\n all parsed as a single newline */
static bool isNewline(const char c) {
  return c == '\n' || c == '\r';
}


static bool isDigitOrDot(const char c) {
  return isdigit(c) || c == '.';
}

//////////////////////////////////////////////////////////////////////////////////////////////

Scanner::Scanner(FILE* source, int startLineNumber) :
  m_source(source), m_currentLine(startLineNumber - 1), m_nextTokenIndex(0) {
  assert(source != NULL);
  readAhead();
}


Scanner::Scanner(const std::string& s) :
  m_source(NULL), m_string(s), m_currentLine(0), m_nextTokenIndex(0) {
  readAhead();
}


Token Scanner::next() {
  if (m_nextTokenIndex == m_token.size()) {
    m_token.clear();
    m_nextTokenIndex = 0;
    readAhead();
  }

  return m_token[m_nextTokenIndex++];
}


/** Reads the next token from input*/
Token Scanner::read(const std::string& currentLine, size_t& currentPosition) const {

  char c = currentLine[currentPosition];

  while (isWhitespace(c)) {
    if (isNewline(c)) { 
      if ((c == '\r') && (currentPosition + 1 < currentLine.length()) &&
	  currentLine[currentPosition + 1] == '\n') {
	++currentPosition;
      }
      return Token(Token::END_OF_LINE, SourceLocation("<stdin>", m_currentLine, currentPosition++));
    } else if (c == EOF) {
      return Token(Token::END_OF_FILE, SourceLocation("<stdin>", m_currentLine, currentPosition++));
    } else {
      c = currentLine[++currentPosition];
    }
  }

  const bool isNegativeNumber = ((c == '-') && isdigit(currentLine[currentPosition + 1]));

  if (isdigit(c) || isNegativeNumber) {
    size_t endNumPosition = currentPosition + (int) isNegativeNumber;
    bool hasExponent = false;

    do {
      //check for exponential notation
      if (! hasExponent && (toupper(currentLine[endNumPosition + 1]) == 'E')) {
	if ((endNumPosition + 2 < currentLine.length()) &&
	    isdigit(currentLine[endNumPosition + 2])) {
	  endNumPosition += 2;
	  hasExponent = true;
	} else if ((endNumPosition + 3 < currentLine.length()) &&
		   (currentLine[endNumPosition + 2] == '-') &&
		   isdigit(currentLine[endNumPosition + 3])) {
	  endNumPosition += 3;
	  hasExponent = true;
	}
      }
    } while (isDigitOrDot(currentLine[++endNumPosition]));
                      
    double d = 0;
    sscanf(currentLine.substr(currentPosition, endNumPosition - currentPosition).c_str(), "%lf", &d);
    currentPosition = endNumPosition;
    return Token(Token::NUMBER, d, SourceLocation("<stdin>", m_currentLine, currentPosition));
        
  } else if (currentLine.substr(currentPosition, 4) == "true") {

    Token t(Token::BOOLEAN, true, SourceLocation("<stdin>", m_currentLine, currentPosition));
    currentPosition += 4;
    return t;

  } else if (currentLine.substr(currentPosition, 5) == "false") {

    Token t(Token::BOOLEAN, false, SourceLocation("<stdin>", m_currentLine, currentPosition));
    currentPosition += 5;
    return t;

  } else if (c == '"') { 

    // quoted string
    size_t endquotePos = currentLine.find('"', currentPosition + 1);
    assert (endquotePos != std::string::npos);
    Token t(Token::STRING, currentLine.substr(currentPosition, endquotePos - currentPosition), SourceLocation("<stdin>", m_currentLine, currentPosition));
    return t;

  } else if (isalpha(c)) { 

    // word symbol
    const int startingPosition = currentPosition;

    for (char c = currentLine[currentPosition]; isalpha(c) || (c == '_'); c = currentLine[++currentPosition]) {}

    const std::string& sym = currentLine.substr(startingPosition, currentPosition - startingPosition);
    return Token(Token::SYMBOL, sym, SourceLocation("<stdin>", m_currentLine, currentPosition++));

  }  else { 

  // other symbol
  size_t symbolLength = 1;
  switch(c) {
  case '+':
  case '-':
  case '*':
  case '/':
  case '^':
  case '%':
  case '(':
  case ')':
    // length 1
    break;

  case '&':
  case '|':
  case '=':
    symbolLength = (currentLine[currentPosition + 1] == c) ? 1 : 0;
  break;

  case '>':
  case '<':
    symbolLength = (currentLine[currentPosition + 1] == '=') ? 2 : 1;
  break;

  default:
    symbolLength = 0;
  }

  assert(symbolLength != 0);
  const std::string& sym = currentLine.substr(currentPosition, symbolLength);
  currentPosition += symbolLength;
  return Token(Token::SYMBOL, sym, SourceLocation("<stdin>", m_currentLine, currentPosition));
 }
}


void Scanner::readAhead() {
  const std::string& src = (m_source != NULL) ? readline(m_source) : m_string;

  if (m_source == NULL) {
    // Consume the string
    m_string = "";
  }

  if (src.length() == 0) {
    // handle line with only EOF
    m_token.push_back(Token(Token::END_OF_FILE));
  }

  size_t currentPosition = 0; 
  while (currentPosition < src.length()) {
    m_token.push_back(read(src, currentPosition));
  }

  ++m_currentLine;
}






