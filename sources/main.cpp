#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <Windows.h>
#include <tlhelp32.h>
#include <conio.h>

std::mutex mtx;

void startProc(std::string procName);
bool isProcessRun(std::string processName);

int main()
{
	std::string nameOfProgs;
	std::vector<std::string> vecNameOfProgs;
	std::ifstream in;

	// Записываем строки из файла в вектор
	in.open("TXT\\cfg.txt");
	if (!in.is_open()) // Проверяем открылся ли файл
	{
		std::cout << "Unknown file" << std::endl;
	}
	do {
		in >> nameOfProgs;
		vecNameOfProgs.push_back(nameOfProgs);
	} while (!in.eof());
	in.close();

	// Открываем внешний процесс на компьютере.
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	for (size_t var = 0; var < vecNameOfProgs.size(); var++)
	{
		if (!CreateProcess(NULL, const_cast<char*>(vecNameOfProgs[var].c_str()), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
		{
			std::cout << "CreateProcess Failed" << std::endl;
			vecNameOfProgs.erase(vecNameOfProgs.begin() + var);
		}
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	// Реализация многопоточности
	std::vector<std::thread> th_vec;
	do
	{
		for (size_t i = 0; i < vecNameOfProgs.size(); ++i)
		{
			th_vec.push_back(std::thread(startProc, vecNameOfProgs[i]));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
		system("cls");
		for (size_t i = 0; i < vecNameOfProgs.size(); ++i)
		{
			th_vec.at(i).join();
		}
		th_vec.clear();
	} while (GetAsyncKeyState(VK_ESCAPE) == 0); // нажать esc, чтобы завершить проверку
	
	vecNameOfProgs.clear();
	WaitForSingleObject(pi.hProcess, INFINITE);
	return 0;
}

void startProc(std::string procName)
{
	mtx.lock();
	std::cout << "Process name: " << procName << ": " << (isProcessRun(procName)? " works " : " not works ") << std::endl;
	mtx.unlock();
}

bool isProcessRun(std::string processName)
{
	HANDLE hSnap = NULL;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) return FALSE;
	if (hSnap != NULL)
	{
		if (Process32First(hSnap, &pe32))
		{
			if (lstrcmp(pe32.szExeFile, processName.c_str()) == 0)
			{
				CloseHandle(hSnap);
				return TRUE;
			}
			while (Process32Next(hSnap, &pe32))
			{
				if (lstrcmp(pe32.szExeFile, processName.c_str()) == 0)
				{
					CloseHandle(hSnap);
					return TRUE;
				}
			}
		}
	}
	CloseHandle(hSnap);
	return FALSE;
}