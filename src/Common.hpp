#ifndef CLIENT_SERV_EXCHANGE_COMMON_HPP
#define CLIENT_SERV_EXCHANGE_COMMON_HPP

#include <string>

static short port = 5555;

namespace Requests
{
  static std::string Registration = "Reg";
  static std::string Hello = "Hel";
}

#endif //CLIENT_SERV_EXCHANGE_COMMON_HPP