// CS136 support code
// By Morgan McGuire morgan@cs.williams.edu and Sam Donow
// Williams College Computer Science Department, 2014
// Released into the Public Domain
//
// Last updated 2014-08-01

#ifndef Scanner_h
#define Scanner_h

#include <stdio.h>
#include <string>
#include <vector>

/** Location in a file from which a Token originated. */
class SourceLocation {
 public:
  std::string         filename;

  /** The first line of a file is 1 */
  int                 line;

  /** The first character on a line is 1 */
  int                 character;

 SourceLocation() : line(0), character(0) {}

 SourceLocation(const std::string& s, int L, int c) : filename(s), line(L), character(c) {}
};


/** Representation of a simple value with information on its origin. */
class Token {
 public:

  enum Type {STRING, SYMBOL, NUMBER, BOOLEAN, END_OF_LINE, END_OF_FILE};

 private:

  Type              m_type;
  std::string       m_string;
  double            m_number;
  bool              m_boolean;
  SourceLocation    m_location;
    
 public:

 Token() : m_string("<UNITIALIZED>"), m_number(0), m_boolean(false) {}

  /** t must be END_OF_FILE or END_OF_LINE */
  Token(Type t, const SourceLocation& location = SourceLocation());

  /** t must be NUMBER */
  Token(Type t, double n, const SourceLocation& location = SourceLocation());

  /** t must be BOOLEAN */
  Token(Type t, bool b, const SourceLocation& location = SourceLocation());

  /** T must be SYMBOL or STRING */
  Token(Type t, const std::string& s, const SourceLocation& location = SourceLocation());

  /** T must be SYMBOL or STRING */
  Token(Type t, const char* s, const SourceLocation& location = SourceLocation());
    
  Type type() const;

  /** This is the contents of the string value (i.e., without quotes). The token must have type STRING */
  const std::string& string() const;

  /** A printable version of this token */
  const char* c_str() const;

  /** The token must have type SYMBOL */
  const std::string& symbol() const;

  /** The token must have type NUMBER */
  double number() const;

  /** The token must have type BOOLEAN */
  bool boolean() const;

  /** Origin of this token */
  const SourceLocation& sourceLocation() const;
};


/** Reads input from stdin and presents it as Tokens */
class Scanner {
 private:
  /** If not NULL, read from the source */
  FILE*               m_source;

  /** Used if m_source is NULL */
  std::string         m_string;

  /** Line from which m_token was read */
  int                 m_currentLine;

  size_t              m_nextTokenIndex;

  /** A queue of tokens that are to be returned from next() */
  std::vector<Token>  m_token;

  /** Clears m_token and then Reads a full line of tokens into
      m_token. Called from next() and Scanner() */
  void readAhead();

  /** Gets the next Token from str starting at currentPosition,
      and updates currentPosition */
  Token read(const std::string& str, size_t& currentPosition) const;

 public: 

  /** source must not be NULL, but may be stdin (the default)
        Does not close the source when the instance class terminates. 
        Offsets reported line numbers in SourceLocation by startLineNumber. */
  Scanner(FILE* source = stdin, int startLineNumber = 1);

  /** Parses from the string str. Note that next() will block
        indefinitely at the end of the string, so you should always
        include some kind of sentinel token at the end of the string. */
  Scanner(const std::string& str);

  /** Return the next token.
      Will block if reading from stdin and the next token has not been typed yet. */
  Token next();

};

#endif
