#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "AcorpSprinter56kExt.h"

using namespace std;
using boost::asio::ip::tcp;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

class myThreadInfo 
{
public:
    myThreadInfo();
    unsigned long id; // ��������� ID ������
    boost::interprocess::interprocess_semaphore sem_1; // ���������� ��������� ������
    boost::interprocess::interprocess_semaphore sem_2; // ������� �� �������� ������
    boost::interprocess::interprocess_semaphore sem_3; // ������������� �������� ������
    void setDataForTransfer(std::string incdata);
    std::string getDataForTransfer();
    bool thread_completed;    
private:
    boost::mutex mtx_;
    std::string localdata_;
    void setThreadId();
};
myThreadInfo::myThreadInfo()
            :sem_1(0),
             sem_2(0),
             sem_3(0),
             localdata_("undefined class constructor"),
             thread_completed(false)
{
    setThreadId();
}
void myThreadInfo::setDataForTransfer(std::string incdata)
{
    boost::lock_guard<boost::mutex> guard(mtx_);
    localdata_ = incdata;
}
std::string myThreadInfo::getDataForTransfer()
{
    boost::lock_guard<boost::mutex> guard(mtx_);
    return localdata_;
}
void myThreadInfo::setThreadId()
{
    std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
    unsigned long threadNumber = 0;
    sscanf(threadId.c_str(), "%lx", &threadNumber);
    id = threadNumber;
}

class chanParam
{
public:
    chanParam();
    std::string channame;
    std::string serialport;
    bool debug;
};
chanParam::chanParam() : channame("undefined"),serialport("undefined"),debug(false) {}

vector<myThreadInfo*> rdrs(0); // ������ ������� ������� - ����� �������� ����� ������� ���� � ���� ����

boost::mutex mtx_rdrs;    // ����� ������ �������
boost::mutex mtx_cout;    // ����� ������� ��� ������ ����
int dscount = 0;          // ���������� ������� ���������� ������
boost::mutex mtx_dscount; // ����� ���������� ������� ���������� ������
const int max_length = 1024;
boost::interprocess::interprocess_semaphore gsem_1(0);

void logMsg(string message)
{
    boost::lock_guard<boost::mutex> guard(mtx_cout);
    //boost::posix_time::ptime now = boost::posix_time::second_clock::local_time(); // ����� ����� ��� �����������
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time(); // ����� � ��������������
    boost::posix_time::time_duration duration( now.time_of_day() );
    std::stringstream ss2;
    ss2 << now.date().year() << "-" << now.date().month() << "-" << now.date().day() << " " << now.time_of_day();
    string curdate = ss2.str();
    cout << curdate << " " << message << endl;
}

void readerCheckAlive()
{
    int sz = rdrs.size();
    for(int j=0; j < sz; j++) // �������� ��� �� �����
    {
        if(rdrs[j] == NULL)
        {
            rdrs.erase(rdrs.begin()+j);
            std::stringstream ss;
            ss << "��������� ���������� ����� ������, NULL ptr, rdrs.size ����� ����������: " << rdrs.size();               
            logMsg(ss.str());
            break;
        }   
    }
}

