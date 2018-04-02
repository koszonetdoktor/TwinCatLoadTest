// ReadFromADS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "C:\TwinCAT\AdsApi\TcAdsDll\Include\TcAdsDef.h"
#include "C:\TwinCAT\AdsApi\TcAdsDll\Include\TcAdsApi.h"
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <tchar.h>

void ReadInfo(void);
void ReadByName(void);
void VarInfo(void);
void TransStruct(void);
void AccessArray(void);

// Create new struct
typedef struct PlcStruct {
	INT16 shortVal;
	INT32 intVal;
	byte byteVal;
	DOUBLE doubleVal;
	FLOAT floatVal;
} SPlcVar, *pSPlcVar;

unsigned long igroup = 16448;
unsigned long ioffset = 512028;

using namespace std;

int main()
{
	cout << "Hali vilag! jajajaj\n mitol fugg hogy jo e banyek? ";
	// le kell buildelni minden egyes alkalommal külön a C++ projektet is
	//cout << "ReadInfo! \n";
	//ReadInfo();
	//cout << "ReadByName! \n";
	//ReadByName();
	
	cout << "VarInfo! \n";
	cout << "Variable name\tData type\tLEngth\tAddress (IndexGroup / IndexOffset)\tComment\t\n";
	VarInfo();
	cout << "\nRead By name...2 \n";
	ReadByName();
	
    return 0;
}

void AccessArray(void) {
	long          nErr, nPort;
	AmsAddr       Addr;
	PAmsAddr      pAddr = &Addr;
	unsigned long lHdlVar;
	int           nIndex;
	short         Data[10];
	char          szVar[] = { "MAIN.ex_struct" };

	// Open communication port on the ADS router
	nPort = AdsPortOpen();
	nErr = AdsGetLocalAddress(pAddr);
	if (nErr) cerr << "Error: AdsGetLocalAddress: " << nErr << '\n';

	// Select Port: TwinCAT 3 PLC1 = 851
	pAddr->port = 851;

	// Fetch handle for the PLC variable 
	nErr = AdsSyncReadWriteReq(pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar);
	if (nErr) cerr << "Error: AdsSyncReadWriteReq: " << nErr << '\n';
	// Read values of the PLC variables (by handle)
	nErr = AdsSyncReadReq(pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(Data), &Data[0]);
	if (nErr)
		cerr << "Error: AdsSyncReadReq: " << nErr << '\n';
	else
	{
		for (nIndex = 0; nIndex < 10; nIndex++)
			cout << "Data[" << nIndex << "]: " << Data[nIndex] << '\n';
	}
	cout.flush();
	_getch();
	// Close communication port
	nErr = AdsPortClose();
	if (nErr) cerr << "Error: AdsPortClose: " << nErr << '\n';
}

void TransStruct(void) {
	long nErr, nPort;
	AmsAddr Addr;
	PAmsAddr pAddr = &Addr;
	ULONG lHdlVar;

	// New struct. Assign test values
	PlcStruct PlcVar;

	PlcVar.shortVal = 1;
	PlcVar.intVal = 2;
	PlcVar.byteVal = 3;
	PlcVar.doubleVal = 4.04;
	PlcVar.floatVal = (FLOAT)5.05;

	// Declare PLC variable which should notify changes
	char szVar[] = { "MAIN.PLCVar" };


	// Extract values from struct and write to byte array
	// Circumvent memory holes caused by padding
	BYTE *pData = new BYTE[19];
	int nIOffs = 0;

	memcpy_s(&pData[nIOffs], 19, &PlcVar.shortVal, 2);
	nIOffs += 2;
	memcpy_s(&pData[nIOffs], 17, &PlcVar.intVal, 4);
	nIOffs += 4;
	memcpy_s(&pData[nIOffs], 13, &PlcVar.byteVal, 1);
	nIOffs++;
	memcpy_s(&pData[nIOffs], 12, &PlcVar.doubleVal, 8);
	nIOffs += 8;
	memcpy_s(&pData[nIOffs], 4, &PlcVar.floatVal, 4);


	// Open communication port on the ADS router
	nPort = AdsPortOpen();
	nErr = AdsGetLocalAddress(pAddr);
	if (nErr) printf("Error: Ads: Open port: %d\n", nErr);

	// TwinCAT 3 PLC1 = 851
	pAddr->port = 851;

	// Get variable handle
	nErr = AdsSyncReadWriteReq(pAddr,
		ADSIGRP_SYM_HNDBYNAME,
		0x0,
		sizeof(lHdlVar),
		&lHdlVar,
		sizeof(szVar),
		szVar);

	// Write the struct to the Plc
	AdsSyncWriteReq(pAddr,
		ADSIGRP_SYM_HNDBYNAME, // IndexGroup 
		lHdlVar, // IndexOffset
		0x13, // Size of struct
		(void*)pData);
	
	if (nErr) printf("Error: Ads: Write struct: %d\n", nErr);

	// Close communication
	delete[] pData;

	//Release handle of plc variable
	nErr = AdsSyncWriteReq(pAddr, ADSIGRP_SYM_RELEASEHND, 0, sizeof(lHdlVar), &lHdlVar);
	if (nErr) printf("Error: AdsSyncWriteReq: %d \n", nErr);

	nErr = AdsPortClose();
	if (nErr) printf("Error: Ads: Close port: %d\n", nErr);

	_getch();
}

