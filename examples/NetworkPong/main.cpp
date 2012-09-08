// All of Gosu.
#include <Gosu/Gosu.hpp>
// To safely include std::tr1::shared_ptr
#include <Gosu/TR1.hpp>
// Makes life easier for Windows users compiling this.
#include <Gosu/AutoLink.hpp>

#include <cmath>
#include <cstdlib>
#include <list>
#include <memory>
#include <sstream> // For int <-> string conversion
#include <vector>
#include <iostream>
#include <stdexcept>

enum ZOrder
{
    zBall,
    zPads,
    zUI
};

enum PacketType
{
    ptPadPosition,
    ptBallPosition,
    ptScoreUpdate
};


#define PADSIZE 20
#define BALLSIZE 7

class GameWindow : public Gosu::Window
{
protected:
    Gosu::Font font;
    double ballX, ballY;
    double ballXSpeed, ballYSpeed;
    double m_dPlayer1_y, m_dPlayer2_y;
    unsigned player1_score, player2_score;
public:
    GameWindow()
    :   Window(640, 480, false),
        font(graphics(), Gosu::defaultFontName(), 20)
    {
        setCaption(L"Gosu Multiplayer Pong");
        ballX = 640/2;
        ballY = 480/2;
        player1_score = 0;
        player2_score = 0;
        ballXSpeed = 2.0;
        ballYSpeed = 2.0;
    }
    
    virtual void update() = 0;

    void draw()
    {
        graphics().drawLine(10, m_dPlayer1_y - PADSIZE, Gosu::Colors::blue, 10, m_dPlayer1_y + PADSIZE, Gosu::Colors::blue, zPads);
        graphics().drawLine(graphics().width() - 10, m_dPlayer2_y - PADSIZE, Gosu::Colors::red, graphics().width() - 10, m_dPlayer2_y + PADSIZE, Gosu::Colors::red, zPads);
        
        graphics().drawQuad(
           ballX-BALLSIZE, ballY-BALLSIZE, Gosu::Colors::white,
           ballX+BALLSIZE, ballY-BALLSIZE, Gosu::Colors::white,
           ballX+BALLSIZE, ballY+BALLSIZE, Gosu::Colors::white,
           ballX-BALLSIZE, ballY+BALLSIZE, Gosu::Colors::white,
           zBall
        );

        std::wstringstream score;
        score << L"Score: " << player1_score << ":" << player2_score;
        font.draw(score.str(), 10, 10, zUI, 1, 1, Gosu::Colors::yellow);
    }

    void buttonDown(Gosu::Button btn)
    {
        if (btn == Gosu::kbEscape)
           close();
    }
};

class Server : public GameWindow
{
    Gosu::ListenerSocket listenerSocket;
    std::auto_ptr<Gosu::CommSocket> connection;
public:
    Server(uint16_t port)
    :listenerSocket(port)
    {
        listenerSocket.onConnection = std::tr1::bind(&Server::onConnection, this, std::tr1::placeholders::_1);
    }
    
    void onConnection(Gosu::Socket& sock)
    {
        std::cout << "incoming connection" << std::endl;
        if (connection.get()) {
            std::cout << "cannot accept a second client" << std::endl;
            Gosu::CommSocket cs(Gosu::cmManaged, sock);
            cs.disconnect();
            return;
        }
        connection.reset(new Gosu::CommSocket(Gosu::cmManaged, sock));
        connection->onReceive = std::tr1::bind(&Server::onReceive, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2);
        std::cout << "connection accepted" << std::endl;
    }
    
