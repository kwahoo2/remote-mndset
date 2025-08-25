// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */

#ifndef DATA_SENDER_H
#define DATA_SENDER_H

#include "structs.h"

class DataSender {
    int sockfd;
public:
    DataSender();
    ~DataSender();
    int openSocket(void);
    bool isSocketOpened(void);
    int sendData(const r_remote_data &data);
    void closeSocket(void);
};


#endif // DATA_SENDER_H
