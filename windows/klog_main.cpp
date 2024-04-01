#define UNICODE
#include <Windows.h>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <map>

#include "sqlite3.h"

const char DATABASE_FILE[] = "keystrokes.sqlite";

using namespace std;
// defines whether the window is visible or not
// should be solved with makefile, not in this file
#define visible // (visible / invisible)
// Defines whether you want to enable or disable 
// boot time waiting if running at system boot.
#define bootwait // (bootwait / nowait)
// defines which format to use for logging
// 0 for default, 10 for dec codes, 16 for hex codex
#define FORMAT 0
// defines if ignore mouseclicks
#define mouseignore
// variable to store the HANDLE to the hook. Don't declare it anywhere else then globally
// or you will get problems since every function uses this variable.

#if FORMAT == 0
const std::map<int, std::string> keyname{ 
	{VK_BACK, "[BACKSPACE]" },
	{VK_RETURN,	"\n" },
	{VK_SPACE,	"_" },
	{VK_TAB,	"[TAB]" },
	{VK_SHIFT,	"[SHIFT]" },
	{VK_LSHIFT,	"[LSHIFT]" },
	{VK_RSHIFT,	"[RSHIFT]" },
	{VK_CONTROL,	"[CONTROL]" },
	{VK_LCONTROL,	"[LCONTROL]" },
	{VK_RCONTROL,	"[RCONTROL]" },
	{VK_MENU,	"[ALT]" },
	{VK_LWIN,	"[LWIN]" },
	{VK_RWIN,	"[RWIN]" },
	{VK_ESCAPE,	"[ESCAPE]" },
	{VK_END,	"[END]" },
	{VK_HOME,	"[HOME]" },
	{VK_LEFT,	"[LEFT]" },
	{VK_RIGHT,	"[RIGHT]" },
	{VK_UP,		"[UP]" },
	{VK_DOWN,	"[DOWN]" },
	{VK_PRIOR,	"[PG_UP]" },
	{VK_NEXT,	"[PG_DOWN]" },
	{VK_OEM_PERIOD,	"." },
	{VK_DECIMAL,	"." },
	{VK_OEM_PLUS,	"+" },
	{VK_OEM_MINUS,	"-" },
	{VK_ADD,		"+" },
	{VK_SUBTRACT,	"-" },
	{VK_CAPITAL,	"[CAPSLOCK]" },
};
#endif
HHOOK _hook;

// This struct contains the data received by the hook callback. As you see in the callback function
// it contains the thing you will need: vkCode = virtual key code.
KBDLLHOOKSTRUCT kbdStruct;

int incStrokeCount(int key_stroke);
int keystrokeCount = 0;

bool keep_running = true;

const int R_INTERVAL = 20;

// This is the callback function. Consider it the event that is raised when, in this case,
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		// the action is valid: HC_ACTION.
		if (wParam == WM_KEYDOWN)
		{
			// lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

			// save to file
			incStrokeCount(kbdStruct.vkCode);
		}
	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
	// Set the hook and set it to use the callback function above
	// WH_KEYBOARD_LL means it will set a low level keyboard hook. More information about it at MSDN.
	// The last 2 parameters are NULL, 0 because the callback function is in the same thread and window as the
	// function that sets and releases the hook.
	if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
		LPCWSTR a = L"Failed to install hook!";
		LPCWSTR b = L"Error";
		MessageBox(NULL, a, b, MB_ICONERROR);
	}
}

void ReleaseHook()
{
	UnhookWindowsHookEx(_hook);
}

int incStrokeCount(int key_stroke)
{
    if ((key_stroke == 1) || (key_stroke == 2))
        return 0;
    
    if (key_stroke == VK_BACK ||
        key_stroke == VK_RETURN ||
        key_stroke == VK_SPACE ||
        key_stroke == VK_TAB ||
        key_stroke == VK_SHIFT ||
        key_stroke == VK_LSHIFT ||
        key_stroke == VK_RSHIFT ||
        key_stroke == VK_CONTROL ||
        key_stroke == VK_LCONTROL ||
        key_stroke == VK_RCONTROL ||
        key_stroke == VK_ESCAPE ||
        key_stroke == VK_END ||
        key_stroke == VK_HOME ||
        key_stroke == VK_LEFT ||
        key_stroke == VK_UP ||
        key_stroke == VK_RIGHT ||
        key_stroke == VK_DOWN ||
        key_stroke == 190 ||
        key_stroke == 110 ||
        key_stroke == 189 ||
        key_stroke == 109 || 
        key_stroke == 20 ) {
        // Ignore
    }
    else {
        //if ( keystrokeCount % 10 == 0) printf("keystroke detected\n");
        keystrokeCount++;
    }
    return 0;
}

