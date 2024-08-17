#include <iostream>
#include <boost/asio.hpp>

#include "Common.hpp"
#include "json.hpp"

using boost::asio::ip::tcp;

// Sending message to the server.
void SendMessage(
  tcp::socket& aSocket,
  const std::string& aClientID,
  const std::string& aRequestType,
  const std::string& aMessage)
{
  nlohmann::json req;
  req["UserId"] = aClientID;
  req["ReqType"] = aRequestType;
  req["Message"] = aMessage;

  std::string request = req.dump();
  boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}

// Returns server's response for last request.
std::string ReadMessage(tcp::socket& aSocket)
{
  boost::asio::streambuf b;
  boost::asio::read_until(aSocket, b, "\0");
  std::istream is(&b);
  std::string line(std::istreambuf_iterator<char>(is), {});
  return line;
}

// Register new User.
// Returns ClientID.
std::string ProcessRegistration(tcp::socket& aSocket)
{
  std::string name;
  std::cout << "Hello! Enter your name: ";
  std::cin >> name;

  // For registration ID is not necessary that's why filled it as "0".
  SendMessage(aSocket, "0", Requests::Registration, name);
  return ReadMessage(aSocket);
}

void EnteringMenu()
{
  std::cout << "Menu:\n"
               "1) Login\n"
               "2) Register\n"
               "*) Exit\n"
               << std::endl;
}

void Login(std::string& aClientID, tcp::socket& aSocket)
{
  std::string client_name, message;

  while(true)
  {
    std::cout << "Enter user name: ";
    std::cin >> client_name;
    SendMessage(aSocket, "0", Requests::FindUser, client_name);
    // Message contains "-1" if not found; else clientID;
    message = ReadMessage(aSocket);

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
        aClientID = ProcessRegistration(aSocket);
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

    std::string client_ID = "0";
    short menu_option_num;

    std::cout << "Welcome!\n";
    while(true) {
      std::cout << "-------------\n";
      EnteringMenu();
      std::cin >> menu_option_num;

      if (menu_option_num == 1)
      {
        Login(client_ID, s);
        break;
      }
      else if (menu_option_num == 2) {
        client_ID = ProcessRegistration(s);
        if (client_ID == "-1")
        {
          std::cout << "This user is already registered!\n";
          continue;
        }
        break;
      }
      else
        exit(0);
    }
    
    std::string cin_buffer;
    while (true)
    {
      std::cout << "-------------\n";
      std::cout << "Menu:\n"
                   "1) Hello Request\n"
                   "2) Exit\n"
                   << std::endl;

      
      std::cin >> cin_buffer;
      menu_option_num = std::stoi(cin_buffer);
      std::cout << "-------------\n";
      switch (menu_option_num)
      {
        // Checking connection with Hello request.
        case 1:
        {
          SendMessage(s, client_ID, Requests::Hello, "");
          std::cout << ReadMessage(s);
          break;
        }
        case 2:
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
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}