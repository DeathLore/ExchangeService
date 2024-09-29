#include <iostream>
#include <boost/asio.hpp>

#include "Common.hpp"
#include "json.hpp"

using boost::asio::ip::tcp;

void EnteringMenu()
{
  std::cout << "Menu:\n"
               "1) Login\n"
               "2) Register\n"
               "*) Exit\n"
               << std::endl;
}

class Client final
{
public:

  // Starts client's session.
  // Offers to Login, Register new user, Close app.
  Client(tcp::socket& socket) : ClientSocket(socket) 
  {
    std::cout << "Welcome!\n";
    while(true) {
      std::cout << "-------------\n";
      EnteringMenu();
      std::cin >> menu_option_num;

      if (menu_option_num == 1)
      {
        Login(client_ID_);
        break;
      }
      else if (menu_option_num == 2) {
        client_ID_ = ProcessRegistration();
        if (client_ID_ == "-1")
        {
          std::cout << "This user is already registered!\n";
          continue;
        }
        break;
      }
      else
        exit(0);
    }
  }


  // Sending message to the server.
  void SendMessage(
    const std::string& aClientID,
    const Requests aRequestType,
    const std::string& aMessage) [[deprecated]]
  {
    // Request that would be send to the Server.
    nlohmann::json req;
    req[USER_ID] = aClientID;
    req[REQUEST_TYPE] = aRequestType;
    req[MESSAGE] = aMessage;

    // std::cout << req << std::endl;

    std::string request = req.dump();
    boost::asio::write(ClientSocket, boost::asio::buffer(request, request.size()));
  }

  // Sending json as a message to server
  void SendJsonMessage(
    const std::string& aClientID,
    const Requests aRequestType,
    const nlohmann::json& aMessage)
  {
    // Request that would be send to the Server.
    nlohmann::json req;
    req[USER_ID] = aClientID;
    req[REQUEST_TYPE] = aRequestType;
    req[MESSAGE] = aMessage;

    // std::cout << "json: " << req << std::endl;

    std::string request = req.dump();
    boost::asio::write(ClientSocket, boost::asio::buffer(request, request.size()));
  }

  // Returns server's response for last request.
  void ReadMessage()
  {
    std::cout << "RM1\n";
    boost::asio::streambuf b;
    boost::asio::read_until(ClientSocket, b, "\0");
    std::istream is(&b);
    std::getline(is, data_);
    // std::string line(std::istreambuf_iterator<char>(is), {});
    // ClientSocket.read_some(boost::asio::buffer(data_, data_.length()), "\\");
    std::cout << data_ << " RM2\n";
    // return line;
  }

  // Moves server's json response to "message".
  void ReadJsonMessage()
  {
    std::cout << "RJ1\n";
    ReadMessage();
    message = nlohmann::json::parse(data_);
    std::cout << "RJ2\n";
  }

  // Register new User.
  // Returns ClientID.
  std::string ProcessRegistration()
  {
    std::string name;
    std::cout << "Enter your name: ";
    std::cin >> name;

    // For registration ID is not necessary that's why filled it as "0".
    SendMessage("0", Requests::Registration, name);
    ReadJsonMessage();

    return (message[S_STATUS] == Response::Error) ? "-1" : message[MESSAGE][S_DATA];
  }

  // Logging as already existing user
  void Login(std::string& aClientID)
  {
    std::string client_name;

    // Main login loop.
    // Handles unsuccessful login.
    while(true)
    {
      std::cout << "Enter user name: ";
      std::cin >> client_name;
      SendMessage("0", Requests::FindUser, client_name);
      // Message contains "-1" if not found; else clientID;
      std::cout << "1\n";
      ReadJsonMessage();
      std::cout << "2\n";

      // Login was unsuccessful.
      // Offering variants to handle it.
      if (message[S_STATUS] == Response::Error)
      {
        short menu_option_num;
        std::cout << "-------------\n";
        std::cout << "User \"" << client_name << "\" not found.\n";
        EnteringMenu();
        std::cin >> menu_option_num;
        
        // Trying login again.
        if (menu_option_num == 1)
          continue;
        // Register new yser.
        else if (menu_option_num == 2)
        {
          aClientID = ProcessRegistration();
          if (aClientID == "-1")
          {
            std::cout << "This user is already registered!\n";
            continue;
          }
          break;
        }
        // User wished to end using app.
        else
          exit(0);
      }
      
      // Login successful;
      // Saving Client ID.
      aClientID = message;
      break;
    }
  }

