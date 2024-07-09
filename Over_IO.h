#pragma once
#include "global.h"
#include "protocol.h"


class Over_IO
{
public:
	WSAOVERLAPPED over_;
	WSABUF wsabuf_;
	char send_buf_[BUF_SIZE];
	IO_TYPE io_key_;

	// ai_target_c_id_ : AI가 추적하는 대상의 client id
	int ai_target_c_id_;

	Over_IO()
	{
		wsabuf_.buf = send_buf_;
		wsabuf_.len = BUF_SIZE;
		io_key_ = IO_RECV;
		ZeroMemory(&over_, sizeof(over_));
	}
	Over_IO(char* packet)
	{
		wsabuf_.len = packet[0];
		wsabuf_.buf = send_buf_;
		ZeroMemory(&over_, sizeof(over_));
		io_key_ = IO_SEND;
		memcpy(send_buf_, packet, wsabuf_.len);
	}
};