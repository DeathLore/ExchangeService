#include <cstdlib>
#include <iostream>
#include <list>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include "json.hpp"
#include "Common.hpp"

using boost::asio::ip::tcp;

class Notification final
{
public:

void addNotification(const std::string UserID) {

}

std::string sendNotification(const std::string UserID) {

}

private:

// <UserID, Notifications>
std::map<std::string, std::string> mNotifications; 

};

class Trade final
{
  // Map was used for fast searching Prices.
  // List as a map's value was used to store undefined amount of user's
  // trade requests, moreover every trade requires to read only the first or last request.
  // Each list's node stores UserID and amount of money to exchange.
  using UserInfo = std::list<std::pair<std::string, uint>>;
  // <Price, UserInfo>
  // UserInfo: <UserID, TradeValue>
  std::map<uint, UserInfo> Buyers;
  std::map<uint, UserInfo> Sellers;

public:
  nlohmann::json add_buyer(const std::string& UserID, const uint& Price, uint& TradeValue)
  {
    nlohmann::json return_info;

    auto found_seller = find_seller_request(Price);
    if (found_seller != Sellers.end())
    {
      uint sellers_value = (*(*found_seller).second.begin()).second;
      if (TradeValue == sellers_value)
      {
        return_info[(*(*found_seller).second.begin()).first] = {{SUB_BALANCE_USD, sellers_value}, {ADD_BALANCE_RUB, sellers_value * (*found_seller).first}};
        return_info[UserID] = {{ADD_BALANCE_USD, TradeValue}, {SUB_BALANCE_RUB, TradeValue * (*found_seller).first}};
        return_info[UserID] += {"Seller_ID", (*(*found_seller).second.begin()).first};
        (*found_seller).second.pop_front();
        // std::cout << "B 1.1" << std::endl;
      }
      else if (TradeValue < sellers_value)
      {
        return_info[(*(*found_seller).second.begin()).first] = {{SUB_BALANCE_USD, TradeValue}, {ADD_BALANCE_RUB, TradeValue * (*found_seller).first}};
        return_info[UserID] = {{ADD_BALANCE_USD, TradeValue}, {SUB_BALANCE_RUB, TradeValue * (*found_seller).first}};
        return_info[UserID] += {"Seller_ID", (*(*found_seller).second.begin()).first};
        (*(*found_seller).second.begin()).second -= TradeValue;
        // std::cout << "B 1.2" << std::endl;
      }
      else
      {
        return_info[(*(*found_seller).second.begin()).first] = {{SUB_BALANCE_USD, sellers_value}, {ADD_BALANCE_RUB, sellers_value * (*found_seller).first}};
        return_info[UserID] = {{ADD_BALANCE_USD, sellers_value}, {SUB_BALANCE_RUB, sellers_value * (*found_seller).first}};
        return_info[UserID] += {"Seller_ID", (*(*found_seller).second.begin()).first};
        (*found_seller).second.pop_front();
        TradeValue -= sellers_value;
        // std::cout << "B 1.3" << std::endl;
        return_info["Continue"] = -1;
      }
    }
    else
    {
      Buyers[Price].emplace_back(UserID, TradeValue);
      return_info["JustAdded"] = -1;
      // std::cout << "B 1.0" << std::endl;
    }
    

    return return_info;
  }

  nlohmann::json add_seller(const std::string& UserID, const uint& Price, uint& TradeValue)
  {
    nlohmann::json return_info;

    auto found_buyer = find_buyer_request(Price);
    if (found_buyer != Buyers.rend())
    {
      uint buyers_value = (*(*found_buyer).second.begin()).second;
      if (TradeValue == buyers_value)
      {
        return_info[(*(*found_buyer).second.begin()).first] = {{ADD_BALANCE_USD, buyers_value}, {SUB_BALANCE_RUB, buyers_value * (*found_buyer).first}};
        return_info[UserID] = {{SUB_BALANCE_USD, TradeValue}, {ADD_BALANCE_RUB, TradeValue * (*found_buyer).first}};
        return_info[UserID] += {"Buyer_ID", (*(*found_buyer).second.begin()).first};
        (*found_buyer).second.pop_front();
        // std::cout << "S 1.1" << std::endl;
      }
      else if (TradeValue < buyers_value)
      {
        return_info[(*(*found_buyer).second.begin()).first] = {{ADD_BALANCE_USD, TradeValue}, {SUB_BALANCE_RUB, TradeValue * (*found_buyer).first}};
        return_info[UserID] = {{SUB_BALANCE_USD, TradeValue}, {ADD_BALANCE_RUB, TradeValue * (*found_buyer).first}};
        return_info[UserID] += {"Buyer_ID", (*(*found_buyer).second.begin()).first};
        (*(*found_buyer).second.begin()).second -= TradeValue;
        // std::cout << "S 1.2" << std::endl;
      }
      else
      {
        return_info[(*(*found_buyer).second.begin()).first] = {{ADD_BALANCE_USD, buyers_value}, {SUB_BALANCE_RUB, buyers_value * (*found_buyer).first}};
        return_info[UserID] = {{SUB_BALANCE_USD, buyers_value}, {ADD_BALANCE_RUB, buyers_value * (*found_buyer).first}};
        return_info[UserID] += {"Buyer_ID", (*(*found_buyer).second.begin()).first};
        (*found_buyer).second.pop_front();
        TradeValue -= buyers_value;
        // std::cout << "S 1.3" << std::endl;
        return_info["Continue"] = -1;
      }
    }
    else
    {
      Sellers[Price].emplace_back(UserID, TradeValue);
      return_info["JustAdded"] = -1;
      // std::cout << "S 1.0" << std::endl;
    }

    return return_info;
    
  }

