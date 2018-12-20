/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Andrey.Seredin
 *
 * Created on May 23, 2016, 4:27 PM
 */

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "serialmodem.h"

using namespace std;
using boost::asio::ip::tcp;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

class myThreadInfo {
public:
  myThreadInfo();
  unsigned long id; // системный ID потока
  boost::interprocess::interprocess_semaphore sem_1; // готовность принимать данные
  boost::interprocess::interprocess_semaphore sem_2; // команда на передачу данных
  boost::interprocess::interprocess_semaphore sem_3; // подтверждение передачи данных
  void setDataForTransfer(std::string incdata);
  std::string getDataForTransfer();
  bool thread_completed;
private:
  boost::mutex mtx_;
  std::string localdata_;
  void setThreadId();
};

myThreadInfo::myThreadInfo()
            : sem_1(0),
              sem_2(0),
              sem_3(0),
              localdata_("undefined class constructor"),
              thread_completed(false)
{
  setThreadId();
}

void myThreadInfo::setDataForTransfer(std::string incdata) {
  boost::lock_guard<boost::mutex> guard(mtx_);
  localdata_ = incdata;
}

std::string myThreadInfo::getDataForTransfer() {
  boost::lock_guard<boost::mutex> guard(mtx_);
  return localdata_;
}

void myThreadInfo::setThreadId() {
  std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
  unsigned long threadNumber = 0;
  sscanf(threadId.c_str(), "%lx", &threadNumber);
  id = threadNumber;
}

class chanParam {
public:
  chanParam();
  std::string channame;
  std::string serialport;
  bool debug;
};

chanParam::chanParam() 
         : channame("undefined"), serialport("undefined"), debug(false) {
}

vector<myThreadInfo*> rdrs(0); // вектор потоков ридеров - после создания ридер заносит инфу о себе сюда

boost::mutex mtx_rdrs; // лочит вектор потоков
boost::mutex mtx_cout; // лочит консоль для вывода лога
int dscount = 0; // глобальный счётчик переданных данных
boost::mutex mtx_dscount; // лочит глобальный счётчик переданных данных
const int max_length = 1024;
boost::interprocess::interprocess_semaphore gsem_1(0);

void logMsg(string message) {
  boost::lock_guard<boost::mutex> guard(mtx_cout);
  boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time(); // время с микросекундами
  boost::posix_time::time_duration duration(now.time_of_day());
  std::stringstream ss;
  ss << now.date().year() << "-" << now.date().month() << "-" << now.date().day() << " " << now.time_of_day();
  string curdate = ss.str();
  cout << curdate << " " << message << endl;
}

void cleanRdrs(int noready_s, int ready_stage) {
    for (int j=0; j < noready_s; j++ ) {
        std::vector<myThreadInfo*>::iterator it;
        for( it = rdrs.begin(); it != rdrs.end(); it++ ) {
            if( (*it)->thread_completed == true ) {
                myThreadInfo *mtPtr = (*it);
                std::stringstream ss;
                ss << "после готовности " << ready_stage << ", "
                   << "стираю myThreadInfo для потока: " << mtPtr->id 
                   << " и удаляю из rdrs";
                logMsg(ss.str());
                delete(mtPtr);
                rdrs.erase(it);
                break;
            }
        }
    }    
}

