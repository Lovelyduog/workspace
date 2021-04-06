#include "mysql_plugin.hpp"
#include <iostream>
#include<sstream>
namespace db{

    MysqlPlugin* MysqlPlugin::s_instance;

    MysqlPlugin::MysqlPlugin()
    {

    }

    MysqlPlugin* MysqlPlugin::Getinstance()
    {
        if(s_instance == nullptr)
        {
            s_instance = new MysqlPlugin();
        }
    }

    bool MysqlPlugin::Connect(const std::string& host, const std::string& user, const std::string& passord)
    {
        _Driver = sql::mysql::get_mysql_driver_instance();
        if(_Driver)
        {
            _Connection = _Driver->connect(host, user, passord);

            if (_Connection)
            {
                sql::Statement *state = _Connection->createStatement();
                if (state)
                {
                    std::ostringstream osql1;
                    osql1 << "use test_db;";
                    state->execute(osql1.str());
                    std::ostringstream osql;
                    osql << "create table if not exists userinfo(";
                    osql << "name varchar(40) not null";
                    // osql << "id int not null";
                    osql << ")ENGINE = InnoDB;";
                    std::cout << osql.str() << std::endl;
                    state->execute(osql.str());
                }
                
            }
            
        }
        return true;
    }

}//end namespace db
