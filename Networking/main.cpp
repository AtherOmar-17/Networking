#include <iostream>
#include <enet/enet.h>

ENetAddress address;
ENetHost* host;
ENetPeer* peer = nullptr;

bool CreateServer()
{
	address.host = ENET_HOST_ANY;
	address.port = 1234;
	host = enet_host_create(&address, 32, 2, 0, 0);

	if (host == nullptr)
	{
		return false;
	}

	std::cout << "Server Created!" << std::endl;
	return true;
}

bool CreateClient()
{
	host = enet_host_create(NULL, 1, 2, 0, 0);

	if (host == nullptr)
	{
		return false;
	}

	std::cout << "Client Created!" << std::endl;
	return true;
}

class NetworkMessage
{
public:

	NetworkMessage(int const& type)
		: m_type(type)
	{

	}

	virtual ~NetworkMessage()
	{

	}

	int GetType() { return m_type; }

private:

	int m_type;
	bool enable;
	bool active;
	bool received;
};

class PositionMessage : public NetworkMessage
{
public:

	PositionMessage(float const& x, float const& y)
		: NetworkMessage(0)
		, m_x(x)
		, m_y(y)
	{
	}

	float GetX() { return m_x; }
	float GetY() { return m_y; }

private:

	float m_x;
	float m_y;
};

class StringMessage : public NetworkMessage
{
public:

	StringMessage(const char* myMessage)
		: NetworkMessage(1)
	{
	}
};



int main(int argc, char** argv)
{
	if (enet_initialize() != 0)
	{
		std::cout << "error initializing enet" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "ENet Initialized!" << std::endl;

	std::cout << "1) Create Server" << std::endl;
	std::cout << "2) Create Client" << std::endl;
	int userInput;
	std::cin >> userInput;

	if (userInput == 1)
	{
		if (!CreateServer())
		{
			std::cout << "server creation failed" << std::endl;
			return EXIT_FAILURE;
		}

		while (1)
		{
			ENetEvent event;
			ENetPacket* packet;

			if (peer)
			{
				StringMessage message("My Message");
				
				packet = enet_packet_create(&message, sizeof(StringMessage), ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(peer, 0, packet);
				enet_host_flush(host);
			}

			while (enet_host_service(host, &event, 1000) > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:

					std::cout << "A new client connected from" << std::endl << event.peer->address.host << ":" << event.peer->address.port << std::endl;
					event.peer->data = (void*)("Client information");
					peer = event.peer;
					break;
				case ENET_EVENT_TYPE_RECEIVE:

					std::cout << "A packet of length " << event.packet->dataLength << " containing " << event.packet->data << " was received from ";
					std::cout << event.peer->data << " on channel " << event.channelID << std::endl;

					enet_packet_destroy(event.packet);
					break;

				case ENET_EVENT_TYPE_DISCONNECT:

					std::cout << event.peer->data << " disconnected." << std::endl;
					event.peer->data = nullptr;
					break;
				}
			}
		}
	}
	else
	{
		if (!CreateClient())
		{
			std::cout << "client creation failed" << std::endl;
			return EXIT_FAILURE;
		}

		ENetEvent event;
		ENetPeer* peer;
		enet_address_set_host(&address, "127.0.0.1");
		address.port = 1234;
		peer = enet_host_connect(host, &address, 2, 0);
		if (peer == nullptr)
		{
			std::cout << "peer connection failed" << std::endl;
			return EXIT_FAILURE;
		}

		if (enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
		{
			std::cout << "Connected to server." << std::endl;
		}
		else
		{
			enet_peer_reset(peer);
			std::cout << "Connection to server failed." << std::endl;
		}

		while (1)
		{
			while (enet_host_service(host, &event, 1000) > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_RECEIVE:

					NetworkMessage* message = (NetworkMessage*)event.packet->data;

					switch (message->GetType())
					{
					case 0:
					{
						PositionMessage* posMessage = (PositionMessage*)event.packet->data;
						std::cout << "A packet of length " << event.packet->dataLength << " containing " << posMessage->GetX() << "," << posMessage->GetY() << std::endl;
					}
						break;

					default:
						std::cout << "Unknown message received of type " << message->GetType() << " A packet of length " << event.packet->dataLength << std::endl;
					}

					break;
				}
			}
		}
	}

	enet_host_destroy(host);
}