  std::reverse_iterator<std::map<uint, Trade::UserInfo>::iterator> find_buyer_request(const uint& MinPrice) 
  {
    auto map_iterator = Buyers.rbegin();
    for (; map_iterator != Buyers.rend(); ++map_iterator)
    {
      if (!((*map_iterator).second.empty())
          && MinPrice <= (*map_iterator).first)
        break;
    }

    return map_iterator;
  }

  std::map<uint, Trade::UserInfo>::iterator find_seller_request(const uint& MaxPrice)
  {
    auto map_iterator = Sellers.begin();
    for (; map_iterator != Sellers.end(); ++map_iterator)
    {
      if (!((*map_iterator).second.empty())
          && MaxPrice >= (*map_iterator).first)
        break;
    }

    return map_iterator;
  }

};


class Core final
{
public:
  // Returns new user's ID as a string.
  std::string RegisterNewUser(const std::string& aUserName)
  {
    size_t newUserId = mUsers.size();
    mUsers[newUserId][USER_NAME] = aUserName;
    mUsers[newUserId][RUB_BALANCE] = 0;
    mUsers[newUserId][USD_BALANCE] = 0;

    return std::move(std::to_string(newUserId));
  }

  std::string GetUserName(const std::string& aUserID)
  {
    const auto userIt = mUsers.find(std::stoi(aUserID));
    if (userIt == mUsers.cend())
    {
      return "Error! Unknown User";
    }
    else
    {
      return userIt->second[USER_NAME];
    }
  }

  // Message contains "-1" if not found; else clientID;
  std::string FindUserID(const std::string& supposedUserName)
  {
    for (const auto& [key, value] : mUsers)
    {
      if (value[USER_NAME] == supposedUserName)
        return std::to_string(key);
    }
    
    return "-1";
  }

  int GetBalanceUSD(const std::string& aUserID)
  {
    return mUsers[std::stoi(aUserID)].value(USD_BALANCE, 0);
  }
  int GetBalanceRUB(const std::string& aUserID)
  {
    return mUsers[std::stoi(aUserID)].value(RUB_BALANCE, 0);
  }

  void buyUSD(const std::string& UserID, const uint& Price, uint TradeValue)
  {
    nlohmann::json tradeResult;

    while (true) 
    {
      tradeResult = tradeRequests.add_buyer(UserID, Price, TradeValue);

      if (!tradeResult.contains("JustAdded")) 
      {
        int iUserID = std::stoi(UserID);
        mUsers[iUserID][USD_BALANCE] = mUsers[iUserID].value(USD_BALANCE, 0) + tradeResult[UserID].value(ADD_BALANCE_USD, 0);
        mUsers[iUserID][RUB_BALANCE] = mUsers[iUserID].value(RUB_BALANCE, 0) - tradeResult[UserID].value(SUB_BALANCE_RUB, 0);

        std::string seller_ID = tradeResult[UserID]["Seller_ID"];
        uint iSeller_ID = std::stoi(seller_ID);
        mUsers[iSeller_ID][USD_BALANCE] = mUsers[iSeller_ID].value(USD_BALANCE, 0) - tradeResult[seller_ID].value(SUB_BALANCE_USD, 0);
        mUsers[iSeller_ID][RUB_BALANCE] = mUsers[iSeller_ID].value(RUB_BALANCE, 0) + tradeResult[seller_ID].value(ADD_BALANCE_RUB, 0);
        // NOTIFY HERE
      }

      if (!tradeResult.contains("Continue"))
        break;
    }
  }

