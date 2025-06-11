#pragma once
#include <string>

// Connects to the server at the given host and port, returns socket fd or -1 on error
int connect_to_server(const std::string& host, const std::string& port);

// Runs the interactive client loop (send commands, print responses)
void run_client(int sockfd);