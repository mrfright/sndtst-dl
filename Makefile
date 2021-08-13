CFLAGS =-g -Wall -O3 `curl-config --cflags` `pkg-config tidy --cflags`
LDLIBS=`curl-config --libs ` `pkg-config tidy --libs` -lpthread

#why nee lpthread?

#gnu says default standard is gnu17, which is late enough

#I guess can upt just this and assumes depends on just sndtst-dl.c?
sndtst-dl:
