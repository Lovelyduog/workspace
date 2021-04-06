Program('server',
    [
        Glob('*.cpp'),
        Glob('#/db_plugin/*.cpp')
    ],
    CPPPATH = [
                '#'
    ],
    LIBS = [
            'libboost_system',
            'libboost_thread',
            'libboost_date_time',
            'libboost_filesystem',
            'libboost_regex',
            'libboost_serialization',
            'libboost_program_options',
            'libmysqlcppconn-static',
            'libmysqlclient',
            'libssl',
            'libcrypto',
            'libpthread',
            'libicuuc',
            'libicudata',
            'rt',
            'dl',
            'z'
        ],
    LINKFLAGS = '-static',
    LIBPATH = [
                '/usr/lib/x86_64-linux-gnu',
                '/usr/local/lib',
                '/usr/lib',
                '/usr/local/mysql/lib',
                '#/'
            ],
    CCFLAGS = '-g -DPOSIX  -DNDEBUG -Wall -DISABLE_PICOJSON -std=c++11 -DBOOST_COROUTINES_NO_DEPRECATION_WARNING'
)