void VarInfo(void) {
	long                  nErr, nPort;
	char                  *pchSymbols = NULL;
	unsigned int                  uiIndex;
	AmsAddr               Addr;
	PAmsAddr              pAddr = &Addr;
	AdsSymbolUploadInfo   tAdsSymbolUploadInfo;
	PAdsSymbolEntry       pAdsSymbolEntry;

	// Open communication port on the ADS router
	nPort = AdsPortOpen();
	nErr = AdsGetLocalAddress(pAddr);
	if (nErr) cerr << "Error: AdsGetLocalAddress: " << nErr << '\n';

	// Select Port: TwinCAT 3 PLC1 = 851
	pAddr->port = 851;

	// Read the length of the variable declaration
	nErr = AdsSyncReadReq(pAddr, ADSIGRP_SYM_UPLOADINFO, 0x0, sizeof(tAdsSymbolUploadInfo), &tAdsSymbolUploadInfo);
	if (nErr) cerr << "Error: AdsSyncReadReq: " << nErr << '\n';
	pchSymbols = new char[tAdsSymbolUploadInfo.nSymSize];
	assert(pchSymbols);

	// Read information about the PLC variables 
	nErr = AdsSyncReadReq(pAddr, ADSIGRP_SYM_UPLOAD, 0, tAdsSymbolUploadInfo.nSymSize, pchSymbols);
	if (nErr) cerr << "Error: AdsSyncReadReq: " << nErr << '\n';

	// Output information about the PLC variables 
	pAdsSymbolEntry = (PAdsSymbolEntry)pchSymbols;
	for (uiIndex = 0; uiIndex < tAdsSymbolUploadInfo.nSymbols; uiIndex++)
	{
		cout << PADSSYMBOLNAME(pAdsSymbolEntry) << "\t\t\t" //1. give the name of the variable
			<< pAdsSymbolEntry->iGroup << '\t'	//2. give the Address' index group  
			<< pAdsSymbolEntry->iOffs << '\t'	//3. give the Address' index offset
			<< pAdsSymbolEntry->size << '\t'	//4. length of the variable
			<< PADSSYMBOLTYPE(pAdsSymbolEntry) << '\t'	//5. type of the variable
			<< PADSSYMBOLCOMMENT(pAdsSymbolEntry) << '\n';	//comment
		pAdsSymbolEntry = PADSNEXTSYMBOLENTRY(pAdsSymbolEntry); cout.flush();
	}
	_getch();

	// Close communication port
	cout << "\nclose communication port... \n";
	nErr = AdsPortClose();
	if (nErr) cerr << "Fehler: AdsPortClose: " << nErr << '\n';

	// Release memory
	if (pchSymbols) delete(pchSymbols);
}

