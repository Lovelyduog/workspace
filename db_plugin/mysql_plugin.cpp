#include "mysql_plugin.hpp"
#include <iostream>
#include<sstream>
namespace db{

    MysqlPlugin* MysqlPlugin::s_instance;

    MysqlPlugin::MysqlPlugin():
    _work_io_service(),_work(new boost::asio::io_service::work(_work_io_service)),
    _work_thread(new boost::thread(boost::bind(&boost::asio::io_service::run, &_work_io_service))),
    _check_deadline(_work_io_service)
    {
        _bInited = false;
        _bConnecting = false;
        _bConnected = false;
        _bClosing = false;

        for(int i = 0; i < MAX_DB_NUM; ++i)
        {
            _Drivers[i] = nullptr;
        }

        for(int i = 0; i < MAX_DB_NUM; ++i)
        {
            _Connections[i] = nullptr;
        }

    }

    void MysqlPlugin::Init(const std::string& ip, const std::string& user, const std::string& password)
    {
        if(_bInited)
        {
            return;
        }

        for(int i = 0; i < MAX_DB_NUM; ++i)
        {
            _connection_ips[i] = ip;
        }
        _user = user;
        _password = password;
        ConnectMasterDB();
        _bInited = true;
    }

    MysqlPlugin* MysqlPlugin::Getinstance()
    {
        if(s_instance == nullptr)
        {
            s_instance = new MysqlPlugin;
        }
        return s_instance;
    }

    void MysqlPlugin::ConnectMasterDB()
    {
        if (!_bConnected && !_bClosing)
        {
            _bConnecting = true;;
        }
        else
        {
            return;
        }
        
        if(_bConnected)
        {
            _bConnecting = false;
            return;
        }

        sql::Statement* state = nullptr;
        {  
            for (int i = 0; i < MAX_DB_NUM; ++i)
            {
                boost::mutex::scoped_lock lock(_rwMutex[i]);

                try
                {
                    if (_Drivers[i] != nullptr && !_Connections[i]->isClosed())
                    {
                        continue;
                    }

                    if(_Drivers[i] != nullptr)
                    {
                        _Drivers[i] = nullptr;
                    }

                    _Drivers[i] = sql::mysql::get_mysql_driver_instance();
                    if(_Drivers[i] == nullptr)
                    {
                        _bConnecting = false;
                        return;
                    }

                    if (_Connections[i] != nullptr)
                    {
                        if(!_Connections[i]->isClosed())
                        {
                            _Connections[i]->close();
                        }
                        delete _Connections[i];
                        _Connections[i] = nullptr;
                    }
                    
                    sql::ConnectOptionsMap ConnectionProperties;
                    CreateConnectionProperties(_connection_ips[i], _user, _password, ConnectionProperties);
                    _Connections[i] = _Drivers[i]->connect(ConnectionProperties);

                    if (_Connections[i] == nullptr)
                    {
                        _bConnecting = false;
                        return;
                    }
                    
                    state = _Connections[i]->createStatement();
                    if (state == nullptr)
                    {
                        _bConnecting = false;
                        return;
                    }

                    std::ostringstream osCreateDb;
                    osCreateDb << "create database if not exists Tx_" << i <<";";
                    state->execute(osCreateDb.str());
                    std::ostringstream osUseDb;
                    osUseDb << "use Tx_ " << i << ";";
                    state->execute(osUseDb.str());
                    state->close();
                    delete state;
                    state = nullptr;
                    
                }
                catch(sql::SQLException& e)
                {
                    if (nullptr != state)
                    {
                        state->close();
                        delete state;
                        state = nullptr;
                    }
                    _bConnected = false;
                }
                catch(...)
                {
                    if (nullptr != state)
                    {
                        state->close();
                        delete state;
                        state = nullptr;
                    }
                    _bConnected = false;
                }
                
            }
            
        }

        if(_bConnecting)
        {
            _bConnecting = false;
            _bConnected = true;
        } 
    }

    //该操作必定是在一次操作db中进行的,在调用前必须加锁
    // 该函数内部不能加锁,应为会产生死锁(前面的锁没打开,后面又加了锁)
    void MysqlPlugin::reconnectDb(int iDBPos)
    {
        sql::Statement* state = nullptr;
        {
            try
            {
                if(_Connections[iDBPos] != nullptr)
                {
                    if(!_Connections[iDBPos]->isClosed())
                    {
                        _Connections[iDBPos]->close();
                    }
                    
                    delete _Connections[iDBPos];
                    _Connections[iDBPos] = nullptr;
                }

                sql::ConnectOptionsMap ConnectionProperties;
                CreateConnectionProperties(_connection_ips[iDBPos], _user, _password, ConnectionProperties);
                _Connections[iDBPos] = _Drivers[iDBPos]->connect(ConnectionProperties);

                if(_Connections[iDBPos] == nullptr)
                {
                    _bConnecting = false;
                    return;
                }

                state = _Connections[iDBPos]->createStatement();
                if (nullptr == state)
                {
                    _bConnecting = false;
                    return;
                }
                std::ostringstream ostrDB;
                ostrDB << "use  Tx_" << iDBPos << ";";

                state->execute(ostrDB.str());

                state->close();
                delete state;
                state = nullptr;
                
            }
            catch(sql::SQLException& e)
            {
                if (nullptr != state)
                {
                    state->close();
                    delete state;
                    state = nullptr;
                }

                int errorCode = e.getErrorCode();
                if (IsErrorCodeNeedReconnect(errorCode))
                {
                   closeMasterDb();
                }
            }
            catch(...)
            {
                if (nullptr != state)
                {
                    state->close();
                    delete state;
                    state = nullptr;
                }
            }
        }
    }