void writer(chanParam param) {
  std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
  unsigned long threadNumber = 0;
  sscanf(threadId.c_str(), "%lx", &threadNumber);

  {
    std::stringstream ss;
    ss << "======== ОПРЕДЕЛИТЕЛЬ НОМЕРА №: " << threadNumber << " запущен.";
    logMsg(ss.str());
  }

  SerialModem::SerialModem mdm(param.serialport);
  mdm.debug = param.debug;
  if (!mdm.init()) {
    std::stringstream ss;
    ss << "модем не инициализирован. поток определителя №: " << threadNumber;
    logMsg(ss.str());
    return;
  }

  try {
    for (int i = 1;; i++) {
      if (rdrs.size() == 0) {
        std::stringstream ss;
        ss << "определитель №: " << threadNumber << ", нет потоков готовых к отправке данных, жду 5 сек., размер вектора потоков: " << rdrs.size() << ", iteration: " << i;
        logMsg(ss.str());
        boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());
        bool ret = gsem_1.timed_wait(t + boost::posix_time::seconds(5));
      } else {
        mdm.WaitPhoneCall();
        std::string incnmbr = mdm.phonenum;
        std::string channel = param.channame;
        std::stringstream dfs;
        dfs << "DAT" << incnmbr << ";" << channel;
        std::string sendstr = dfs.str();

        std::vector<int> ready;   // счётчик готовых передатчиков
        std::vector<int> noready; // счётчик не готовых передатчиков
        mtx_rdrs.lock(); // блокирую доступ к вектору потоков передатчиков
        // ожидание готовности передатчиков принимать данные для передачи                                                     
        boost::posix_time::ptime t1(boost::posix_time::microsec_clock::universal_time());
        for (int j = 0; j < rdrs.size(); j++) {
            bool ret = rdrs[j]->sem_1.timed_wait(t1 + boost::posix_time::seconds(10));
            if (ret) {
              rdrs[j]->setDataForTransfer(sendstr);
              rdrs[j]->sem_2.post(); // семафор на передачу
              ready.push_back(j);
              std::stringstream ss;
              ss << "определитель №: " << threadNumber << ", есть готовность 1, id готового потока: " << rdrs[j]->id << ", счётчик готовых потоков: " << ready.size();
              logMsg(ss.str());
            } else {
              // если не дождался готовности передатчика в течении 10 секунд
              noready.push_back(j);
              // пишем в лог
              std::stringstream ss;
              ss << "определитель №: " << threadNumber << ", нет готовности 1 для передатчика: " << rdrs[j]->id << ", счётчик не готовых потоков: " << noready.size();
              logMsg(ss.str());
            }
        }
        // проверка состояния передатчиков после семафора на передачу
        // если количество готовых передатчиков не равно количеству ожидаемых
        // то очищаем поток передатчиков от тех кого не дождались
        if (noready.size() != 0) {
          cleanRdrs(noready.size(), 1);
        }
        ready.clear();
        noready.clear();
        
        // ожидание подтвержения передачи от передатчиков                                                                   
        boost::posix_time::ptime t2(boost::posix_time::microsec_clock::universal_time());
        for (int j = 0; j < rdrs.size(); j++) {
          bool ret = rdrs[j]->sem_3.timed_wait(t2 + boost::posix_time::seconds(10));
          if (ret) {
            ready.push_back(j);
            std::stringstream ss;
            ss << "определитель №: " << threadNumber << ", "
               << "есть готовность 2, id готового потока: " 
               << rdrs[j]->id << ", "
               << "счётчик готовых потоков: " << ready.size();
            logMsg(ss.str());
          } else {
            noready.push_back(j);
            std::stringstream ss;
            ss << "определитель №: " << threadNumber << ", "
               << "нет готовности 2 для потока: "
               << rdrs[j]->id << ", "
               << "счётчик не готовых потоков: " << noready.size();
            logMsg(ss.str());
          }
        }
        // проверка выполнения передачи
        // если количество выполненных передач не равно количеству всех потоков передатчиков
        // то очищаем поток передатчиков от тех кого не дождались
        if (noready.size() != 0) {
            cleanRdrs(noready.size(), 2);
        }
        ready.clear();
        noready.clear();
        mtx_rdrs.unlock();
        
      }
    }

    {
      std::stringstream ss;
      ss << "======== ОПРЕДЕЛИТЕЛЬ НОМЕРА №: " << threadNumber << " остановлен.";
      logMsg(ss.str());
    }
  } catch (std::exception& e) {
    std::stringstream ss;
    ss << "Exception: " << e.what() << ", определитель thread №: " << threadNumber;
    logMsg(ss.str());
  }
}

