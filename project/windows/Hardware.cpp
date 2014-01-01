#include <string>
using std::string;




#include "stdafx.h"

#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>

#include <hx/CFFI.h>




# pragma comment(lib, "wbemuuid.lib")
namespace hardware {
	
	
std::string GetHarddiskSerialNumber()
{
		HRESULT hres;

		// No need to initialize COM in openFL, as it seems that its already initialized when starting windows
		// Step 1: --------------------------------------------------
		// Initialize COM. ------------------------------------------
        
		hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
		if (FAILED(hres))
		{
			cout << "Failed to initialize COM library. Error code = 0x" 
				<< hex << hres << endl;
			return "Failed to initialize COM library. Error code = 0x";                  // Program has failed.
		}
        
		// Step 2: --------------------------------------------------
		// Set general COM security levels --------------------------
		// Note: If you are using Windows 2000, you need to specify -
		// the default authentication credentials for a user by using
		// a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
		// parameter of CoInitializeSecurity ------------------------

		hres =  CoInitializeSecurity(
			NULL, 
			-1,                          // COM authentication
			NULL,                        // Authentication services
			NULL,                        // Reserved
			RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
			RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
			NULL,                        // Authentication info
			EOAC_NONE,                   // Additional capabilities 
			NULL                         // Reserved
			);

						  
		if (FAILED(hres))
		{
			cout << "Failed to initialize security. Error code = 0x" 
				<< hex << hres << endl;
			CoUninitialize();
			return "Failed to initialize security. Error code = 0x";                    // Program has failed.
		}
		
		// Step 3: ---------------------------------------------------
		// Obtain the initial locator to WMI -------------------------

		IWbemLocator *pLoc = NULL;

		hres = CoCreateInstance(
			CLSID_WbemLocator,             
			0, 
			CLSCTX_INPROC_SERVER, 
			IID_IWbemLocator, (LPVOID *) &pLoc);
	 
		if (FAILED(hres))
		{
			cout << "Failed to create IWbemLocator object."
				<< " Err code = 0x"
				<< hex << hres << endl;
			CoUninitialize();
			return "1";                 // Program has failed.
		}

		// Step 4: -----------------------------------------------------
		// Connect to WMI through the IWbemLocator::ConnectServer method

		IWbemServices *pSvc = NULL;
	 
		// Connect to the root\cimv2 namespace with
		// the current user and obtain pointer pSvc
		// to make IWbemServices calls.
		hres = pLoc->ConnectServer(
			 _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
			 NULL,                    // User name. NULL = current user
			 NULL,                    // User password. NULL = current
			 0,                       // Locale. NULL indicates current
			 NULL,                    // Security flags.
			 0,                       // Authority (for example, Kerberos)
			 0,                       // Context object 
			 &pSvc                    // pointer to IWbemServices proxy
			 );
		
		if (FAILED(hres))
		{
			cout << "Could not connect. Error code = 0x" 
				 << hex << hres << endl;
			pLoc->Release();     
			CoUninitialize();
			return "1";                // Program has failed.
		}

		//cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


		// Step 5: --------------------------------------------------
		// Set security levels on the proxy -------------------------

		hres = CoSetProxyBlanket(
		   pSvc,                        // Indicates the proxy to set
		   RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		   RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		   NULL,                        // Server principal name 
		   RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		   RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		   NULL,                        // client identity
		   EOAC_NONE                    // proxy capabilities 
		);

		if (FAILED(hres))
		{
			cout << "Could not set proxy blanket. Error code = 0x" 
				<< hex << hres << endl;
			pSvc->Release();
			pLoc->Release();     
			CoUninitialize();
			return "1";               // Program has failed.
		}

		// Step 6: --------------------------------------------------
		// Use the IWbemServices pointer to make requests of WMI ----

		// For example, get the name of the operating system
		IEnumWbemClassObject* pEnumerator = NULL;
		hres = pSvc->ExecQuery(
			bstr_t("WQL"), 
			bstr_t("SELECT * FROM Win32_OperatingSystem"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
			NULL,
			&pEnumerator);
		
		if (FAILED(hres))
		{
			cout << "Query for operating system name failed."
				<< " Error code = 0x" 
				<< hex << hres << endl;
			pSvc->Release();
			pLoc->Release();
			CoUninitialize();
			return "1";               // Program has failed.
		}

		// Step 7: -------------------------------------------------
		// Get the data from the query in step 6 -------------------
	 
		IWbemClassObject *pclsObj;
		ULONG uReturn = 0;
		std::string result= "";
		while (pEnumerator)
		{
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
				&pclsObj, &uReturn);

			if(0 == uReturn)
			{
				break;
			}

			VARIANT vtProp;

			// Get the value of the Name property
			hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
			wstring ws(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
			result = string(ws.begin(), ws.end());
			VariantClear(&vtProp);

			hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
			wstring ws2(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
			result+=",";
			result+= string(ws2.begin(), ws2.end());
			VariantClear(&vtProp);

			

			

			pclsObj->Release();
		}

		// Cleanup
		// ========
		
		pSvc->Release();
		pLoc->Release();
		pEnumerator->Release();
		//pclsObj->Release();
		CoUninitialize();
		return result;
		
	}
}// name space

