TARGET	= smbcfs
CXXFLAGS	+= -Wall -pipe
# CXXFLAGS	+= -O0 -g
CXXFLAGS	+= -O3
CXXFLAGS	+= ${pkg-config --cflags fuse}
LIBS	+= ${pkg-config --libs-only-l --libs-only-other fuse}
LDFLAGS	+= ${pkg-config --libs-only-L fuse}
LIBS	+= -lsmbclient
LDFLAGS += -L/opt/local/lib/samba3

.PHONY: all clean

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -o $(TARGET) main.cpp

clean:
	$(RM) $(TARGET)
	$(RM) -r $(TARGET).dSYM
