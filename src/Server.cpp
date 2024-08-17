#include <cstdlib>
#include <iostream>
#include <list>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include "json.hpp"
#include "Common.hpp"

using boost::asio::ip::tcp;

class Core
{
public:
  // Returns new user's ID as a string.
  std::string RegisterNewUser(const std::string& aUserName)
  {
    size_t newUserId = mUsers.size();
    mUsers[newUserId][USER_NAME] = aUserName;
    mUsers[newUserId][RUB_BALANCE] = "0";
    mUsers[newUserId][USD_BALANCE] = "0";

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

  std::string GetBalanceUSD(const std::string& aUserID)
  {
    return mUsers[std::stoi(aUserID)][USD_BALANCE];
  }
  std::string GetBalanceRUB(const std::string& aUserID)
  {
    return mUsers[std::stoi(aUserID)][RUB_BALANCE];
  }


private:
  // <UserId, UserInfo>
  // UserInfo contains: UserName, RUB_Balance, USD_Balance
  std::map<size_t, nlohmann::json> mUsers;
};

Core& GetCore()
{
  static Core core;
  return core;
}

class Session
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
        reply = "Hello, " + core.GetUserName(json_message[USER_ID]) + "!\n";
      }
       else if (reqType == Requests::FindUser)
      {
        reply = core.FindUserID(json_message[MESSAGE]);
      }
      else if (reqType == Requests::CheckBalance)
      {
        reply = nlohmann::json{
          {USD_BALANCE, core.GetBalanceUSD(json_message[USER_ID]).c_str()},
          {RUB_BALANCE, core.GetBalanceRUB(json_message[USER_ID]).c_str()}
        }.dump();
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

int main()
{
  try
  {
    boost::asio::io_context io_context;
    Core core = GetCore();

    Server s(io_context);
    s.start();

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}