void sender(socket_ptr sock) {
  myThreadInfo *mtPtr = NULL;
  mtPtr = new myThreadInfo();
  mtx_rdrs.lock();
  rdrs.push_back(mtPtr);
  mtx_rdrs.unlock();
  gsem_1.post();
  try {
    for (int i = 1;; i++) {
      mtPtr->sem_1.post(); // сообщаю о готовности потока к передаче    
      mtPtr->sem_2.wait(); // жду команды на передачу               

      string localdata = mtPtr->getDataForTransfer();
      if (localdata != "undefined - clear from reader thread") {
        char buffer[max_length] = {0};
        boost::system::error_code error;
        std::size_t length = localdata.copy(buffer, localdata.size(), 0);
        boost::asio::write(*sock, boost::asio::buffer(buffer, sizeof (buffer)), error);
        if (error == boost::asio::error::eof) {
          std::stringstream ss;
          ss << "Sender. boost::asio::error::eof";
          logMsg(ss.str());
          break; // Connection closed cleanly by peer.
        } else if (error)
          throw boost::system::system_error(error);
        std::stringstream ss;
        ss << "Sender. поток: " << mtPtr->id << ", iteration: " << i << ", данные: \"" << localdata << "\"";
        logMsg(ss.str());
      }
      mtPtr->setDataForTransfer("undefined - clear from reader thread");
      mtPtr->sem_3.post(); // сообщаю о завершении передачи
    }
    mtPtr->thread_completed = true;
    
    {
      std::stringstream ss;           
      ss << "Sender. Normal exit" << " thread id=" << mtPtr->id;
      logMsg(ss.str());               
    }
  } catch (std::exception& e) {
    mtPtr->thread_completed = true;
    std::stringstream ss;
    ss << "Sender. Exception: " << e.what() << " thread id=" << mtPtr->id;
    logMsg(ss.str());
  }
}

void acc_inc_con(unsigned short port) // приём входящих соединений по TCP/IP
{
  boost::asio::io_service io_service;
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
  for (;;) {
    socket_ptr sock(new tcp::socket(io_service));
    a.accept(*sock);
    boost::thread t(boost::bind(sender, sock));
  }
}

bool global_init() {
  try {
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("config.ini", pt);

    string listenport = pt.get<std::string>("tcpserver.listenport");
    boost::thread listener(boost::bind(acc_inc_con, atoi(listenport.c_str())));

    // запускаем потоки определители номеров
    int chancounter = 0;
    for (boost::property_tree::ptree::iterator it1 = pt.begin(); it1 != pt.end(); it1++) {
      string sectionname = it1->first.data();
      if (sectionname.find("channel") != std::string::npos) {
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
        if (debug == "1")
          p.debug = true;
        boost::thread callerid(boost::bind(writer, p));
      }
    }
  } catch (std::exception& e) {
    std::stringstream ss;
    ss << "Exception in global_init: " << e.what();
    logMsg(ss.str());
    return false;
  }
  return true;
}

int main(int argc, char* argv[]) {
  logMsg("enter char:\r\nr - показать потоки ридеры.\r\ns - размер вектора с инфой о ридерах\r\ne - завершить программу");
  try {
    if (!global_init()) {
      logMsg("Не выполнена инициализация потоков. Программа прервана.");
      return 1;
    }

    for (;;) {
      char c;
      cin >> c;
      switch (c) {
        case 'e':
          logMsg("exit.");
          return 0;
        case 'r':
          {
            for (int i = 0; i < rdrs.size(); i++) {
              std::stringstream ss;
              ss << "rdrs[" << i << "]= " << rdrs[i]->id << " data: " << rdrs[i]->getDataForTransfer();
              logMsg(ss.str());
              rdrs[i]->sem_2.post();
            }
          }
          break;
        case 's':
          {
            for (int i = 0; i < rdrs.size(); i++) {
              std::stringstream ss;
              ss << "rdrs.size: " << rdrs.size() << " rdrs[" << i << "]=" << rdrs[i] << ", sender thrd id: " << rdrs[i]->id << ", completed: " << rdrs[i]->thread_completed;
              logMsg(ss.str());
            }
            if(rdrs.size()==0)
              logMsg("rdrs.size() = 0");
          }
          break;
        default:
          logMsg("enter char:\r\nr - показать семафор ридерам.\r\ns - размер вектора с инфой о ридерах\r\ne - завершить программу");
      }
    }
  } catch (...) {
    std::stringstream ss;
    ss << "Исключение перехвачено в main(): " << boost::current_exception_diagnostic_information() << std::endl;
    logMsg(ss.str());
  }

  return 0;
}

