/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   serialmodem.h
 * Author: Andrey.Seredin
 *
 * Created on May 23, 2016, 4:32 PM
 */

#ifndef SERIALMODEM_H
#define SERIALMODEM_H

// file serialmodem.h

#include <boost/asio.hpp>
#include <boost/asio/serial_port_base.hpp>

namespace SerialModem {

class SerialModem {
public:
    SerialModem(std::string serialPortName);
    SerialModem();
    void WaitPhoneCall();
    bool init();
    bool read_char(char &val);
    std::string phonenum;
    bool stopwait;
    bool debug;
private:
    std::string spname;
    boost::asio::io_service io;
    boost::asio::serial_port port;
    boost::asio::deadline_timer timer;
    size_t timeout;
    bool read_error;
    char c;
    static const std::string at;
    static const std::string atz;
    static const std::string ate0;
    static const std::string atvcid;
    bool timeoutexpired;
    int ring_counter;
    int nmbr_counter;
    std::string before_ring_str;
    void read_complete(const boost::system::error_code& error, 
                       size_t bytes_transferred);
    void time_out(const boost::system::error_code& error);
    std::string MdmLineToHumanString(std::string mline);
};

} // end namespace SerialModem


#endif /* SERIALMODEM_H */

