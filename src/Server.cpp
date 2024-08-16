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
    mUsers[newUserId] = aUserName;

    return std::to_string(newUserId);
  }

  std::string GetUserName(const std::string& aUserId)
  {
    const auto userIt = mUsers.find(std::stoi(aUserId));
    if (userIt == mUsers.cend())
    {
      return "Error! Unknown User";
    }
    else
    {
      return userIt->second;
    }
  }

  // Message contains "-1" if not found; else clientID;
  std::string FindUserID(const std::string& supposedUserName)
  {
    for (const auto& [key, value] : mUsers)
    {
      if (value == supposedUserName)
        return std::to_string(key);
    }
    
    return "-1";
  }

private:
  // <UserId, UserName>
  std::map<size_t, std::string> mUsers;
};

Core& GetCore()
{
  static Core core;
  return core;
}

class session
{
public:
  session(boost::asio::io_context& io_context)
      : socket_(io_context) { }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
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
      reqType = json_message["ReqType"];

      std::string reply = "Error! Unknown request type";
      if (reqType == Requests::Registration)
      {
        // Register new user and send's it's ID back to user.
        reply = GetCore().RegisterNewUser(json_message["Message"]);
      }
      else if (reqType == Requests::Hello)
      {
        // Welcoming User.
        // User's name finding via UserID in received message.
        reply = "Hello, " + GetCore().GetUserName(json_message["UserId"]) + "!\n";
      }
       else if (reqType == Requests::FindUser)
      {
        reply = GetCore().FindUserID(json_message["Message"]);
      }

      boost::asio::async_write(socket_,
          boost::asio::buffer(reply, reply.size()),
          boost::bind(&session::handle_write, this,
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
      boost::bind(&session::handle_read, this,
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

  nlohmann::json json_message;
  nlohmann::json reqType;
};

class server final
{
public:
  server(boost::asio::io_context& io_context)
      : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    std::cout << "Server started! Listen " << port << " port" << std::endl;
  }

  ~server()
  {
    sessions_list_.clear();
    std::cout << "Server stopped!" << std::endl;
  }

  server(const server& other) = delete;
  server(server&& other) = delete;
  server& operator=(const server& other) = delete;
  server& operator=(server&& other) = delete;

  void start()
  {
    sessions_list_.emplace_back(io_context_);
    session_it_ = sessions_list_.end();
    acceptor_.async_accept(session_it_->socket(),
        boost::bind(&server::handle_accept, this,
        boost::asio::placeholders::error));
  }

  void handle_accept(const boost::system::error_code& error)
  {
    if (!error)
    {
      session_it_->start();
      sessions_list_.emplace_back(io_context_);
      session_it_ = sessions_list_.end();
      acceptor_.async_accept(session_it_->socket(),
           boost::bind(&server::handle_accept, this,
           boost::asio::placeholders::error));
    }
    else
    {
      sessions_list_.erase(session_it_);
    }
  }

private:
  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
  // List is used because there are multiple sessions 
  // that are created and expired frequently.
  std::list<session> sessions_list_;
  std::list<session>::iterator session_it_;
};

int main()
{
  try
  {
    boost::asio::io_context io_context;
    Core core = GetCore();

    server s(io_context);
    s.start();

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}