  // Starts main client's loop that offers interaction with the Server.
  // Has to be called once after client obj initialization.
  void Start() {
    std::string cin_buffer;
    while (true)
    {
      std::cout << "-------------\n";
      std::cout << "Menu:\n"
                   "1) Hello Request\n"
                   "2) Balance\n"
                   "3) Buy USD\n"
                   "4) Sell USD\n"
                   "5) Notifications\n"
                   "7) Exit\n"
                   << std::endl;

      
      std::cin >> cin_buffer;
      menu_option_num = std::stoi(cin_buffer);
      std::cout << "-------------\n";
      switch (menu_option_num)
      {
        // Checking connection with Hello request.
        case 1:
        {
          SendMessage(client_ID_, Requests::Hello, "");
          ReadJsonMessage();
          std::string printable = message[MESSAGE][S_TEXT].dump();
          // bool delete_slash = true;
          // std::for_each(printable, [&delete_slash](char &c) 
          // {
          //   if (c == '\\' && delete_slash)
          //   {
          //     delete_slash = !delete_slash;
          //     c = '';
          //   }
          // });
          std::cout << printable << std::endl;
          break;
        }
        // Asking user's balance.
        case 2:
        {
          SendMessage(client_ID_, Requests::CheckBalance, "");
          ReadJsonMessage();
          
          if (Response::Success == message[S_STATUS])
          {
            std::cout << "USD: " << message[MESSAGE][S_DATA][USD_BALANCE] << std::endl;
            std::cout << "RUB: " << message[MESSAGE][S_DATA][RUB_BALANCE] << std::endl;
          }
          else
            std::cout << message[MESSAGE][S_TEXT] << std::endl;

          break;
        }
        // Buying USD by selling RUB.
        case 3:
        {
          std::cout << "Enter how many USD you want to buy: ";
          std::cin.sync();
          std::cin >> cin_buffer;
          std::cin.sync();
          uint tradeValue = std::stoi(cin_buffer);
          std::cout << "Enter price of 1 USD in RUBs: ";
          std::cin.sync();
          std::cin >> cin_buffer;
          std::cin.sync();
          uint price = std::stoi(cin_buffer);

          SendJsonMessage(client_ID_, Requests::BuyUSD, nlohmann::json{{PRICE, price}, {TRADE_VALUE, tradeValue}});
          ReadJsonMessage();
          break;
        }
        // Selling USD by buying RUB.
        case 4:
        {
          std::cout << "Enter how many USD you want to sell: ";
          std::cin.sync();
          std::cin >> cin_buffer;
          std::cin.sync();
          uint tradeValue = std::stoi(cin_buffer);
          std::cout << "Enter price of 1 USD in RUBs: ";
          std::cin.sync();
          std::cin >> cin_buffer;
          std::cin.sync();
          uint price = std::stoi(cin_buffer);

          SendJsonMessage(client_ID_, Requests::SellUSD, nlohmann::json{{PRICE, price}, {TRADE_VALUE, tradeValue}});
          ReadJsonMessage();
          break;
        }
        case 5:
        {
          SendMessage(client_ID_, Requests::Notification, "-1");
          ReadJsonMessage();

          if (Response::Success == message[S_STATUS])
          {
            std::cout << message[MESSAGE][S_TEXT];
          }
          else
            std::cout << message[MESSAGE][S_TEXT] << std::endl;
          break;
        }
        // Ends session (and the whole program).
        case 7:
        {
          exit(0);
          break;
        }
        default:
        {
          std::cout << "Unknown menu option\n" << std::endl;
          continue;
        }
      }
    }
  }

private:
  // Client ID in string format.
  // Currently used only as decimal number.
  std::string client_ID_ = "";
  // Socket connected to the Server
  tcp::socket& ClientSocket;
  short menu_option_num;

  // enum { max_length = 1024 };
  // Data transmitted by Server
  std::string data_;
  // 
  nlohmann::json message;
};


#ifndef TESTING
int main()
{
  try
  {
    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
    tcp::resolver::iterator iterator = resolver.resolve(query);

    tcp::socket s(io_context);
    s.connect(*iterator);

    Client client(s);
    client.Start();   
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
#endif