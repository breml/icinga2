#include "i2-jsonrpc.h"

using namespace icinga;

void ConnectionManager::BindServer(JsonRpcServer::RefType server)
{
	m_Servers.push_front(server);
	server->OnNewClient.bind(bind_weak(&ConnectionManager::NewClientHandler, shared_from_this()));
}

void ConnectionManager::UnbindServer(JsonRpcServer::RefType server)
{
	m_Servers.remove(server);
	// TODO: unbind event
}

void ConnectionManager::BindClient(JsonRpcClient::RefType client)
{
	m_Clients.push_front(client);
	client->OnNewMessage.bind(bind_weak(&ConnectionManager::NewMessageHandler, shared_from_this()));
}

void ConnectionManager::UnbindClient(JsonRpcClient::RefType client)
{
	m_Clients.remove(client);
	// TODO: unbind event
}

int ConnectionManager::NewClientHandler(NewClientEventArgs::RefType ncea)
{
	JsonRpcClient::RefType client = static_pointer_cast<JsonRpcClient>(ncea->Client);
	BindClient(client);

	return 0;
}

int ConnectionManager::CloseClientHandler(EventArgs::RefType ea)
{
	UnbindClient(static_pointer_cast<JsonRpcClient>(ea->Source));

	return 0;
}

int ConnectionManager::NewMessageHandler(NewMessageEventArgs::RefType nmea)
{
	OnNewMessage(nmea);

	return 0;
}
