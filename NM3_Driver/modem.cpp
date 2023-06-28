#include "modem.h"


//**************************************************************************************************
//*** Modem class constructor
//**************************************************************************************************

Modem::Modem(wchar_t port[4], unsigned long baudrate):baudrate_(baudrate) {
    for (int i = 0; i < 4; i++) {
        port_[i] = port[i];
    }
}

//**************************************************************************************************
//*** Function to get a 2-way propagation time (ping) from Nanomodem
//**************************************************************************************************

int Modem::Ping(unsigned Address) {
    //open Serial port
    HANDLE hCom;                                                                                                //serial port stuff
    COMMTIMEOUTS localtimeout = { 0,0,100,0,0 }, remotetimeout = { 0,0,4500,0,0 };
    DCB dcb;

    char rxbuf[1000], command[100];            // define rx and command buffers
    unsigned long no_bytes = 0;
    unsigned TmpAddress;
    int propagation = -1;                             //returns -1 if no range measured (timed out)

    hCom = CreateFile(port_,
        GENERIC_READ | GENERIC_WRITE, 0,    /* comm devices must be opened w/exclusive-access */
        NULL,                               /* no security attrs */
        OPEN_EXISTING,                      /* comm devices must use OPEN_EXISTING */
        0,                                  /* not overlapped I/O */
        NULL                                /* hTemplate must be NULL for comm devices */
    );

    if (hCom == INVALID_HANDLE_VALUE) {
        std::cout << "\n*****\nError: Cannot open Modem COM port.\n*****\n";
        return(-2);
    }

    if (!GetCommState(hCom, &dcb)) {
        std::cout << "\n*****\nError: Cannot get DCB for Modem COM port.\n*****\n";
        return(-2);
    }

    dcb.BaudRate = baudrate_;                                         //for Nanomodem (via RS232)
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(hCom, &dcb)) {
        std::cout << "\n*****\nError: Cannot set DCB for Modem COM port.\n*****\n";
        return(-2);
    }

    PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    sprintf_s(command, "$P%3u", Address);

    SetCommTimeouts(hCom, &localtimeout);             //timeouts for local response
    WriteFile(hCom, command, 5, &no_bytes, NULL);     //send command to modem
    ReadFile(hCom, rxbuf, 7, &no_bytes, NULL);        //read local response

    if ((no_bytes == 7) && (rxbuf[0] == '$')) {
        SetCommTimeouts(hCom, &remotetimeout);        //timeouts for acoustic response
        ReadFile(hCom, rxbuf, 13, &no_bytes, NULL);   //read response or time out after 4.5 s

        if ((no_bytes == 13) && (rxbuf[0] == '#') && (rxbuf[1] == 'R') && (rxbuf[5] == 'T') && (rxbuf[11] == '\r') && (rxbuf[12] == '\n')) { // check acoustic response is correct format        
            //decode packet
            sscanf_s(rxbuf, "#R%3uT%5u\r\n", &TmpAddress, &propagation);    // retrieve adress and propagation time from acoustic response
            if (TmpAddress != Address) propagation = -1;                    // incorrect address
        }
    } else {
        std::cout << "\n*****\nError: Modem not connected - check connections and serial port settings.\n*****\n";
        propagation = -2;                          //-2 returned when modem not connected
    }

    CloseHandle(hCom);

    return(propagation);
}

//**************************************************************************************************
//*** Function to switch on modems system time
//**************************************************************************************************

int Modem::SysTimeEnable() {
    //open Serial port
    HANDLE hCom;                                                                                                //serial port stuff
    COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };
    DCB dcb;

    char rxbuf[1000], command[100];            // define rx and command buffers
    unsigned long no_bytes = 0;
    int systime = -1;                             //returns -1 if no range measured (timed out)

    hCom = CreateFile(port_,
        GENERIC_READ | GENERIC_WRITE, 0,    /* comm devices must be opened w/exclusive-access */
        NULL,                               /* no security attrs */
        OPEN_EXISTING,                      /* comm devices must use OPEN_EXISTING */
        0,                                  /* not overlapped I/O */
        NULL                                /* hTemplate must be NULL for comm devices */
    );

    if (hCom == INVALID_HANDLE_VALUE) {
        std::cout << "\n*****\nError: Cannot open Modem COM port.\n*****\n";
        return(-2);
    }

    if (!GetCommState(hCom, &dcb)) {
        std::cout << "\n*****\nError: Cannot get DCB for Modem COM port.\n*****\n";
        return(-2);
    }

    dcb.BaudRate = baudrate_;                                         //for Nanomodem (via RS232)
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(hCom, &dcb)) {
        std::cout << "\n*****\nError: Cannot set DCB for Modem COM port.\n*****\n";
        return(-2);
    }

    PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    sprintf_s(command, "$XTE");

    SetCommTimeouts(hCom, &localtimeout);              //timeouts for local response
    WriteFile(hCom, command, 4, &no_bytes, NULL);      //send command to modem
    ReadFile(hCom, rxbuf, 20, &no_bytes, NULL);        //read local response

    if ((no_bytes == 20) && (rxbuf[0] == '#') && (rxbuf[1] == 'X') && (rxbuf[2] == 'T') && (rxbuf[3] == 'E') && (rxbuf[18] == '\r') && (rxbuf[19] == '\n')) { // check local response is correct format
            
        sscanf_s(rxbuf, "#XTE%14u\r\n", &systime);      //retrieve time from local response
    } else {
        std::cout << "\n*****\nError: Modem not connected - check connections and serial port settings.\n*****\n";
        systime = -2;                          //-2 returned when modem not connected
    }

    CloseHandle(hCom);
    return(systime);
}