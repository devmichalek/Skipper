#pragma once
#include <string>

void PrintMessage(int color, const char* prefix, const char* &filename, int &line, const char* &msg);
void PrintError(const char* filename, int line, const char* msg);
void PrintFatalError(const char* filename, int line, const char* msg);
void PrintWarning(const char* filename, int line, const char* msg);
void PrintSuccess(const char* filename, int line, const char* msg);