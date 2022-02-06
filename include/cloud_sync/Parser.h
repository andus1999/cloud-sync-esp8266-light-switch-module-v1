#pragma once
#include <string>
#include <Arduino.h>

class Parser {
public:
  typedef std::function<void(std::string, std::string)> ParserCallback;
  Parser(ParserCallback, bool eventParser = false);
  Parser() = default;
  void resetParser(void);
  void parse(const char *data, int len);
  
private:
  ParserCallback callback;
  enum ParserState
  {
    EVENT,
    DATA,
    PATH,
    PATHDATA,
    PAYLOAD,
    JSON,
    KEY,
    JSON_VALUE,
    VALUE,
    DONE,
    WAIT
  };
  ParserState parserState;

  std::string event;
  std::string path;
  std::string value;
  bool quote;
  void updateValue();
  bool jsonParser = false;
};
