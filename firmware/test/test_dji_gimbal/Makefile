CXX = c++
CXXFLAGS = -std=c++11 -Wall -Wextra -I../../lib/dji_can -Iunity

SRCS = test_dji_gimbal.cpp \
       ../../lib/dji_can/dji_crc.cpp \
       ../../lib/dji_can/dji_protocol.cpp \
       ../../lib/dji_can/dji_gimbal.cpp \
       unity/unity.c

TARGET = test_runner

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