void Stealth()
{
#ifdef visible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1); // visible window
#endif

#ifdef invisible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0); // invisible window
	FreeConsole(); // Detaches the process from the console window. This effectively hides the console window and fixes the broken invisible define.
#endif
}

void FileTimeToTimeT(time_t& t, FILETIME ft) {
    ULARGE_INTEGER time_value;

    time_value.LowPart  = ft.dwLowDateTime;
    time_value.HighPart = ft.dwHighDateTime;

    t = (time_value.QuadPart - 116444736000000000LL) / 10000000LL;
}

time_t GetTimeStamp() {
  SYSTEMTIME lt;
  ULARGE_INTEGER timestamp64bit;
  GetLocalTime(&lt);
  FILETIME ft;
  SystemTimeToFileTime(&lt, &ft);
  time_t t;
  FileTimeToTimeT(t, ft);
  return t;
}

void TimeToString(char* str, time_t t) {
  struct tm *tm = gmtime(&t);
  strftime(str, 20, "%d/%m/%y - %H:%M", tm);
}

int create_db() {
    sqlite3* db;
    char *errMsg = 0;

    // Open the database connection
    int rc = sqlite3_open(DATABASE_FILE, &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    } 

    // Execute SQL statement
    const char *query = "CREATE TABLE IF NOT EXISTS KEYLOG("
                        "TIMESTAMP     INT PRIMARY_KEY     NOT NULL,"
                        "COUNT         INT                 NOT NULL);";

    rc = sqlite3_exec(db, query, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Table create query succeeded" << std::endl;
    }

    sqlite3_close(db);
    return 0;
}

int insert_row(time_t timestamp, int keystroke_count) {
    sqlite3* db;
    char *errMsg = 0;

    // Open the database connection
    int rc = sqlite3_open(DATABASE_FILE, &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    const char *query = "INSERT INTO KEYLOG (TIMESTAMP, COUNT) "
                        "VALUES (%ld, %d);";

    char query_expanded[256];

    sprintf(query_expanded, query, timestamp, keystroke_count);
    //printf("insert query: %s\n", query_expanded);

    rc = sqlite3_exec(db, query_expanded, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        //std::cout << "Row inserted successfully" << std::endl;
    }

    // Close the database connection
    sqlite3_close(db);

    return 0;
}

// This thread will report and update database periodically
DWORD WINAPI reporter(LPVOID lpParam) {
    int threadId = *(reinterpret_cast<int*>(lpParam));
    time_t last, delay;

    last = 0;
    delay = 60 * R_INTERVAL; // R_INTERVAL minutes delay

    while (keep_running) {
        time_t current = GetTimeStamp();
        if ((current - last) >= delay) {
            char str[256]; 
            TimeToString(str, current);
            printf("%s - %d\n", str, keystrokeCount);
            insert_row(current, keystrokeCount);
            last = current;
            time_t mins = (last / 60) % 60;
            delay = 60 * (R_INTERVAL - (mins % R_INTERVAL));
            std::cout.flush();
            //printf("Sleeping for %d seconds\n", delay);
            Sleep(delay * 1000);
        } else {
            // Check back in a second
            Sleep(1000);
        }
    }
    //printf("Reporter thread quitting\n");
    return 0;
}

int main()
{

  HANDLE thread;
  DWORD tid;
  int rank = 0;
  create_db();
  thread = CreateThread(NULL, 0, reporter, &rank, 0, &tid);

  // visibility of window
	Stealth();

	// Set the hook
	SetHook();

  // loop to keep the console application running.
  /*std::string line;
	while (keep_running) {
      std::getline(std::cin, line);

      if (line == "done") {
          printf("Marking the end of the program\n");
          ReleaseHook();
          keep_running = false;
      } else if (line == "show") {
          printf("Current keystroke count: %d\n", keystrokeCount);
      } else {
      }
	}*/
  MSG msg;

  while(GetMessage(&msg, NULL, 0, 0)) {
    //std::cout << "Message: " << msg.message << std::endl;
  }

  WaitForMultipleObjects(1, &thread, TRUE, INFINITE);
  CloseHandle(thread);
  return 0;
}
