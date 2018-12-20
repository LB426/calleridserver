// file AcorpSprinter56kExt.h

#include <boost/asio.hpp>
#include <boost/asio/serial_port_base.hpp>

namespace AcorpSprinter56kExt {

class AcorpSprinter56kExt {
public:
    AcorpSprinter56kExt(std::string serialPortName);
    AcorpSprinter56kExt();
    void WaitPhoneCall();
    void init();
    bool read_char(char &val);
    std::string cpnum;
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
    bool stopWait;
    bool timeoutexpired;
    void read_complete(const boost::system::error_code& error, 
                       size_t bytes_transferred);
    void time_out(const boost::system::error_code& error);
};

} // end namespace AcorpSprinter56kExt