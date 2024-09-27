/*
Server and Client communicates via JSON files.

Client sends Request file.
Request is a JSON that contains 3 fields.
1) UserID
2) Request type
3) Message

Server sends Response file.
Response is a JSON that contains 2 fields.
1) Status of processed request (error or success)
2) Message
  - Message is also a JSON that contains:
    1. Data
    2. Text

Test
*/




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

// Requests that Client sends to the Server.
enum class Requests : ushort
{
  Registration,
  Hello,
  FindUser,
  CheckBalance,
  BuyUSD,
  SellUSD,
  Notification
};

// Status of request performed by Server.
enum class Response : ushort
{
  Success,
  Error
};

// namespace Requests
// {
//   static const std::string Registration = "Reg";
//   static const std::string Hello = "Hel";
//   static const std::string FindUser = "FndUsr";
//   static const std::string CheckBalance = "ChkBlnc";
//   static const std::string BuyUSD = "BuyUSD";
//   static const std::string SellUSD = "SellUSD";
//   static const std::string Notification = "Notif";
// }

// namespace Response
// {
//   static const std::string Success = "Success";
//   static const std::string Error = "Error";
// }

#endif //CLIENT_SERV_EXCHANGE_COMMON_HPP