    void update()
    {
        m_dPlayer1_y = input().mouseY();
        listenerSocket.update();
        if (!connection.get()) return;
        if (!connection->connected()) return;
        
        ballX += ballXSpeed;
        ballY += ballYSpeed;
        if (ballX < 0) {
            ballX = graphics().width()/2;
            ballY = graphics().height()/2;
            player2_score++;
        } else if (ballX > graphics().width()) {
            ballX = graphics().width()/2;
            ballY = graphics().height()/2;
            player1_score++;
        }
        if (ballX < 15) {
            if (std::abs(ballY - m_dPlayer1_y) < PADSIZE) {
                ballXSpeed *= -1;
            }
        } else if (ballX > graphics().width() - 15) {
            if (std::abs(ballY - m_dPlayer2_y) < PADSIZE) {
                ballXSpeed *= -1;
            }
        }
        if (ballY < 0) {
            ballYSpeed *= -1;
        } else if (ballY > graphics().height()) {
            ballYSpeed *= -1;
        }
        {
            std::stringstream ss;
            ss << ptPadPosition << " ";
            ss << m_dPlayer1_y;
            std::string data = ss.str();
            connection->send(data.c_str(), data.size());
        }
        {
            std::stringstream ss;
            ss << ptBallPosition << " ";
            ss << ballX << " ";
            ss << ballY;
            std::string data = ss.str();
            connection->send(data.c_str(), data.size());
        }
        {
            std::stringstream ss;
            ss << ptScoreUpdate << " ";
            ss << player1_score << " ";
            ss << player2_score;
            std::string data = ss.str();
            connection->send(data.c_str(), data.size());
        }
        connection->update();
    }
    
    void onReceive(const void* data, std::size_t size)
    {
        std::stringstream stream(std::string(static_cast<const char*>(data), static_cast<const char*>(data)+size));
        int inttype;
        if (!(stream >> inttype)) {
            std::cout << "could not read type from stream" << std::endl;
            std::cout << stream << std::endl;
            return;
        }
        PacketType type = static_cast<PacketType>(inttype);
        switch(type) {
            case ptPadPosition:
                if (!(stream >> m_dPlayer2_y)) {
                    std::cout << "could not read player paddle position" << std::endl;
                    std::cout << stream << std::endl;
                }
            break;
            default:
                std::cout << "invalid packet received" << std::endl;
            break;
        }
    }
};

class Client : public GameWindow
{
    Gosu::CommSocket connection;
public:
    Client(std::string addr, uint16_t port)
    :connection(Gosu::cmManaged, Gosu::stringToAddress(addr), port)
    {
        connection.onReceive = std::tr1::bind(&Client::onReceive, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2);
    }
    
    void update()
    {
        m_dPlayer2_y = input().mouseY();
        std::stringstream ss;
        ss << ptPadPosition << " ";
        ss << m_dPlayer2_y;
        std::string strdata = ss.str();
        connection.send(strdata.c_str(), strdata.size());
        connection.update();
    }

    void onReceive(const void* data, std::size_t size)
    {
        std::stringstream stream(std::string(static_cast<const char*>(data), static_cast<const char*>(data)+size));
        int inttype;
        if (!(stream >> inttype)) {
            std::cout << "could not read type from stream" << std::endl;
            std::cout << stream << std::endl;
            return;
        }
        PacketType type = static_cast<PacketType>(inttype);
        switch(type) {
            case ptPadPosition:
                if (!(stream >> m_dPlayer1_y)) {
                    std::cout << "could not read player1 paddle pos" << std::endl;
                    std::cout << stream << std::endl;
                    return;
                }
            break;
            case ptBallPosition:
                if (!(stream >> ballX)) {
                    std::cout << "could not read ball x pos" << std::endl;
                    std::cout << stream << std::endl;
                    return;
                }
                if (!(stream >> ballY)) {
                    std::cout << "could not read ball y pos" << std::endl;
                    std::cout << stream << std::endl;
                    return;
                }
            break;
            case ptScoreUpdate:
                if (!(stream >> player1_score)) {
                    std::cout << "could not read player1 score" << std::endl;
                    std::cout << stream << std::endl;
                    return;
                }
                if (!(stream >> player2_score)) {
                    std::cout << "could not read player2 score" << std::endl;
                    std::cout << stream << std::endl;
                    return;
                }
            break;
            default:
                std::cout << "unknown packet received" << std::endl;
            break;
        }
    }
};

int main(int argc, char** argv)
{
    std::string addr;
    uint16_t port = 50042;
    if (argc >= 2) {
        addr = argv[1];
    }
    if (argc >= 3) {
        port = atoi(argv[2]);
    }
    if (addr == "") {
        try {
            Server server(port);
            server.show();
            return(0);
        } catch(const std::runtime_error& e) {
            std::cout << e.what() << std::endl;
            addr = "localhost";
        }
    }
    Client client(addr, port);
    client.show();
    return 0;
}
