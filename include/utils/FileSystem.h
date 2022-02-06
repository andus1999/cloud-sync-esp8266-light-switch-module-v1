#pragma once

#include <string>
#include <LittleFS.h>

#define SSID_FILE "SSID.txt"
#define PASS_FILE "PASS.txt"
#define REFRESH_TOKEN_FILE "REFRESH_TOKEN.txt"
#define NETWORK_UID_FILE "NETWORK_UID.txt"
#define INDEX_FILE "index.html"
#define SUCCESS_FILE "success.html"
#define SERVER_CERT_FILE "cert.txt"
#define SERVER_KEY_FILE "key.txt"
#define INIT_FILE "init.txt"
#define HTML_FILE "index.html"
#define FONT_FILE "font.tff"

class FileSystem
{
public:
  FileSystem(FileSystem const &) = delete;
  void operator=(FileSystem const &) = delete;
  static FileSystem &getInstance();

  std::string getSsid();
  std::string getPass();
  std::string getNetworkUid();
  std::string getRefreshToken();
  std::string getIndex();
  std::string getSuccess();
  std::string getServerCert();
  std::string getServerKey();
  std::string getInit();
  std::string getHtml();
  std::string getFont();

  void setSsid(std::string ssid);
  void setPass(std::string pass);
  void setNetworkUid(std::string uid);
  void setRefreshToken(std::string refreshToken);
  void setServerCert(std::string serverCert);
  void setServerKey(std::string serverKey);
  void setInit(std::string init);
  void setHtml(std::string html);

private:
  FileSystem();
  ~FileSystem();

  std::string readFile(std::string filename);
  void writeFile(std::string filename, std::string content);
  void setIndex(std::string index);
  void setSuccess(std::string success);
};