void ReadInfo(void){
	long           nErr, nPort;
	AdsVersion     Version;
	AdsVersion     *pVersion = &Version;
	char           pDevName[50];
	AmsAddr        Addr;
	PAmsAddr       pAddr = &Addr;

	// Open communication port on the ADS router
	nPort = AdsPortOpen();
	nErr = AdsGetLocalAddress(pAddr);
	if (nErr) cerr << "Error: AdsGetLocalAddress: " << nErr << '\n';
	pAddr->port = 851;

	nErr = AdsSyncReadDeviceInfoReq(pAddr, pDevName, pVersion);
	if (nErr)
		cerr << "Error: AdsSyncReadDeviceInfoReq: " << nErr << '\n';
	else
	{
		cout << "Name: " << pDevName << '\n';
		cout << "Version: " << (int)pVersion->version << '\n';
		cout << "Revision: " << (int)pVersion->revision << '\n';
		cout << "Build: " << pVersion->build << '\n';
	}
	cout.flush();
	_getch();

	// Close the communication port
	nErr = AdsPortClose();
	if (nErr) cerr << "Error: AdsPortClose: " << nErr << '\n';
}

void ReadByName(void) {
	long      nErr, nPort;
	AmsAddr   Addr;
	PAmsAddr  pAddr = &Addr;
	ULONG     lHdlVar, nData;
	//char	  szVar[] = { "Grinder.GrindingWheelMS" };

	char      szVar[] = { "MAIN.increment" };
	int clientBuffer;
	int readData;

	// Open communication port on the ADS router
	nPort = AdsPortOpen();
	nErr = AdsGetLocalAddress(pAddr);
	if (nErr) cerr << "Error: AdsGetLocalAddress: " << nErr << '\n';

	// TwinCAT 3 PLC1 = 851
	pAddr->port = 851;

	// Fetch handle for an <szVar> PLC variable 
	nErr = AdsSyncReadWriteReq(pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar);
	//nErr = AdsSyncReadWriteReq(pAddr, igroup, ioffset, 2, &clientBuffer, sizeof(szVar), szVar);
	/*
	long __stdcall AdsSyncReadWriteReq( AmsAddr* pAddr,					// Ams address of ADS server
							unsigned long indexGroup,			//	index group in ADS server interface
							unsigned long indexOffset,			// index offset in ADS server interface
							unsigned long cbReadLength,		// count of bytes to read
							void* pReadData,						// pointer to the client buffer
							unsigned long cbWriteLength, 		// count of bytes to write
							void* pWriteData						// pointer to the client buffer
							);

	*/
	if (nErr) cerr << "Error: AdsSyncReadWriteReq: " << nErr << '\n';
	do
	{
		while (1) {
			// Read value of a PLC variable (by handle)
			nErr = AdsSyncReadReq(pAddr, igroup, ioffset, 2, &readData);
			/*
			long __stdcall AdsSyncReadReq( AmsAddr*	pAddr,						// Ams address of ADS server
			unsigned long		indexGroup,		//	index group in ADS server interface
			unsigned long		indexOffset,	// index offset in ADS server interface
			unsigned long		length,			// count of bytes to read
			void*				pData				// pointer to the client buffer
			);
			*/
			if (nErr)
				cerr << "Fehler: AdsSyncReadReq: " << nErr << '\n';
			else
				cout << "Wert: " << readData << '\n';
			cout.flush();
			readData = 0;
			_getch();
		}
		if (readData > 10)
		{
			// Reset the value of the PLC variable to 0 
			nData = 0;
			nErr = AdsSyncWriteReq(pAddr, igroup, ioffset, sizeof(readData), &readData);
			if (nErr) cerr << "Error: AdsSyncWriteReq: " << nErr << '\n';
		}
	} while (_getch() == '\r');  // read next value with RETURN, else end


								 //Release handle of plc variable
	nErr = AdsSyncWriteReq(pAddr, ADSIGRP_SYM_RELEASEHND, 0, sizeof(lHdlVar), &lHdlVar);
	if (nErr) cerr << "Error: AdsSyncWriteReq: " << nErr << '\n';

	// Close communication port
	nErr = AdsPortClose();
	if (nErr) cerr << "Error: AdsPortClose: " << nErr << '\n';
}

