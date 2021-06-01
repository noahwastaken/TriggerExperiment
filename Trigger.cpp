#include <Trigger.h>

constexpr int LIFE_ALIVE = 0;

namespace G
{
	namespace Proc
	{
		DWORD m_dwPID = 0x0;
		HANDLE m_hProc = 0;
	}

	namespace Off
	{
		constexpr std::ptrdiff_t m_lifeState = 0x25F;
		constexpr std::ptrdiff_t m_iCrosshairId = 0xB3E4;
		constexpr std::ptrdiff_t m_iTeamNum = 0xF4;
		constexpr std::ptrdiff_t dwLocalPlayer = 0xD3DD14;
		constexpr std::ptrdiff_t dwEntityList = 0x4D5239C;
	}

	namespace Mem
	{
		template<class T>
		T Read(DWORD dwAddr)
		{
			T cData;
			ReadProcessMemory(G::Proc::m_hProc, reinterpret_cast<LPCVOID>(dwAddr), &cData, sizeof(T), 0);
			return cData;
		}
	}
}

bool GetProcess(const wchar_t* szName)
{
	if (auto hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0))
	{
		PROCESSENTRY32 ProcEntry;
		ProcEntry.dwSize = sizeof(ProcEntry);

		while (Process32Next(hProcess, &ProcEntry))
		{
			if (!wcscmp(ProcEntry.szExeFile, szName))
			{
				CloseHandle(hProcess);

				G::Proc::m_dwPID = ProcEntry.th32ProcessID;
				G::Proc::m_hProc = OpenProcess(0x38, 0, G::Proc::m_dwPID);

				return true;
			}
		}

		CloseHandle(hProcess);
	}
	return false;

	DWORD GetModule(const wchar_t* szModule)
	{
		if (auto hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, G::Proc::m_dwPID))
		{
			MODULEENTRY32 ModEntry;
			ModEntry.dwSize = sizeof(ModEntry);

			while (Module32Next(hModule, &ModEntry))
			{
				if (!wcscmp(ModEntry.szModule, szModule))
				{
					CloseHandle(hModule);
					return reinterpret_cast<DWORD>(ModEntry.modBaseAddr);
				}
			}

			CloseHandle(hModule);
		}

		return 0x0;
	}

	int main()
	{
		SetConsoleTitleW(L"Trigger Example");

		if (GetProcess(L"insert.exe"))
		{
			if (const auto dwClient = GetModule(L"insert.dll"))
			{
				wprintf(L" If Running!\n");

				while (!(GetAsyncKeyState(VK_F11) & 0x01))
				{
					if (const auto& dwLocal = G::Mem::Read<DWORD>(dwClient + G::Off::dwLocalPlayer))
					{
						const auto nCrossID = G::Mem::Read<int>(dwLocal + G::Off::m_iCrosshairId);

						if (nCrossID <= 64 && nCrossID >= 0)
						{
							if (const auto& dwEntity = G::Mem::Read<DWORD>(dwClient + G::Off::dwEntityList + (nCrossID * 0x10)))
							{
								if (G::Mem::Read<byte>(dwEntity + G::Off::m_lifeState) == LIFE_ALIVE)
								{
									if (G::Mem::Read<int>(dwEntity + G::Off::m_iTeamNum) != G::Mem::Read<int>(dwLocal + G::Off::m_iTeamNum))
									{
										if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000)
										{

											mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);

											std::this_thread::sleep_for(std::chrono::milliseconds(10));

											mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
										}
									}
								}
							}
						}
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
			else
			{
				wprintf(L"Not able to access insert.dll!\n");
			}
		}
		else
		{
			wprintf(L"Did not find insert.exe!\n");
		}

		if (G::Proc::m_hProc)
			CloseHandle(G::Proc::m_hProc);

		wprintf(L"Quitted!\n");
		_wsystem(L"pause");

		return EXIT_SUCCESS;
	}