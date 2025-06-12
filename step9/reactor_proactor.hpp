#pragma once

#include <pthread.h>

typedef void (*reactorFunc)(int fd);

void* startReactor();
int addFdToReactor(void* reactor, int fd, reactorFunc func);
int removeFdFromReactor(void* reactor, int fd);
int stopReactor(void* reactor);
void runReactor(void* reactor); // Add this line for running the event loop

typedef void* (*proactorFunc)(int sockfd);
pthread_t startProactor(int sockfd, proactorFunc threadFunc);
int stopProactor(pthread_t tid);