    void MysqlPlugin::CreateConnectionProperties(const std::string& host, const std::string &strUserName, const std::string &strPassword, sql::ConnectOptionsMap &ConnectionProperties)
    {
        ConnectionProperties["hostName"] = CreateConnectPropertyVal(host);
        ConnectionProperties["userName"] = CreateConnectPropertyVal(strUserName);
        ConnectionProperties["password"] = CreateConnectPropertyVal(strPassword);
        int iTimeOut = 2;
        ConnectionProperties["OPT_WRITE_TIMEOUT"] = CreateConnectPropertyVal(iTimeOut);
        ConnectionProperties["OPT_READ_TIMEOUT"] = CreateConnectPropertyVal(iTimeOut);
        bool bCompress = true;
        ConnectionProperties["CLIENT_COMPRESS"] = CreateConnectPropertyValBool(bCompress);
        bool bMultiStatements = true;
        ConnectionProperties["CLIENT_MULTI_STATEMENTS"] = CreateConnectPropertyValBool(bMultiStatements);
    }


    sql::ConnectPropertyVal MysqlPlugin::CreateConnectPropertyVal(const std::string &strValue)
    {
        sql::ConnectPropertyVal val(strValue);
        return val;
    }

    sql::ConnectPropertyVal MysqlPlugin::CreateConnectPropertyVal(int iValue)
    {
        sql::ConnectPropertyVal val(iValue);
        return val;
    }

    sql::ConnectPropertyVal MysqlPlugin::CreateConnectPropertyValBool(bool bValue)
    {
        sql::ConnectPropertyVal val(bValue);
        return val;
    }
    bool MysqlPlugin::Connect(const std::string& host, const std::string& user, const std::string& passord)
    {
        
  

    }

    void MysqlPlugin::closeMasterDb()
    {
        if (!_bClosing && !_bConnecting)
        {
            _bClosing = true;
        }
        else
        {
            return;
        }

        _bClosing = false;
        _bConnected = false;

        return;
    }

    bool MysqlPlugin::IsErrorCodeNeedReconnect(int iErrorCode)
    {
        if ((iErrorCode == 0x7dd) || (iErrorCode == 1043) || (iErrorCode == 1130) || (iErrorCode == 1158) || (iErrorCode == 1159) || (iErrorCode == 1160) || (iErrorCode == 1161) || (iErrorCode == 2001) || (iErrorCode == 2002) || (iErrorCode == 2003) || (iErrorCode == 2004) || (iErrorCode == 2005) || (iErrorCode == 2006) || (iErrorCode == 2014) || (iErrorCode == 2048))
        {
            return true;
        }

        return false;
    }

    bool MysqlPlugin::FuncHandleSqlException(int dbPos, const std::string &command, sql::SQLException &e)
    {
        int errorCode = e.getErrorCode();
        if (IsErrorCodeNeedReconnect(errorCode))
        {
            closeMasterDb();
            reconnectDb(dbPos);
            return true;
        }

        return false;
    }

    bool MysqlPlugin::execute(int dbPos, const std::string& command)
    {
        //超时操作还待斟酌
        boost::posix_time::ptime time_begin = boost::posix_time::microsec_clock::universal_time();
        boost::posix_time::ptime time_begin_mutex;

        bool bSuccess = false;
        bool bRet = false;
        sql::Statement* state = nullptr;
        for(int i = 0; i < 5; ++i)
        {   
            boost::mutex::scoped_lock scop_lock(_rwMutex[dbPos]);
            time_begin_mutex = boost::posix_time::microsec_clock::universal_time();
            try
            {
                if(_Connections[dbPos] != nullptr )
                {
                    break;
                }

                if(!_Connections[dbPos]->isClosed())
                {
                    state = _Connections[dbPos]->createStatement();

                    if (state != nullptr)
                    {
                        state->execute(command);
                        bSuccess = true;
                        bRet = true;
                        state->close();
                        delete state;
                        state = nullptr;
                    }                 
                }
                else
                {
                    closeMasterDb();
                }
            }
            catch(sql::SQLException& e)
            {
                if(nullptr != state)
                {
                    state->close();
                    delete state;
                    state = nullptr;
                }
                bSuccess = !FuncHandleSqlException(dbPos, command, e);
            }
            catch(...)
            {
                if(nullptr != state)
                {
                    state->close();
                    delete state;
                    state = nullptr;
                }
            }

            if (bSuccess)
            {
                break;
            }
        }  

        boost::posix_time::ptime time_end = boost::posix_time::microsec_clock::universal_time();    

        boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse = time_end - time_begin;
        int ticks = time_elapse.ticks();
        if (ticks >= 5000)
        {
        }

        time_elapse = time_end - time_begin_mutex;
        ticks = time_elapse.ticks();
        if (ticks >= 5000)
        {
        }

        return bRet;
    }

}//end namespace db