  void sellUSD(const std::string& UserID, const uint& Price, uint TradeValue)
  {
    nlohmann::json tradeResult;

    while (true) 
    {
      tradeResult = tradeRequests.add_seller(UserID, Price, TradeValue);
      
      if (!tradeResult.contains("JustAdded")) 
      {
        int iUserID = std::stoi(UserID);

        mUsers[iUserID][USD_BALANCE] = mUsers[iUserID].value(USD_BALANCE, 0) - tradeResult[UserID].value(SUB_BALANCE_USD, 0);
        mUsers[iUserID][RUB_BALANCE] = mUsers[iUserID].value(RUB_BALANCE, 0) + tradeResult[UserID].value(ADD_BALANCE_RUB, 0);

        std::string buyer_ID = tradeResult[UserID]["Buyer_ID"];
        int iBuyer_ID = std::stoi(buyer_ID);
        mUsers[iBuyer_ID][USD_BALANCE] = mUsers[iBuyer_ID].value(USD_BALANCE, 0) + tradeResult[buyer_ID].value(ADD_BALANCE_USD, 0);
        mUsers[iBuyer_ID][RUB_BALANCE] = mUsers[iBuyer_ID].value(RUB_BALANCE, 0) - tradeResult[buyer_ID].value(SUB_BALANCE_RUB, 0);
        // NOTIFY HERE
      }

      if (!tradeResult.contains("Continue"))
        break;
    }
  }


private:
  // <UserId, UserInfo>
  // UserInfo contains: UserName, RUB_Balance, USD_Balance
  std::map<size_t, nlohmann::json> mUsers;

  Trade tradeRequests;
  Notification notifications;
};

Core& GetCore()
{
  static Core core;
  return core;
}


class Session final
{
public:
  Session(boost::asio::io_context& io_context)
      : socket_(io_context) { }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&Session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  }

  void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
  {
    if (!error)
    {
      data_[bytes_transferred] = '\0';

      // Parsing json, that was read.
      json_message = nlohmann::json::parse(data_);
      reqType = json_message[REQUEST_TYPE];

      std::string reply = "Error! Unknown request type";
      // TO DO: Replace if sequence to switch.
      if (reqType == Requests::Registration)
      {
        if (core.FindUserID(json_message[MESSAGE]) == "-1")
        // Register new user and send it's ID back to user.
          reply = core.RegisterNewUser(json_message[MESSAGE]);
        else
          // If user already registered - error "-1".
          reply = "-1";
      }
      else if (reqType == Requests::Hello)
      {
        // Welcoming User.
        // User's name finding via UserID in received message.
        reply = "Hello, " + core.GetUserName(json_message.value(USER_ID, "-1")) + "!\n";
      }
       else if (reqType == Requests::FindUser)
      {
        reply = core.FindUserID(json_message[MESSAGE]);
      }
      else if (reqType == Requests::CheckBalance)
      {
        reply = nlohmann::json{
          {USD_BALANCE, core.GetBalanceUSD(json_message.value(USER_ID, "-1"))},
          {RUB_BALANCE, core.GetBalanceRUB(json_message.value(USER_ID, "-1"))}
        }.dump();
      } 
      else if (reqType == Requests::BuyUSD)
      {
        core.buyUSD(json_message[USER_ID], json_message[MESSAGE].value(PRICE, -1), json_message[MESSAGE].value(TRADE_VALUE, -1));
        reply = "-1";
      }
      else if (reqType == Requests::SellUSD)
      {
        core.sellUSD(json_message[USER_ID], json_message[MESSAGE].value(PRICE, -1), json_message[MESSAGE].value(TRADE_VALUE, -1));
        reply = "-1";
      }

      boost::asio::async_write(socket_,
          boost::asio::buffer(reply, reply.size()),
          boost::bind(&Session::handle_write, this,
          boost::asio::placeholders::error));
    }
    else
    {
      delete this;
    }
  }

  void handle_write(const boost::system::error_code& error)
  {
    if (!error)
    {
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&Session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
  }

private:
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];

  Core& core = GetCore();

  nlohmann::json json_message;
  nlohmann::json reqType;
};

class Server final
{
public:
  Server(boost::asio::io_context& io_context)
      : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    std::cout << "Server started! Listen " << port << " port" << std::endl;
  }

  ~Server()
  {
    std::cout << "Server stopped!" << std::endl;
  }

  Server(const Server& other) = delete;
  Server(Server&& other) = delete;
  Server& operator=(const Server& other) = delete;
  Server& operator=(Server&& other) = delete;

  void start()
  {
    Session* new_session = new Session(io_context_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&Server::handle_accept, this,
        new_session,
        boost::asio::placeholders::error));
  }

  void handle_accept(Session* new_session, const boost::system::error_code& error)
  {
    if (!error)
    {
      new_session->start();
      new_session = new Session(io_context_);
      acceptor_.async_accept(new_session->socket(),
                boost::bind(&Server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
    }
    else
    {
      delete new_session;
    }
  }

private:
  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
};

int main() try
{
  boost::asio::io_context io_context;
  Server s(io_context);
  s.start();

  io_context.run();
  return 0;
}
catch (std::exception& e)
{
  std::cerr << "Exception: " << e.what() << "\n";
  return 1;
}

  
