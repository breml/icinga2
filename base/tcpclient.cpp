#include "i2-base.h"

using namespace icinga;

TCPClient::TCPClient(void)
{
	m_SendQueue = new_object<FIFO>();
	m_RecvQueue = new_object<FIFO>();
}

void TCPClient::Start(void)
{
	TCPSocket::Start();

	function<int (EventArgs::RefType)> rd = bind_weak(&TCPClient::ReadableEventHandler, shared_from_this());
	OnReadable.bind(rd);

	function<int (EventArgs::RefType)> wd = bind_weak(&TCPClient::WritableEventHandler, shared_from_this());
	OnWritable.bind(wd);
}

FIFO::RefType TCPClient::GetSendQueue(void)
{
	return m_SendQueue;
}

FIFO::RefType TCPClient::GetRecvQueue(void)
{
	return m_RecvQueue;
}

int TCPClient::ReadableEventHandler(EventArgs::RefType ea)
{
	char buffer[4096];
	int rc;

	rc = recv(GetFD(), buffer, sizeof(buffer), 0);

	if (rc <= 0) {
		Close();
		return 0;
	}

	m_RecvQueue->Write(buffer, rc);

	EventArgs::RefType dea = new_object<EventArgs>();
	dea->Source = shared_from_this();
	OnDataAvailable(dea);

	return 0;
}

int TCPClient::WritableEventHandler(EventArgs::RefType ea)
{
	int rc;

	rc = send(GetFD(), (const char *)m_SendQueue->Peek(), m_SendQueue->GetSize(), 0);

	if (rc <= 0) {
		Close();
		return 0;
	}

	m_SendQueue->Read(NULL, rc);

	return 0;
}

bool TCPClient::WantsToRead(void) const
{
	return true;
}

bool TCPClient::WantsToWrite(void) const
{
	return (m_SendQueue->GetSize() > 0);
}
