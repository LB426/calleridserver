/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

// file serialmodem.cpp
#include <iostream>
#include <exception>
#include <string>
#include <boost/locale.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port_base.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "serialmodem.h"

using namespace std;

void logMsg(string message);

namespace SerialModem {

  const std::string SerialModem::at = "AT\r\n";
  const std::string SerialModem::atz = "ATZ\r\n";
  const std::string SerialModem::ate0 = "ATE0\r\n";
  const std::string SerialModem::atvcid = "AT+VCID=1\r\n";

  SerialModem::SerialModem(std::string serialPortName)
             : stopwait(false),
             debug(false),
  spname(serialPortName),
  port(io),
  timer(port.get_io_service()),
  timeout(500),
  read_error(true),
  timeoutexpired(false) 
  {
    timeout = 10000;
    phonenum = "undefined";
    nmbr_counter = 0;
    ring_counter = 0;
    before_ring_str = "";
  }

  bool SerialModem::init() {
    try {
      port.open(spname);
      port.set_option(boost::asio::serial_port_base::baud_rate(115200));
      if (&port.is_open) {
        {
          std::stringstream ss;
          ss << "Port open: " << spname;
          logMsg(ss.str());
        }

        char cc;
        std::string result;
        boost::asio::write(port, boost::asio::buffer(atz.c_str(), atz.size()));
        for (;;) {
          boost::asio::read(port, boost::asio::buffer(&cc, 1));
          result += cc;
          if (result.find("OK") != std::string::npos) {
            logMsg("ATZ is OK");
            break;
          }
        }
        result = "";
        boost::asio::write(port, boost::asio::buffer(ate0.c_str(), ate0.size()));
        for (;;) {
          boost::asio::read(port, boost::asio::buffer(&cc, 1));
          result += cc;
          if (result.find("OK") != std::string::npos) {
            logMsg("ATE0 is OK");
            break;
          }
        }
        result = "";
        boost::asio::write(port, boost::asio::buffer(atvcid.c_str(), atvcid.size()));
        for (;;) {
          boost::asio::read(port, boost::asio::buffer(&cc, 1));
          result += cc;
          if (result.find("OK") != std::string::npos) {
            logMsg("AT+VCID=1 is OK");
            break;
          }
        }
      } else {
        std::stringstream ss;
        ss << "Port NO open: " << spname;
        logMsg(ss.str());
        return false;
      }
    }    catch (boost::system::system_error& e) {
      stopwait = true;
      std::stringstream ss;
      ss << "Exception Error: " << e.what() << ", serial port: " << spname << " завершаю поток этого порта.";
      logMsg(ss.str());
      return false;
    }
    return true;
  }

  void SerialModem::read_complete(const boost::system::error_code &error,
          size_t bytes_transferred) {
    read_error = (error || bytes_transferred == 0);
    timer.cancel();
  }

  void SerialModem::time_out(const boost::system::error_code &error) {
    if (error) {
      return;
    }
    port.cancel();
    timeoutexpired = true;
  }

  bool SerialModem::read_char(char &val) {
    //val = c = '\0';
    port.get_io_service().reset();
    boost::asio::async_read(port, boost::asio::buffer(&c, 1),
            boost::bind(&SerialModem::read_complete,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    timer.expires_from_now(boost::posix_time::milliseconds(timeout));
    timer.async_wait(boost::bind(&SerialModem::time_out,
            this,
            boost::asio::placeholders::error));
    port.get_io_service().run();
    //if (!read_error)
    //    val = c;
    return !read_error;
  }

  std::string SerialModem::MdmLineToHumanString(std::string mline) {
    boost::replace_all(mline, "\t", "\\t");
    boost::replace_all(mline, "\r", "\\r");
    boost::replace_all(mline, "\n", "\\n");
    boost::replace_all(mline, "\0", "\\0");
    return mline;
  }

  void SerialModem::WaitPhoneCall() {
    {
      std::stringstream ss;
      ss << "WaitPhoneCall on port: " << spname;
      logMsg(ss.str());
    }
    ring_counter = 0;
    std::string rsp;
    for (;;) {
      if (stopwait) break;
      while (read_char(c)) {
        rsp += c;

        /* Р?С'Р>Р°Р?РєР° */
        if (debug) {
          std::string stmp = rsp;
          int code = (int) c;
          switch (c) {
            case '\r':
              std::cout << "inc buf size: " << rsp.size() << " cur char: " << "\\r char int val: " << code << " str: \"" << MdmLineToHumanString(stmp) << "\"" << std::endl;
              break;
            case '\n':
              std::cout << "inc buf size: " << rsp.size() << " cur char: " << "\\n char int val: " << code << " str: \"" << MdmLineToHumanString(stmp) << "\"" << std::endl;
              break;
            case '\0':
              std::cout << "inc buf size: " << rsp.size() << " cur char: " << "ZERO char int val: " << code << " str: \"" << MdmLineToHumanString(stmp) << "\"" << std::endl;
              break;
            default:
              std::cout << "inc buf size: " << rsp.size() << " cur char: '" << c << "' char int val: " << code << " str: \"" << MdmLineToHumanString(stmp) << "\"" << std::endl;
          }
          stmp = "";
        }

        /* РєР?Р?РчС┼ Р?С'Р>Р°Р?РєРё */

        boost::smatch xResults;
        boost::regex myRegEx(".*(NMBR\\s+=\\s+)(\\d{0,})\r.*");
        if (boost::regex_match(rsp, xResults, myRegEx)) {
          phonenum = xResults[2];
          std::stringstream ss;
          ss << "INCOMING CALL NMBR: " << phonenum;
          logMsg(ss.str());
          return;
          /* прерываем цикл ожидание номера для отправки номера, 
           * цикл возобновиться извне, 
           * поэтому rsp строка накопитель не очищается
           */
        }
      }
      if (timeoutexpired) {
        std::stringstream ss;
        ss << "WaitCALL timeout expired, timeout value: " << timeout << " timeoutexpired: " << timeoutexpired;
        //logMsg(ss.str());
        timeoutexpired = false;
        ring_counter = 0;
      }
    }
  }

} //end namespace SerialModem