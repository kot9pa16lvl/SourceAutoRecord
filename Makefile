.PHONY: all clean cvars
.FORCE:

CXX=g++-10
SDIR=src
ODIR=obj

SRCS=$(wildcard $(SDIR)/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/Demo/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/Hud/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/Routing/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/Speedrun/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/Speedrun/Rules/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/Stats/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/ReplaySystem/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/Tas/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/Tas/TasTools/*.cpp)
SRCS+=$(wildcard $(SDIR)/Features/Timer/*.cpp)
SRCS+=$(wildcard $(SDIR)/Games/*.cpp)
SRCS+=$(wildcard $(SDIR)/Modules/*.cpp)
SRCS+=$(wildcard $(SDIR)/Utils/*.cpp)
SRCS+=$(wildcard $(SDIR)/Utils/SDK/*.cpp)
SRCS+=$(wildcard $(SDIR)/Utils/ed25519/*.cpp)

OBJS=$(patsubst $(SDIR)/%.cpp, $(ODIR)/%.o, $(SRCS))

VERSION=$(shell git describe --tags)

# Header dependency target files; generated by g++ with -MMD
DEPS=$(OBJS:%.o=%.d)

WARNINGS=-Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-delete-non-virtual-dtor -Wno-overloaded-virtual
CXXFLAGS=-std=c++17 -m32 $(WARNINGS) -I$(SDIR) -fPIC -D_GNU_SOURCE -Ilib/ffmpeg/include -Ilib/SFML/include -Ilib/curl/include -Ilib/discord-rpc/include -DSFML_STATIC -DCURL_STATICLIB
LDFLAGS=-m32 -shared -lstdc++fs -Llib/ffmpeg/lib/linux -lavformat -lavcodec -lavutil -lswscale -lswresample -lx264 -lx265 -lvorbis -lvorbisenc -lvorbisfile -logg -lopus -lvpx -Llib/SFML/lib/linux -lsfml -Llib/curl/lib/linux -lcurl -lssl -lcrypto -lnghttp2 -Llib/discord-rpc/lib/linux -ldiscord-rpc

# Import config.mk, which can be used for optional config
-include config.mk

all: sar.so
clean:
	rm -rf $(ODIR) sar.so src/Version.hpp

-include $(DEPS)

sar.so: src/Version.hpp $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(ODIR)/%.o: $(SDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

src/Version.hpp: .FORCE
	if [ "$$RELEASE_BUILD" ]; then echo "#define SAR_VERSION \"$(VERSION)\"" >"$@"; fi
	if [ -z "$$RELEASE_BUILD" ]; then echo "#define SAR_VERSION \"$(VERSION)-canary\"" >"$@"; fi
	if [ -z "$$RELEASE_BUILD" ]; then echo "#define SAR_DEV_BUILD 1" >>"$@"; fi
	echo "#define SAR_DEMO_SIGN_PUBKEY { $$DEMO_SIGN_PUBKEY }" >>"$@"
	echo "#define SAR_DEMO_SIGN_PRIVKEY { $$DEMO_SIGN_PRIVKEY }" >>"$@"

cvars: docs/cvars.md
docs/cvars.md:
	node cvars.js "$(STEAM)Portal 2"
