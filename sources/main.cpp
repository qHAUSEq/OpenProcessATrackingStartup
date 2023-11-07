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
//	setlocale(LC_ALL, "RU");
	system("chcp1251");
	std::string nameOfProgs;
	std::vector<std::string> vecNameOfProgs;
	std::ifstream in;

	// Записываем строки из файла в вектор
	in.open("TXT\\cfg.txt");
	if (!in.is_open()) // Проверяем открылся ли файл
	{
		std::cout << "Unknown file" << std::endl;
		return -1;
	}
	do {
		in >> nameOfProgs;
		vecNameOfProgs.push_back(nameOfProgs);
	} while (!in.eof());
	nameOfProgs.clear();
	in.close();

	// Открываем внешний процесс на компьютере.
	std::vector<STARTUPINFO> si(vecNameOfProgs.size());
	std::vector<PROCESS_INFORMATION> pi(vecNameOfProgs.size());

	for (size_t var = 0; var < vecNameOfProgs.size(); ++var)
	{
		if (!CreateProcess(NULL, const_cast<char*>(vecNameOfProgs[var].c_str()), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si[var], &pi[var]))
		{
			std::cout << "CreateProcess Failed." << std::endl;
			DWORD error = GetLastError();
			char errStr[1024] = {};
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, errStr, 1024, NULL);
			std::cout << errStr << std::endl; // Вывод ошибки, по какой причине не смогло открыться приложение

			// Удаление элемента вектора (Если он не рабочий), чтобы под него не создавался отдельный поток
			CloseHandle(pi[var].hProcess);
			CloseHandle(pi[var].hThread);
			si.erase(si.begin() + var);
			pi.erase(pi.begin() + var);
			vecNameOfProgs.erase(vecNameOfProgs.begin() + var);

			std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // Останавливаем основной поток на 5 сек, чтобы прочитать ошибку
		}
	}

	system("cls");

	// Реализация многопоточности
	std::vector<std::thread> th_vec;
	do
	{
		std::cout << "Enter ESC to exit" << std::endl;
		for (size_t i = 0; i < vecNameOfProgs.size(); ++i)
		{
			th_vec.push_back(std::thread(startProc, vecNameOfProgs[i])); // Добавляем в вектор потоков кол-во потоков = кол-ву открытых программ
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
		system("cls");
		for (size_t i = 0; i < vecNameOfProgs.size(); ++i)
		{
			th_vec.at(i).detach();
		}
		th_vec.clear();
	} while (GetAsyncKeyState(VK_ESCAPE) == 0); // нажать esc, чтобы завершить проверку

	for (int i = 0; i < vecNameOfProgs.size(); ++i)
	{
		if (isProcessRun(vecNameOfProgs[i]) == true) // Проверяем количество включенных программ и если какая-либо включена, то закрываем
		{
			std::string strKillProc = "taskkill /IM " + vecNameOfProgs[i];
			system(strKillProc.c_str()); // Закрывает все программы с помощью консольной команды
			strKillProc.clear();
		}
		CloseHandle(pi[i].hProcess);
		CloseHandle(pi[i].hThread);
	}
	pi.clear();
	si.clear();
	vecNameOfProgs.clear();
	return 0;
}

void startProc(std::string procName)
{
	mtx.lock();
	std::cout << "Process name: " << procName << ": " << (isProcessRun(procName) ? " works " : " not works ") << std::endl; // Вывод в консоль работает ли процесс, с помощью тернарных операторов
	mtx.unlock();
}

bool isProcessRun(std::string processName) // Поиск рабочего процесса
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