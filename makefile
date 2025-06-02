# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2

# 생성할 실행 파일 name
TARGET = queue_test

# 소스 파일, 오브젝트 파일
SRCS = main.cpp queue.cpp
OBJS = $(SRCS:.cpp=.o)

# 기본 타겟
all: $(TARGET)

# 실행 파일 생성
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 개별 오브젝트 파일 생성
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)