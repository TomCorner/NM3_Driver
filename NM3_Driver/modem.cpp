#include "modem.h"


//**************************************************************************************************
//*** Modem class constructor
//**************************************************************************************************

Modem::Modem(wchar_t portnum) {    
    port_[7] = portnum;    
}

//**************************************************************************************************
//*** Function to get a 2-way propagation time (ping) from Nanomodem
//**************************************************************************************************

int Modem::Ping(unsigned Address) {
    //open Serial port
    HANDLE hCom;                                                                    //serial port stuff
    COMMTIMEOUTS localtimeout = { 0,0,100,0,0 }, remotetimeout = { 0,0,4500,0,0 };
    DCB dcb;

    char rxbuf[1000], command[100];            // define rx and command buffers
    unsigned long no_bytes = 0;
    unsigned TmpAddress;
    int propagation = -1;                             //returns -1 if no range measured (timed out)

    if (int serialcheck = ConfigureSerial(hCom, dcb) < 0) { // configure serial port, checks for connection errors
        return serialcheck;
    }

    sprintf_s(command, "$P%03u", Address);               // create command
    std::cout << "Command: " << command << "\n";        // display command

    SetCommTimeouts(hCom, &localtimeout);               //timeouts for local response
    WriteFile(hCom, command, 5, &no_bytes, NULL);       //send command to modem
    ReadFile(hCom, rxbuf, 7, &no_bytes, NULL);          //read local response

    if ((no_bytes == 7) && (rxbuf[0] == '$')) {         // check modem response to ping command
        
        PrintChars(&rxbuf[0], no_bytes);
        SetCommTimeouts(hCom, &remotetimeout);          //timeouts for acoustic response
        ReadFile(hCom, rxbuf, 13, &no_bytes, NULL);     //read response or time out after 4.5 s

        if ((no_bytes == 13) && (rxbuf[0] == '#') && (rxbuf[1] == 'R') && (rxbuf[5] == 'T') && (rxbuf[11] == '\r') && (rxbuf[12] == '\n')) { // check acoustic response is correct format
            PrintChars(&rxbuf[0], no_bytes);
            //decode packet
            sscanf_s(rxbuf, "#R%3uT%5u\r\n", &TmpAddress, &propagation);    // retrieve adress and propagation time from acknowledgement
            if (TmpAddress != Address) propagation = -1;                    // incorrect address
        }
        else {                                  // no acoustic response - timeout
            PrintChars(&rxbuf[0], no_bytes);
        }
    } else if ((rxbuf[0] == 'E')) {             // error in command
        PrintChars(&rxbuf[0], no_bytes);
        std::cout << "\n*****\nError: Modem given unrecognised command.\n*****\n";
    } else {
        std::cout << "\n*****\nError: Modem not connected - check connections and serial port settings.\n*****\n";
        propagation = -2;                          //-2 returned when modem not connected
    }

    CloseHandle(hCom);
    return(propagation);
}

//**************************************************************************************************
//*** Function to enable modems system time
//**************************************************************************************************

int Modem::SysTimeEnable() {
    //open Serial port
    HANDLE hCom;                                    //serial port stuff
    COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };
    DCB dcb;

    char rxbuf[1000], command[100];            // define rx and command buffers
    unsigned long no_bytes = 0;
    int systime = -1;                             //returns -1 if no time measured (timed out)

    if (int serialcheck = ConfigureSerial(hCom, dcb) < 0) { // configure serial port, checks for connection errors
        return serialcheck;
    }

    sprintf_s(command, "$XTE");                         // create command
    std::cout << "Command: " << command << "\n";        // display command

    SetCommTimeouts(hCom, &localtimeout);               //timeouts for local response
    WriteFile(hCom, command, 4, &no_bytes, NULL);       //send command to modem
    ReadFile(hCom, rxbuf, 20, &no_bytes, NULL);         //read local response

    if ((no_bytes == 20) && (rxbuf[0] == '#') && (rxbuf[1] == 'X') && (rxbuf[2] == 'T') && (rxbuf[3] == 'E') && (rxbuf[18] == '\r') && (rxbuf[19] == '\n')) { // check local response is correct format
        PrintChars(&rxbuf[0], no_bytes);
        sscanf_s(rxbuf, "#XTE%14u\r\n", &systime);      //retrieve time from local response    
    } else if ((rxbuf[0] == 'E')) {
        PrintChars(&rxbuf[0], no_bytes);
        std::cout << "\n*****\nError: Modem given unrecognised command.\n*****\n";
    } else {
        std::cout << "\n*****\nError: Modem not connected - check connections and serial port settings.\n*****\n";
        systime = -2;                          //-2 returned when modem not connected
    }

    CloseHandle(hCom);
    return(systime);
}

