#pragma once
#define LE_WIN32 _WIN32
#define LE_WIN64 _WIN64
#define LE_WIN LE_WIN32 || LE_WIN64
#define LE_LINUX __linux__
#define LE_APPLE __APPLE__
#define LE_UNIX __unix