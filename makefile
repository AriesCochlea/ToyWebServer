CXX      = g++
CXXFLAGS = -O2 -std=c++11
LIBS     = -lpthread -lmysqlclient

SOURCE   = main.cpp \
           code/webserver.cpp \
           code/timer.cpp \
           code/http_conn.cpp \
           code/log.cpp \
           code/sql_connection_pool.cpp \


server:
	$(CXX) $(CXXFLAGS) $(SOURCE) $(LIBS) -o server 

clean:
	rm server
