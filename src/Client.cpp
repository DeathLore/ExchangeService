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
    const std::string& aRequestType,
    const std::string& aMessage)
  {
    nlohmann::json req;
    req[USER_ID] = aClientID;
    req[REQUEST_TYPE] = aRequestType;
    req[MESSAGE] = aMessage;

    // std::cout << req << std::endl;

    std::string request = req.dump();
    boost::asio::write(ClientSocket, boost::asio::buffer(request, request.size()));
  }

  void SendJsonMessage(
    const std::string& aClientID,
    const std::string& aRequestType,
    const nlohmann::json& aMessage)
  {
    nlohmann::json req;
    req[USER_ID] = aClientID;
    req[REQUEST_TYPE] = aRequestType;
    req[MESSAGE] = aMessage;

    // std::cout << "json: " << req << std::endl;

    std::string request = req.dump();
    boost::asio::write(ClientSocket, boost::asio::buffer(request, request.size()));
  }

  // Returns server's response for last request.
  std::string ReadMessage()
  {
    boost::asio::streambuf b;
    boost::asio::read_until(ClientSocket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
  }

  nlohmann::json ReadJsonMessage()
  {
    return nlohmann::json::parse(ReadMessage());
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
    return ReadMessage();
  }

  void Login(std::string& aClientID)
  {
    std::string client_name, message;

    while(true)
    {
      std::cout << "Enter user name: ";
      std::cin >> client_name;
      SendMessage("0", Requests::FindUser, client_name);
      // Message contains "-1" if not found; else clientID;
      message = ReadMessage();

      if (std::stoi(message) == -1)
      {
        short menu_option_num;
        std::cout << "-------------\n";
        std::cout << "User \"" << client_name << "\" not found.\n";
        EnteringMenu();
        std::cin >> menu_option_num;
        
        if (menu_option_num == 1)
          continue;
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
        else
          exit(0);
      }
      
      aClientID = message;
      break;
    }
  }


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
          std::cout << ReadMessage();
          break;
        }
        case 2:
        {
          SendMessage(client_ID_, Requests::CheckBalance, "");
          nlohmann::json balance = ReadJsonMessage();
          std::cout << "USD: " << balance[USD_BALANCE] << std::endl;
          std::cout << "RUB: " << balance[RUB_BALANCE] << std::endl;
          break;
        }
        case 3:
        {
          uint price, tradeValue;
          std::cout << "Enter how many USD you want to buy: ";
          std::cin >> tradeValue;
          std::cout << "Enter price of 1 USD in RUBs: ";
          std::cin >> price;

          nlohmann::json message_json = {{PRICE, price}, {TRADE_VALUE, tradeValue}};

          SendJsonMessage(client_ID_, Requests::BuyUSD, message_json);
          ReadMessage();
          break;
        }
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
          ReadMessage();
          break;
        }
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
  std::string client_ID_ = "";
  tcp::socket& ClientSocket;
  short menu_option_num;

  enum { max_length = 1024 };
  char data_[max_length];
};

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