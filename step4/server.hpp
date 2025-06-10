#pragma once
#include <string>

// Start the convex hull server (blocking call)
void run_server(int port = 9034);

// Handle a single command from a client and return the response
std::string handle_command(const std::string& cmdline);