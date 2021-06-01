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