//**************************************************************************************************
//*** Function to get modems system time - flag indicates with system time is enabled or diabled
//**************************************************************************************************

int Modem::SysTimeGet(char& flag) {
    //open Serial port
    HANDLE hCom;                                    //serial port stuff
    COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };
    DCB dcb;

    char rxbuf[1000], command[100];            // define rx and command buffers
    unsigned long no_bytes = 0;
    int systime = -1;                             //returns -1 if no time measured (timed out)

    if (int serialcheck = ConfigureSerial(hCom, dcb) < 0) { // configure serial port, checks for connection errors
        return serialcheck;
    }

    sprintf_s(command, "$XTG");                         // create command
    std::cout << "Command: " << command << "\n";        // dsiplay command

    SetCommTimeouts(hCom, &localtimeout);              //timeouts for local response
    WriteFile(hCom, command, 4, &no_bytes, NULL);      //send command to modem
    ReadFile(hCom, rxbuf, 20, &no_bytes, NULL);        //read local response

    if ((no_bytes == 20) && (rxbuf[0] == '#') && (rxbuf[1] == 'X') && (rxbuf[2] == 'T') && (rxbuf[18] == '\r') && (rxbuf[19] == '\n')) { // check local response is correct format
        PrintChars(&rxbuf[0], no_bytes);
        sscanf_s(rxbuf, "#XT%c%14u\r\n", &flag, 1, &systime);      //retrieve time from local response
    }
    else if ((rxbuf[0] == 'E')) {
        PrintChars(&rxbuf[0], no_bytes);
        std::cout << "\n*****\nError: Modem given unrecognised command.\n*****\n";
    }
    else {
        std::cout << "\n*****\nError: Modem not connected - check connections and serial port settings.\n*****\n";
        systime = -2;                          //-2 returned when modem not connected
    }

    CloseHandle(hCom);
    return(systime);
}

//**************************************************************************************************
//*** Function to disable modems system time
//**************************************************************************************************

int Modem::SysTimeDisable() {
    //open Serial port
    HANDLE hCom;                                    //serial port stuff
    COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };
    DCB dcb;

    char rxbuf[1000], command[100];            // define rx and command buffers
    unsigned long no_bytes = 0;
    int systime = -1;                             //returns -1 if no time measured (timed out)

    if (int serialcheck = ConfigureSerial(hCom, dcb) < 0) { // configure serial port, checks for connection errors
        return serialcheck;
    }

    sprintf_s(command, "$XTD");                         // create command
    std::cout << "Command: " << command << "\n";        // display command

    SetCommTimeouts(hCom, &localtimeout);               //timeouts for local response
    WriteFile(hCom, command, 4, &no_bytes, NULL);       //send command to modem
    ReadFile(hCom, rxbuf, 20, &no_bytes, NULL);         //read local response

    if ((no_bytes == 20) && (rxbuf[0] == '#') && (rxbuf[1] == 'X') && (rxbuf[2] == 'T') && (rxbuf[3] == 'D') && (rxbuf[18] == '\r') && (rxbuf[19] == '\n')) { // check local response is correct format
        PrintChars(&rxbuf[0], no_bytes);
        sscanf_s(rxbuf, "#XTD%14u\r\n", &systime);      //retrieve time from local response    
    }
    else if ((rxbuf[0] == 'E')) {
        PrintChars(&rxbuf[0], no_bytes);
        std::cout << "\n*****\nError: Modem given unrecognised command.\n*****\n";
    }
    else {
        std::cout << "\n*****\nError: Modem not connected - check connections and serial port settings.\n*****\n";
        systime = -2;                          //-2 returned when modem not connected
    }

    CloseHandle(hCom);
    return(systime);
}

//**************************************************************************************************
//*** Function to send unicast message to given address
//**************************************************************************************************

