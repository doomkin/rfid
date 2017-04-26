#include "StdAfx.h"
#include "DatamaxPrinter.h"

// Constructor
DatamaxPrinter::DatamaxPrinter(void)
{
	// load DLL
	hInstanceLib = LoadLibrary(TEXT("DatamaxE4203.dll"));
    
	if (hInstanceLib != NULL) 
    { 
        // get procedure address
		fPrintProcAddr = (MYPROC) GetProcAddress(hInstanceLib, "PrintObjectLabel"); 

		if (fPrintProcAddr != NULL)
		{
			fRunTimeLinkSuccess = TRUE;
			
			// create label template file in temporary directory
			CreateSLB();
		}
		else
			fRunTimeLinkSuccess = FALSE;
	}
}

// Destructor
DatamaxPrinter::~DatamaxPrinter(void)
{
	 if (fRunTimeLinkSuccess) 
		FreeLibrary(hInstanceLib); 
}

// Print label
DWORD DatamaxPrinter::PrintLabel(
	std::wstring prefix,      // 5 digits: 02001 - ������, 04001 - ���������
	std::wstring companyCode, // 8 digits
	std::wstring barcode,     // 12 digits
	std::wstring companyName,
	std::wstring nomenclature,
	std::wstring inventoryNumber,
	std::wstring serialNumber)
{
	// split nomenclature name
	std::wstring nomen = nomenclature;
	std::wstring nomen1, nomen2, nomen3;
	int len = nomen.length();
	int chunk_len = 30;
	
	if (len == 0)
	{
		nomen1 = L"N/A";
		nomen2 = L" ";
		nomen3 = L" ";
	}
	else if (len <= chunk_len)
	{
		nomen1 = nomen.substr(0, len);
		nomen2 = L" ";
		nomen3 = L" ";
	}
	else if (len <= 2 * chunk_len)
	{
		nomen1 = nomen.substr(0, chunk_len);
		nomen2 = nomen.substr(chunk_len, len - chunk_len);
		nomen3 = L" ";
	}
	else if (len <= 3 * chunk_len)
	{
		nomen1 = nomen.substr(0, chunk_len);
		nomen2 = nomen.substr(chunk_len, len - chunk_len);
		nomen3 = nomen.substr(2 * chunk_len, len - 2 * chunk_len);
	}
	else
	{
		nomen1 = nomen.substr(0, chunk_len);
		nomen2 = nomen.substr(chunk_len, len - chunk_len);
		nomen3 = nomen.substr(2 * chunk_len, 3 * chunk_len);
	}

	// call print function from DLL
	DWORD error = (fPrintProcAddr) (
		1, // COM-port
		1, // COM-port number
		ws2s(prefix).c_str(),
		ws2s(companyCode).c_str(),
		ws2s(barcode).c_str(),
		ws2s(companyName).c_str(),
		ws2s(nomen1).c_str(), 
		ws2s(nomen2).c_str(), 
		ws2s(nomen3).c_str(), 
		ws2s(inventoryNumber).c_str(),
		ws2s(serialNumber).c_str(),
		ws2s(sTempPathBuffer).c_str()
	); 

	return error;
}


// Create label template file in temporary directory
DWORD DatamaxPrinter::CreateSLB()
{
	DWORD error = 0;
	wchar_t lpTempPathBuffer[MAX_PATH];
	DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);

	std::wstring tempPathBuffer(lpTempPathBuffer);
	sTempPathBuffer = tempPathBuffer + L"object.slb"; 

	HANDLE hWriteFile = CreateFile( 
		(wchar_t*)sTempPathBuffer.c_str(),
		GENERIC_WRITE,
		0, // the file cannot be shared
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if (hWriteFile == INVALID_HANDLE_VALUE)
	{
		return GetLastError();
	}

	std::stringstream stream;
	stream << char(2) << "m\\r\\n\r\n";
	stream << char(2) << "KW1040\\r\\n\r\n";
	stream << char(2) << "KS0005\\r\\n\r\n";
	stream << "SD\\r\\n\r\n";
	stream << char(2) << "L\\r\\n\r\n";
	stream << "C0030\\r\\n\r\n";
	stream << "R0020\\r\\n\r\n";
	stream << "H20\\r\\n\r\n";
	stream << "PC\\r\\n\r\n";
	stream << "z\\r\\n\r\n";
	stream << "D11\\r\\n\r\n";
	stream << "1911S520230001000010010%ORGNAME\\r\\n\r\n";
	stream << "1911S520200001000200020%OBJNAME1\\r\\n\r\n";
	stream << "1911S520175001000200020%OBJNAME2\\r\\n\r\n";
	stream << "1911S520150001000200020%OBJNAME3\\r\\n\r\n";
	stream << "1911S520120001000200020%OBJNAME4\\r\\n\r\n";
	stream << "1911S520095001000200020%SERIAL\\r\\n\r\n";
	stream << "1e1206500300010C%BCID%ORGID%INVNUM\\r\\n\r\n";
	stream << "1911S520000003000200020%BCID %ORGID %INVNUM\\r\\n\r\n";
	stream << "E\\r\\n\r\n";

	DWORD dwSizeHigh = 0, dwSizeLow = stream.str().length(), dwProcessBytes = 0;

	DWORD hResult = WriteFile(
		hWriteFile,
		stream.str().c_str(),
		dwSizeLow,
		&dwProcessBytes,
		NULL );

	if( (hResult != S_FALSE) || (dwSizeLow != dwProcessBytes) )
		error = 1;

	CloseHandle(hWriteFile);

	return error;
}

std::string DatamaxPrinter::ws2s(std::wstring ws)
{
    // windows-1251 code page
    static const unsigned short cp1251_unicode[256] =
        {
            0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 
            0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
            0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
            0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
            0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
            0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
            0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
            0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
            0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
            0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
            0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
            0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
            0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
            0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
            0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
            0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
            0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, 
            0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
            0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 
            0x0098, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
            0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7, 
            0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
            0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7, 
            0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
            0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 
            0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
            0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 
            0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
            0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 
            0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
            0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 
            0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F
        };

	unsigned char* buffer = new unsigned char[ws.length() + 1];
	buffer[ws.length()] = 0;

    // Encoding.GetEncoding(1251).GetBytes
    for (int pos = 0; pos < ws.length(); pos++)
    {
        // find symbol (slow)
        for (int idx = 0; idx <= 255; idx++)
            if (cp1251_unicode[idx] == ws[pos])
            {
                buffer[pos] = idx;
                break;
            }
    }


	return std::string( (char*)buffer );
}
