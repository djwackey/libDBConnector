# makefile

CC = gcc
CXX = g++
LDFLAGS = -shared -fpic
SOURCE = $(*.c)

#INCLUDE = -I /usr/include/oracle/11.2/client
#LIBDIR = -L /usr/lib/oracle/11.2/client/lib

INCLUDE = -I ./include/mysql
LIBDIR = -L ./lib/mysql

CFLAGS = -Wall -fexceptions
CXXFLAGS = $(CFLAGS)

TARGET = libDBConnector
LIBS = -lpthread -lmysql #-lclntsh

all:
	$(CXX) $(CXXFLAGS) $(INCLUDE) src/IDBConnect.cpp src/IDBRecordSet.cpp src/MySQLConnect.cpp src/MySQLRecordSet.cpp -o $(TARGET) $(LIBDIR) $(LIBS) -g
				     #src/IDBConnect.cpp src/IDBRecordSet.cpp src/OracleConnect.cpp src/OracleRecordSet.cpp
clean:
	rm -f $(TARGET)
