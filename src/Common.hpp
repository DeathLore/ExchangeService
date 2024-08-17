#ifndef CLIENT_SERV_EXCHANGE_COMMON_HPP
#define CLIENT_SERV_EXCHANGE_COMMON_HPP

#include <string>

#define USER_NAME "UserName"
#define RUB_BALANCE "RUBBalance"
#define USD_BALANCE "USDBalance"
#define MESSAGE "Message"
#define REQUEST_TYPE "ReqType"
#define USER_ID "UserID"

static short port = 5555;

namespace Requests
{
  static std::string Registration = "Reg";
  static std::string Hello = "Hel";
  static std::string FindUser = "FndUsr";
  static std::string CheckBalance = "ChkBlnc";
}

#endif //CLIENT_SERV_EXCHANGE_COMMON_HPP