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

	SOCKET_TYPE socket_type_;

	// UDP
	sockaddr_in clientAddr_;        // UDP�� ���� Ŭ���̾�Ʈ �ּ� ����
	int clientAddrLen_;             // Ŭ���̾�Ʈ �ּ� ����
	int sequenceNumber_;            // UDP ��Ŷ ���� ��� ���� ������ ��ȣ

	// ai_target_c_id_ : AI�� �����ϴ� ����� client id
	int ai_target_c_id_;

    Over_IO()
        : ai_target_c_id_(-1), clientAddrLen_(sizeof(clientAddr_)), sequenceNumber_(0) {
        wsabuf_.buf = send_buf_;
        wsabuf_.len = BUF_SIZE;
        io_key_ = IO_RECV;
        ZeroMemory(&over_, sizeof(over_));
        ZeroMemory(&clientAddr_, sizeof(clientAddr_));
    }

    Over_IO(unsigned char* packet, SOCKET_TYPE socketType)
        : ai_target_c_id_(-1), clientAddrLen_(sizeof(clientAddr_)), sequenceNumber_(0), socket_type_(socketType) {
        wsabuf_.len = packet[0];
        wsabuf_.buf = send_buf_;
        ZeroMemory(&over_, sizeof(over_));
        io_key_ = IO_SEND;
        memcpy(send_buf_, packet, wsabuf_.len);
        ZeroMemory(&clientAddr_, sizeof(clientAddr_));
    }
};