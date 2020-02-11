#include <asio.hpp>
#include <functional>
#include <iostream>
#include "httpparser.hpp"

using namespace asio::ip;
using namespace std::placeholders;

class Session : public std::enable_shared_from_this<Session> {
public:
    using Pointer = std::shared_ptr<Session>;

    // methods
public:
    void startSession();
    static Pointer create(asio::io_context& context) { return std::make_shared<Session>(context); }
    inline tcp::socket& socket() { return socket_; }
    // ctors and dtors
public:
    Session(asio::io_context& context)
        : socket_(context),
          maxMessageLength_(2048),
          readMessage_(maxMessageLength_, '\0'),
          parser_("./Html/") {}
    ~Session() { std::cout << "dtor::session*****************************\n" << std::endl; }

private:
    void onReceive(const asio::error_code& ec, std::size_t bytesReceived);
    void onSend(const asio::error_code& ec, std::size_t bytesTransfered);

private:
    tcp::socket socket_;
    std::size_t maxMessageLength_;
    std::string readMessage_;
    std::string messageToSend_;
    HttpParser parser_;
};

void Session::startSession() {
    socket_.async_receive(asio::buffer(readMessage_),
                          std::bind(&Session::onReceive, shared_from_this(), _1, _2));
}

void Session::onReceive(const asio::error_code& ec, std::size_t bytesReceived) {
    if (ec) {
        std::cout << "Session::onReceive: " << ec.message() << std::endl;
    }

    // parse http request and write http response message
    auto http = parser_.parseHttpHeader(readMessage_);
    messageToSend_ = parser_.formResponse(http);

    socket_.async_send(asio::buffer(messageToSend_),
                       std::bind(&Session::onSend, shared_from_this(), _1, _2));
}

void Session::onSend(const asio::error_code& ec, std::size_t bytesTransfered) {
    if (ec) {
        std::cout << "Session::onSend: " << ec.message() << std::endl;
    }

    //    socket_.async_receive(asio::buffer(readMessage_),
    //                          std::bind(&Session::onReceive, shared_from_this(), _1, _2));
}

class Server {
    // ctors and dtors
public:
    Server(asio::io_context& context, short port)
        : acceptor_(context, tcp::endpoint(tcp::v4(), port)) {
        startListen();
    }
    ~Server() { std::cout << "Server dtor" << std::endl; }

    // methods
private:
    void startListen();
    void onAccept(Session::Pointer session, const asio::error_code& ec);

    // fields
private:
    tcp::acceptor acceptor_;
};

void Server::startListen() {
    Session::Pointer session = Session::create(acceptor_.get_executor().context());
    acceptor_.async_accept(session->socket(), std::bind(&Server::onAccept, this, session, _1));
}

void Server::onAccept(Session::Pointer session, const asio::error_code& ec) {
    if (ec) {
        std::cout << "Server::onAccept" << ec.message() << std::endl;
    }

    session->startSession();
    startListen();
}

int main(int argc, char* argv[]) {
    std::string address;
    short port = 12345;
    std::string directory;
    char option;
    if (argc < 4) {
        std::cout << "Not enough arguments\n";
        return 1;
    }
    while ((option = getopt(argc, argv, ":h:p:d:")) != -1) {
        switch (option) {
            case 'h':
                address = optarg;
                break;
            case 'p':
                port = std::stoi(optarg);
                break;
            case 'd':
                directory = optarg;
                break;
            case ':':
                printf("option needs a value\n");
                break;
            case '?':
                printf("unknown option: %c\n", optopt);
                break;
        }
    }

    asio::io_context context;
    Server server(context, port);
    context.run();
    return 0;
}