void writer(chanParam param)
{
    std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
    unsigned long threadNumber = 0;
    sscanf(threadId.c_str(), "%lx", &threadNumber);
    
    {
        std::stringstream ss;                                                                                                                  
        ss << "======== ������������ ������ �: " << threadNumber << " �������.";
        logMsg(ss.str());                                                                                                                      
    }
    
    AcorpSprinter56kExt::AcorpSprinter56kExt mdm(param.serialport);
    mdm.debug = param.debug;
    if(!mdm.init()){
        std::stringstream ss;                                                   
        ss << "����� �� ���������������. ����� ������������ �: " << threadNumber;
        logMsg(ss.str());                                                       
        return;
    }
    
    try
    {
        for(int i=1;;i++)
        {
            if(rdrs.size()==0)
            {
                std::stringstream ss;                                                                                     
                ss << "������������ �: " << threadNumber << ", ��� ������� ������� � �������� ������, ��� 5 ���., ������ ������� �������: " << rdrs.size() << ", iteration: " << i;
                logMsg(ss.str());
                boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());
                bool ret = gsem_1.timed_wait(t + boost::posix_time::seconds(5));
            }
            else
            {
                mdm.WaitPhoneCall();
                std::string incnmbr = mdm.phonenum;
                std::string channel = param.channame;
                std::stringstream dfs;
                dfs << "DAT" << incnmbr << ";" << channel;
                std::string sendstr = dfs.str();
                
                std::stringstream ss;
                ss << "������������ �: " << threadNumber << ", iteration: " << i << ", ������: " << sendstr;
                string localdata = ss.str();

                int rrc = 0;        // ������� ������� ������� � ��������
                int rsc = 0;        // ������� ������� ����������� ��������
                mtx_rdrs.lock();    // �������� ������ � ������� ������� ������������
                for(;;)             // �������� ���������� ������� ��������� ������                                                      
                {                                                                                                                     
                    boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());
                    for(int j=0; j < rdrs.size(); j++)                                                                                
                    {                                                                                                                 
                        bool ret = rdrs[j]->sem_1.timed_wait(t + boost::posix_time::seconds(10));                                      
                        if(ret)
                        {                                                                                                             
                            rrc++;                                                                                                    
                        }
                        else
                        {
                            // ���� �� �������� ���������� ����������� � ������� 10 ������
                            // ������� �� ������� ������� ��������� �� ����� �������� �� ���������
                            // ������� ������ �� ��������� �������� ���� catch ������ ������
                            rdrs.erase(rdrs.begin()+j);
                            // ����� � ���
                            std::stringstream ss;                                                                                    
                            ss << "������������ �: " << threadNumber<< ", ��� ���������� 1 ��� ������ " << rdrs[j]->id << ", ������� ������� �������: " << rrc;             
                            logMsg(ss.str());
                            break;
                        }
                    }
                    if((rrc==rdrs.size())&&(rrc!=0)) // ���� ���������� �������� ����� ���������� � ���������� �� ������� ��������� ���� ������������ ��������
                    {
                        std::stringstream ss;                                                                                        
                        ss << "������������ �: " << threadNumber << ", ���� ���������� 1, ������� ������� �������: " << rrc << ", ������ ������� �������: " << rdrs.size();  
                        logMsg(ss.str());                                                                                            
                        break;                                                                                                        
                    }                                                                                                                 
                }                                                                                                                     
                logMsg("������ ������ � ��������.");                                                                                   
                for(int j=0; j < rdrs.size(); j++)            // ������� ������ � ����� ������                                         
                {                                                                                                                      
                    rdrs[j]->setDataForTransfer(sendstr);
                }
                for(int j=0; j < rdrs.size(); j++)            // ������� ������� �� �������� ������                                    
                {                                                                                                                      
                    rdrs[j]->sem_2.post();                                                                                             
                }                                                                                                                      
                for(;;) // �������� ������������ �������� �� �������                                                                   
                {                                                                                                                      
                    boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());                                   
                    readerCheckAlive();
                    for(int j=0; j < rdrs.size(); j++)                                                                                 
                    {                                                                                                                  
                        bool ret = rdrs[j]->sem_3.timed_wait(t + boost::posix_time::seconds(10));                                       
                        if(ret)                                                                                                        
                        {                                                                                                              
                            rsc++;                                                                                                     
                        }                                                                                                              
                        else                                                                                                           
                        {
                            rdrs.erase(rdrs.begin()+j);
                            std::stringstream ss;                                                                                     
                            ss << "������������ �: " << threadNumber  << ", ��� ���������� 2 ��� ������ " << rdrs[j]->id << " ������� ������� �������: " << rsc;               
                            logMsg(ss.str());                                                                                         
                            break;
                        }
                    }                                                                                                                  
                    if(rsc==rdrs.size())                                                                                               
                    {                                                                                                                  
                        std::stringstream ss;                                                                                         
                        ss << "������������ �: " << threadNumber << ", ���� ���������� 2, ������� ������� �������: " << rsc << " ������ ������� �������: " << rdrs.size();    
                        logMsg(ss.str());                                                                                             
                        break;                                                                                                         
                    }                                                                                                                  
                }                                                                                                                      
                mtx_rdrs.unlock();                                                                                                     
            }
        }
        
        {
            std::stringstream ss;                                                                                                                  
            ss << "======== ������������ ������ �: " << threadNumber << " ����������.";
            logMsg(ss.str());                                                                                                                      
        }
    }
    catch (std::exception& e)
    //catch (...)
    {
        std::stringstream ss;
        ss << "Exception: " << e.what() << ", ������������ thread �: " << threadNumber;
        logMsg(ss.str());
        //std::cout << "Exception in thread ������������: " << threadNumber << " error: " << boost::current_exception_diagnostic_information() << std::endl;
    }
}

