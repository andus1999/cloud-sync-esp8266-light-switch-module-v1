#include "FileSystem.h"

FileSystem &FileSystem::getInstance()
{
  static FileSystem instance;
  return instance;
}

FileSystem::FileSystem()
{
  LittleFS.begin();
  // setInit("false");
  // setSsid("");
  // setPass("");
  // setNetworkUid("");
  // setRefreshToken("");

  //   std::string serverCert = R"EOF(
  // -----BEGIN CERTIFICATE-----
  // MIIC3zCCAkigAwIBAgIUKm+hRC17fAJuIIjWUUBc/KuyFxYwDQYJKoZIhvcNAQEL
  // BQAwcjELMAkGA1UEBhMCQVQxDDAKBgNVBAgMA09PRTENMAsGA1UEBwwEV2VsczEX
  // MBUGA1UECgwOQ2xvdWRTeW5jIFtBVF0xEjAQBgNVBAsMCUNsb3VkU3luYzEZMBcG
  // A1UEAwwQY2xvdWQtc3luYy5sb2NhbDAgFw0yMjAxMjgwNjM2MDVaGA8zMDIxMDUz
  // MTA2MzYwNVowcjELMAkGA1UEBhMCQVQxDDAKBgNVBAgMA09PRTENMAsGA1UEBwwE
  // V2VsczEXMBUGA1UECgwOQ2xvdWRTeW5jIFtBVF0xEjAQBgNVBAsMCUNsb3VkU3lu
  // YzEZMBcGA1UEAwwQY2xvdWQtc3luYy5sb2NhbDCBnzANBgkqhkiG9w0BAQEFAAOB
  // jQAwgYkCgYEAxt6K5Ve0LLYGwkfWy/BlmqNJ7ss4875g7v+QnFks6wM46QvdjM1X
  // nLHXyRT4eIicJIXAZxolybkXN+QBhszRxYplwGZlDtS5NOQa2VgTiXMInZPh8TyT
  // xCG1szkWXm+VvM9Oe46wSToHQaZP8ev2qKB76ErYvFjxVkO45ncCn50CAwEAAaNw
  // MG4wHQYDVR0OBBYEFBZBrjfpyzRYWUU8Z+gkJyxbiMTFMB8GA1UdIwQYMBaAFBZB
  // rjfpyzRYWUU8Z+gkJyxbiMTFMA8GA1UdEwEB/wQFMAMBAf8wGwYDVR0RBBQwEoIQ
  // Y2xvdWQtc3luYy5sb2NhbDANBgkqhkiG9w0BAQsFAAOBgQAJAGpIMO3YbLFhKWbz
  // ivsP6jE57B+zzloAvBGaPmF/sTLx/tUrL+EwKmrhWchXnPpqE3p2nU1A1Y3Ydjyp
  // QvcNkpCTeJM1/YfOazf+ZwLaodbbmiSqBDhC3fu9fxod6I34zWq1hFChCeNrqz7o
  // 0vcPnNHTBRgILMceee9KazmrfA==
  // -----END CERTIFICATE-----
  // )EOF";

  //   std::string serverKey = R"EOF(
  // -----BEGIN PRIVATE KEY-----
  // MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBAMbeiuVXtCy2BsJH
  // 1svwZZqjSe7LOPO+YO7/kJxZLOsDOOkL3YzNV5yx18kU+HiInCSFwGcaJcm5Fzfk
  // AYbM0cWKZcBmZQ7UuTTkGtlYE4lzCJ2T4fE8k8QhtbM5Fl5vlbzPTnuOsEk6B0Gm
  // T/Hr9qige+hK2LxY8VZDuOZ3Ap+dAgMBAAECgYEAjFjD90Tk/+TackPWwAiA/Bge
  // 9ipvn/0MVb8ApIZ+n8CfakCYc2T7XrdXzS5CiHYIk/HcriaS4oEAdM4xkZt8skhw
  // 1RceCjIcC5bEcINJM/K/VdPpfcmZj/y/GiPAjGinwZmeUwAtlIDyBlsS9Yw653bS
  // VDktiF96Z9kL2YlVWiECQQDwg5Ua3We1ByGy2Qxc5HrTEIETBKgp53s/Rlca84Fz
  // KsvJ6mzDmYci5BiCkdA5+rBJhj204CZdqrygRK++/QBZAkEA06yGvq38kpLSBjBd
  // JdDqke+0vLDL8T1Tl9KPG2ooq7G0ixyY6Uu5eC4KdsnQBmmyqpseaKP/KGjWhAAJ
  // PJHQ5QJAc5KHH7Wsk+ytYMezjR+tdabEn2Lpe9lj++/NA4JnffPeoVQol4U6zqVU
  // sclnibTQ6UO8eyIKzY+xvEHFQb1JYQJBAKSH/TCn0qQnxWKUI0Mqsbn/5a+piMya
  // C6j+PMhlavmSJrDe6KimeKLGvtF4qt7hN8RO0IMrvkCtxNhTotpWyVUCQFTzDwJZ
  // kdeUzJUqC/yHjWzu3KsOYyBEEI6W80BLu77WLcPpvxfuldDsYjho+QfEqqPVYdbE
  // dmkM4UVYLwWHViI=
  // -----END PRIVATE KEY-----
  // )EOF";

  //   setServerCert(serverCert);
  //   setServerKey(serverKey);
}

