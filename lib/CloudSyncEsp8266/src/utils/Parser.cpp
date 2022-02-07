#include "Parser.h"

Parser::Parser(ParserCallback cb, bool eventParser)
{
  callback = cb;
  resetParser();
  if (!eventParser)
  {
    parserState = JSON;
    jsonParser = true;
  }
}

void Parser::resetParser()
{
  event = "";
  path = "";
  value = "";
  parserState = EVENT;
  quote = false;
}

void Parser::parse(const char *data, int len)
{
  for (int i = 0; i < len; i++)
  {
    char c = data[i];
    // Serial.write(c);
    if (c == ' ' || c == '\t')
      continue;
    if (c == '"')
    {
      quote = !quote;
      continue;
    }
    if (c == '\n' && jsonParser)
      continue;
    switch (parserState)
    {
    case EVENT:
      if (c == '\n')
      {
        parserState = DATA;
      }
      else
      {
        event.push_back(c);
      }
      break;
    case DATA:
      if (event == "event: keep-alive")
      {
        event = "";
        parserState = DONE;
      }
      if (c == '{' && !quote)
      {
        parserState = PATH;
      }
      break;
    case PATH:
      if (c == ':' && !quote)
      {
        parserState = PATHDATA;
      }
      break;
    case PATHDATA:
      if (quote)
      {
        path.push_back(c);
      }
      else if (c == ',')
      {
        parserState = PAYLOAD;
      }
      break;
    case PAYLOAD:
      if (c == ':' && !quote)
      {
        parserState = JSON;
      }
      break;
    case JSON:
      if (c == '{')
      {
        parserState = KEY;
        if (path.size() != 1)
          path.push_back('/');
      }
      else if (!jsonParser)
      {
        parserState = VALUE;
        value.push_back(c);
      }
      break;
    case KEY:
      if (quote)
      {
        path.push_back(c);
      }
      else if (c == '}')
      {
        path = path.substr(0, path.find_last_of('/'));
      }
      else if (c == ':')
      {
        parserState = JSON_VALUE;
      }
      else if (c == '\n')
      {
        parserState = WAIT;
      }
      break;
    case VALUE:
      if (c == '\n')
      {
        updateValue();
        parserState = WAIT;
      }
      else if (c != '}')
      {
        value.push_back(c);
      }
      break;
    case JSON_VALUE:
      if (c == '{' && !quote)
      {
        parserState = KEY;
        path.push_back('/');
      }
      else if (c == '}' && !quote)
      {
        updateValue();
        path = path.substr(0, path.find_last_of('/'));
        path = path.substr(0, path.find_last_of('/'));
        path.push_back('/');
        parserState = KEY;
      }
      else if (c == ',' && !quote)
      {
        updateValue();
        path = path.substr(0, path.find_last_of('/'));
        path.push_back('/');
        parserState = KEY;
      }
      else
      {
        value.push_back(c);
      }
    case DONE:
      if (c == '\n' && !quote)
      {
        parserState = WAIT;
      }
      break;
    case WAIT:
      if (c == '\n' && !quote)
      {
        parserState = EVENT;
        resetParser();
      }
      break;
    default:
      break;
    }
  }
}

void Parser::updateValue()
{
  std::string loc = path;
  loc.erase(0, 1);
  // Serial.println(loc.c_str());
  // Serial.println(value.c_str());
  callback(loc, value);
  value = "";
}