void session(socket_ptr sock)
{
    myThreadInfo *mtPtr = new myThreadInfo();
    mtx_rdrs.lock();    
    rdrs.push_back(mtPtr);
    mtx_rdrs.unlock();  
    gsem_1.post();    
    try
    {
        for (int i=1;;i++)
        {
            mtPtr->sem_1.post(); // ������� � ���������� ������ � ��������    
            mtPtr->sem_2.wait(); // ��� ������� �� ��������               
            
            string localdata = mtPtr->getDataForTransfer();
            if(localdata != "undefined - clear from reader thread")
            {
                std::stringstream ss;
                ss << "";
                //=======================================================================
                // ��� ������������ ����� ������:
                /*
                mtx_dscount.lock();                                                                                                                                   
                dscount++;
                ss << "��������. �����: " << mtPtr->id << ", iteration: " << i << ", ������: \"" << localdata << "\", ���������� ������� ���������� ������: " << dscount;   
                localdata = ss.str();
                localdata = localdata + "\r\n";
                mtx_dscount.unlock();
                */
                // ����� ����� ��� ������������ ����� ������
                //=======================================================================
                // ��� ���������� ��������� ������ ���������������� ���� ������������
                
                char buffer[max_length] = {0};
                boost::system::error_code error;
                std::size_t length = localdata.copy(buffer,localdata.size(),0);
                boost::asio::write(*sock, boost::asio::buffer(buffer, sizeof(buffer)), error);
                if (error == boost::asio::error::eof)
                {
                    std::stringstream ss;
                    ss << "boost::asio::error::eof";
                    logMsg(ss.str());
                    break; // Connection closed cleanly by peer.
                }
                else if (error)
                    throw boost::system::system_error(error);
                
                ss << "�������� ���������. �����: " << mtPtr->id << ", iteration: " << i << ", ������: \"" << localdata << "\", ���������� ������� ���������� ������: " << dscount;
                logMsg(ss.str());                                                                                                                                         
                mtPtr->setDataForTransfer("undefined - clear from reader thread");                                                                                            
            }    
            mtPtr->sem_3.post(); // ������� � ���������� ��������
        }
        delete mtPtr;
        mtPtr = NULL;
    }
    catch (std::exception& e)
    {
        delete mtPtr;
        mtPtr = NULL;
        std::stringstream ss;
        ss << "Exception in thread tcp client session: " << e.what();
        logMsg(ss.str());
    }
}

void acc_inc_con(unsigned short port) // ���� �������� ���������� �� TCP/IP
{
    boost::asio::io_service io_service;
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
    for (;;)
    {
        socket_ptr sock(new tcp::socket(io_service));
        a.accept(*sock);
        boost::thread t(boost::bind(session, sock));
    }
}

bool global_init()
{
    try
    {
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("config.ini", pt);

    string listenport = pt.get<std::string>("tcpserver.listenport");
    boost::thread listener(boost::bind(acc_inc_con, atoi(listenport.c_str())));
    
    // ��������� ������ ������������ �������
    int chancounter = 0;
    for(boost::property_tree::ptree::iterator it1 = pt.begin(); it1 != pt.end(); it1++)
    {
        string sectionname = it1->first.data();
        if(sectionname.find("channel") != std::string::npos)
        {
            chancounter++;
            std::ostringstream ss;
            ss << chancounter;
            string c = ss.str();
            string s = "channel" + c;
            string sp = s + ".serialport";
            string serialport = pt.get<std::string>(sp);
            string cn = s + ".channame";
            string channame = pt.get<std::string>(cn);
            string dbg = s + ".debug";
            string debug = pt.get<std::string>(dbg);
            chanParam p;
            p.channame = channame;
            p.serialport = serialport;
            if(debug == "1")
                p.debug = true;
            boost::thread callerid(boost::bind(writer, p));
        }
    }
    }
    catch(std::exception& e)
    {
        std::stringstream ss;                                        
        ss << "Exception in global_init: " << e.what();
        logMsg(ss.str());
        return false;
    }
    return true;    
}

int main(int argc, char* argv[])
{
    logMsg("enter char:\r\nr - �������� ������ ������.\r\ns - ������ ������� � ����� � �������\r\ne - ��������� ���������");
    try
    {
        if(!global_init())
        {
            logMsg("�� ��������� ������������� �������. ��������� ��������.");                              
            return 1;
        }
        
        for(;;)
        {
            char c;
            cin >> c;
            switch(c)
            {
                case 'e':
                    logMsg("exit.");
                    return 0;
                case 'r':
                    {
                        mtx_rdrs.lock();
                        for(int i=0; i < rdrs.size(); i++)
                        {
                            std::stringstream ss;
                            ss << "rdrs[" << i << "]= " << rdrs[i]->id << " data: " << rdrs[i]->getDataForTransfer();
                            logMsg(ss.str());
                            rdrs[i]->sem_2.post();
                        }
                        mtx_rdrs.unlock();
                    }
                    break;
                case 's':
                    {
                        std::stringstream ss;
                        ss << "rdrs.size: " << rdrs.size();
                        logMsg(ss.str());
                    }
                    break;
                default:
                    logMsg("enter char:\r\nr - �������� ������� �������.\r\ns - ������ ������� � ����� � �������\r\ne - ��������� ���������");
            }
        }
    }
    catch (...)
    {
        std::stringstream ss;
        ss << "���������� ����������� � main(): " << boost::current_exception_diagnostic_information() << std::endl;
        logMsg(ss.str());
    }

    return 0;
}