FileSystem::~FileSystem()
{
  LittleFS.end();
}

std::string FileSystem::readFile(std::string filename)
{
  File f = LittleFS.open(filename.c_str(), "r");
  if (!f)
  {
    f.close();
    return "";
  }
  std::string result = f.readString().c_str();
  f.close();
  return result;
}

void FileSystem::writeFile(std::string filename, std::string content)
{
  File f = LittleFS.open(filename.c_str(), "w");
  f.write(content.c_str());
  f.close();
}

std::string FileSystem::getSsid()
{
  return readFile(SSID_FILE);
}

std::string FileSystem::getPass()
{
  return readFile(PASS_FILE);
}

std::string FileSystem::getNetworkUid()
{
  return readFile(NETWORK_UID_FILE);
}

std::string FileSystem::getRefreshToken()
{
  return readFile(REFRESH_TOKEN_FILE);
}

std::string FileSystem::getIndex()
{
  return readFile(INDEX_FILE);
}

std::string FileSystem::getSuccess()
{
  return readFile(SUCCESS_FILE);
}

std::string FileSystem::getServerCert()
{
  return readFile(SERVER_CERT_FILE);
}

std::string FileSystem::getServerKey()
{
  return readFile(SERVER_KEY_FILE);
}

std::string FileSystem::getInit()
{
  return readFile(INIT_FILE);
}

std::string FileSystem::getHtml()
{
  return readFile(HTML_FILE);
}

std::string FileSystem::getFont()
{
  return readFile(FONT_FILE);
}

void FileSystem::setSsid(std::string ssid)
{
  writeFile(SSID_FILE, ssid);
}

void FileSystem::setPass(std::string pass)
{
  writeFile(PASS_FILE, pass);
}

void FileSystem::setNetworkUid(std::string networkUid)
{
  writeFile(NETWORK_UID_FILE, networkUid);
}

void FileSystem::setRefreshToken(std::string refreshToken)
{
  writeFile(REFRESH_TOKEN_FILE, refreshToken);
}

void FileSystem::setIndex(std::string html)
{
  writeFile(INDEX_FILE, html);
}

void FileSystem::setSuccess(std::string success)
{
  writeFile(SUCCESS_FILE, success);
}

void FileSystem::setServerCert(std::string cert)
{
  writeFile(SERVER_CERT_FILE, cert);
}

void FileSystem::setServerKey(std::string key)
{
  writeFile(SERVER_KEY_FILE, key);
}

void FileSystem::setInit(std::string init)
{
  writeFile(INIT_FILE, init);
}

void FileSystem::setHtml(std::string html)
{
  writeFile(HTML_FILE, html);
}