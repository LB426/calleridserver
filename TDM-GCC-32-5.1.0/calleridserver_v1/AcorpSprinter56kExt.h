// file AcorpSprinter56kExt.h

#include <boost/asio.hpp>
#include <boost/asio/serial_port_base.hpp>

namespace AcorpSprinter56kExt {

class AcorpSprinter56kExt {
public:
    AcorpSprinter56kExt(std::string serialPortName);
    AcorpSprinter56kExt();
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

} // end namespace AcorpSprinter56kExt