int Modem::SendUnicast(unsigned address, char message[], unsigned messagelength, int txtime) {
    //open Serial port
    HANDLE hCom;                                    //serial port stuff
    COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };
    DCB dcb;

    char rxbuf[1000], command[100];            // define rx and command buffers
    unsigned long no_bytes = 0;
    int check = -1;                             //returns -1 if no time measured (timed out)

    if (int serialcheck = ConfigureSerial(hCom, dcb) < 0) { // configure serial port, checks for connection errors
        return serialcheck;
    }

    unsigned commandlength = 0;
    if (txtime == 0) {
        commandlength = 7 + messagelength;
        sprintf_s(command, "$U%03u%02u%s", address, messagelength, message);  // create command
    }else {
        commandlength = 21 + messagelength;
        sprintf_s(command, "$U%03u%02u%s%014u", address, messagelength, message, txtime);  // create command
    }
    std::cout << "Command: " << command << "\n";                                    // display command

    SetCommTimeouts(hCom, &localtimeout);                                           //timeouts for local response
    WriteFile(hCom, command, commandlength, &no_bytes, NULL);                       //send command to modem
    ReadFile(hCom, rxbuf, 9, &no_bytes, NULL);                                      //read local response

    if ((no_bytes == 9) && (rxbuf[0] == '$') && (rxbuf[1] == 'U')) {
        PrintChars(&rxbuf[0], no_bytes);
        check = 0;
    } else if ((rxbuf[0] == 'E')) {
        PrintChars(&rxbuf[0], no_bytes);
        std::cout << "\n*****\nError: Modem given unrecognised command.\n*****\n";
    } else {
        std::cout << "\n*****\nError: Modem not connected - check connections and serial port settings.\n*****\n";
        check = -2;                          //-2 returned when modem not connected
    }

    CloseHandle(hCom);
    return check;
}

//**************************************************************************************************
//*** Function to send broadcast message
//**************************************************************************************************

int Modem::SendBroadcast(char message[], unsigned messagelength, int txtime) {
    //open Serial port
    HANDLE hCom;                                    //serial port stuff
    COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };
    DCB dcb;

    char rxbuf[1000], command[100];            // define rx and command buffers
    unsigned long no_bytes = 0;
    int check = -1;                             //returns -1 if no time measured (timed out)

    if (int serialcheck = ConfigureSerial(hCom, dcb) < 0) { // configure serial port, checks for connection errors
        return serialcheck;
    }


    unsigned commandlength = 0;
    if (txtime == 0) {
        commandlength = 4 + messagelength;
        sprintf_s(command, "$B%02u%s", messagelength, message);  // create command
    }
    else {
        commandlength = 18 + messagelength;
        sprintf_s(command, "$B%02u%s%014u", messagelength, message, txtime);  // create command
    }
    std::cout << "Command: " << command << "\n";                                    // display command

    SetCommTimeouts(hCom, &localtimeout);                                           //timeouts for local response
    WriteFile(hCom, command, commandlength, &no_bytes, NULL);                       //send command to modem
    ReadFile(hCom, rxbuf, 6, &no_bytes, NULL);                                      //read local response

    if ((no_bytes == 6) && (rxbuf[0] == '$') && (rxbuf[1] == 'B')) {
        PrintChars(&rxbuf[0], no_bytes);
        check = 0;
    }
    else if ((rxbuf[0] == 'E')) {
        PrintChars(&rxbuf[0], no_bytes);
        std::cout << "\n*****\nError: Modem given unrecognised command.\n*****\n";
    }
    else {
        std::cout << "\n*****\nError: Modem not connected - check connections and serial port settings.\n*****\n";
        check = -2;                          //-2 returned when modem not connected
    }

    CloseHandle(hCom);
    return 0;
}

//**************************************************************************************************
//*** Function to send channel probe
//**************************************************************************************************

int Modem::Probe(unsigned chirpcount, Chirp chirpinfo, int txtime) {
    //open Serial port
    HANDLE hCom;                                    //serial port stuff
    COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };
    DCB dcb;

    char rxbuf[1000], command[100];            // define rx and command buffers
    unsigned long no_bytes = 0;
    int check = -1;                             //returns -1 if no time measured (timed out)

    if (int serialcheck = ConfigureSerial(hCom, dcb) < 0) { // configure serial port, checks for connection errors
        return serialcheck;
    }


    // STUFF


    CloseHandle(hCom);
    return 0;
}

//**************************************************************************************************
//*** Function to configure serial port (common across modem commands)
//**************************************************************************************************

int Modem::ConfigureSerial(HANDLE& hCom, DCB& dcb) {

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

    return 0; // return 0 if no errors
}