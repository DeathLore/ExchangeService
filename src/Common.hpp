#ifndef CLIENT_SERV_EXCHANGE_COMMON_HPP
#define CLIENT_SERV_EXCHANGE_COMMON_HPP

#include <string>

#define USER_NAME "UserName"
#define RUB_BALANCE "BalanceRUB"
#define USD_BALANCE "BalanceUSD"
#define ADD_BALANCE_RUB "AddBalanceRUB"
#define ADD_BALANCE_USD "AddBalanceUSD"
#define SUB_BALANCE_RUB "SubBalanceRUB"
#define SUB_BALANCE_USD "SubBalanceUSD"
#define MESSAGE "Message"
#define REQUEST_TYPE "ReqType"
#define USER_ID "UserID"
#define PRICE "Price"
#define TRADE_VALUE "TradeValue"
#define STATUS "Status"

// Server's port
static short port = 5555;

// Requests that Client send to the Server.
namespace Requests
{
  static std::string Registration = "Reg";
  static std::string Hello = "Hel";
  static std::string FindUser = "FndUsr";
  static std::string CheckBalance = "ChkBlnc";
  static std::string BuyUSD = "BuyUSD";
  static std::string SellUSD = "SellUSD";
  static std::string Notification = "Notif";
}

namespace Response
{
  static std::string Success = "Success";
  static std::string Error = "Error";
}

#endif //CLIENT_SERV_EXCHANGE_COMMON_HPP