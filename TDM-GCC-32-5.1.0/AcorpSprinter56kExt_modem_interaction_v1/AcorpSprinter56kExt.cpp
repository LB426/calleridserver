// file AcorpSprinter56kExt.cpp
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
#include "AcorpSprinter56kExt.h"

using namespace std;

namespace AcorpSprinter56kExt{

const std::string AcorpSprinter56kExt::at     = "AT\r\n";
const std::string AcorpSprinter56kExt::atz    = "ATZ\r\n";
const std::string AcorpSprinter56kExt::ate0   = "ATE0\r\n";
const std::string AcorpSprinter56kExt::atvcid = "AT+VCID=1\r\n";

AcorpSprinter56kExt::AcorpSprinter56kExt(std::string serialPortName)
                   : spname(serialPortName),
                     port(io),
                     timer(port.get_io_service()),
                     timeout(500),
                     read_error(true),
                     stopWait(false),
                     timeoutexpired(false)
{
    timeout = 10000;
    cpnum = "undefined";
}

void AcorpSprinter56kExt::init()
{
    try {
        port.open(spname);
        port.set_option(boost::asio::serial_port_base::baud_rate(115200));
        if(&port.is_open){
            std::cout << "Port open\n" ;
            char cc;
            std::string result;
            boost::asio::write(port, boost::asio::buffer(atz.c_str(), atz.size()));
            for(;;)
            {
                boost::asio::read(port,boost::asio::buffer(&cc,1));
                result+=cc;
                if(result.find("OK")!=std::string::npos){
                    std::cout << "ATZ is OK" << std::endl;
                    break;
                }
            }
            result = "";
            boost::asio::write(port, boost::asio::buffer(ate0.c_str(), ate0.size()));
            for(;;)
            {
                boost::asio::read(port,boost::asio::buffer(&cc,1));
                result+=cc;
                if(result.find("OK")!=std::string::npos){
                    std::cout << "ATE0 is OK" << std::endl;
                    break;
                }
            }
            result = "";
            boost::asio::write(port, boost::asio::buffer(atvcid.c_str(), atvcid.size()));
            for(;;)
            {
                boost::asio::read(port,boost::asio::buffer(&cc,1));
                result+=cc;
                if(result.find("OK")!=std::string::npos){
                    std::cout << "AT+VCID=1 is OK" << std::endl;
                    break;
                }
            }
            result = "";
        }else{
            std::cout << "Port NO open\n" ;
        }
    } catch(boost::system::system_error& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        stopWait = true;
    }    
}

void AcorpSprinter56kExt::read_complete(const boost::system::error_code &error,
                   size_t bytes_transferred)
{     
    read_error = (error || bytes_transferred == 0);
    timer.cancel();
}

void AcorpSprinter56kExt::time_out(const boost::system::error_code &error)
{
    if (error){
        return;
    }
    port.cancel();
    timeoutexpired = true;
}

bool AcorpSprinter56kExt::read_char(char &val)
{
    //val = c = '\0';
    port.get_io_service().reset();
    boost::asio::async_read(port, boost::asio::buffer(&c, 1), 
                            boost::bind(&AcorpSprinter56kExt::read_complete, 
                            this, 
                            boost::asio::placeholders::error, 
                            boost::asio::placeholders::bytes_transferred)); 
    timer.expires_from_now(boost::posix_time::milliseconds(timeout));
    timer.async_wait(boost::bind(&AcorpSprinter56kExt::time_out,
                                 this,
                                 boost::asio::placeholders::error));
    port.get_io_service().run();
    //if (!read_error)
    //    val = c;
    return !read_error;
}

void AcorpSprinter56kExt::WaitPhoneCall()
{
    cout << "WaitPhoneCall on port: " << spname << endl;
    std::string rsp;
    for(;;)
    {
        if(stopWait) break;
        while (read_char(c)){
            rsp+=c;
            
            /* отладка */
            /*
            std::string stmp = rsp;
            int code = (int)c;
            switch(c)
            {
                case '\r':
                    boost::replace_all(stmp, "\r", "\\r");
                    boost::replace_all(stmp, "\n", "\\n");
                    boost::replace_all(stmp, "\0", "\\0");
                    std::cout << "inc buf size: " << rsp.size() << "cur char: " << "\\r char int val: " << code << " str: \"" << stmp << "\"" << std::endl;
                    break;
                case '\n':
                    boost::replace_all(stmp, "\r", "\\r");
                    boost::replace_all(stmp, "\n", "\\n");
                    boost::replace_all(stmp, "\0", "\\0");
                    std::cout << "inc buf size: " << rsp.size() << "cur char: " << "\\n char int val: " << code << " str: \"" << stmp << "\"" << std::endl;
                    break;
                case '\0':
                    boost::replace_all(stmp, "\r", "\\r");
                    boost::replace_all(stmp, "\n", "\\n");
                    boost::replace_all(stmp, "\0", "\\0");
                    std::cout << "inc buf size: " << rsp.size() << "cur char: " << "ZERO char int val: " << code << " str: \"" << stmp << "\"" << std::endl;
                    break;
                default:
                    boost::replace_all(stmp, "\r", "\\r");
                    boost::replace_all(stmp, "\n", "\\n");
                    boost::replace_all(stmp, "\0", "\\0");
                    std::cout << "inc buf size: " << rsp.size() << "cur char: " << c << " char int val: " << code << " str: \"" << stmp << "\"" << std::endl;
            }
            stmp = "";
            */
            /* конец отладки */
            
            if(rsp.find("NMBR = ")!=std::string::npos){
                bool flag = true;
                int len = rsp.size();
                char last = rsp.at(len-1);
                if((flag==true)&&(last=='\r')){
                    flag = false;
                    std::string myStr = rsp;
                    rsp = "";
                    boost::smatch xResults;
                    boost::regex myRegEx(".*(NMBR\\s=\\s)(\\d+).*");
                    if(boost::regex_match(myStr,xResults,myRegEx)){
                        boost::posix_time::ptime now2 = boost::posix_time::second_clock::local_time();
                        std::stringstream ss2;
                        ss2 << now2.date().year() << "-" << now2.date().month() << "-" << now2.date().day() << " " << now2.time_of_day();
                        cpnum = xResults[2];
                        std::cout << ss2.str() << "\tNUMBER: \"" << cpnum << "\"" << std::endl;
                    }else{
                        std::cout << "no match in regexp, test line: \"" << myStr << "\"" << std::endl;
                    }
                }
            }
        }
        if(timeoutexpired){
            //std::cout << "timeout expired, timeout value: " << timeout << std::endl;
            timeoutexpired = false;
        }
    }
}

} //end namespace AcorpSprinter56kExt