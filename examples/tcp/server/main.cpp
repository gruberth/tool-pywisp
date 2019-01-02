#define _BUR_USE_DECLARATION_IN_IEC
#ifdef _DEFAULT_INCLUDES
	#include "AsDefault.h"
#endif
#include "../Libraries/TCPServer/TCPServer.h"
#include "../Libraries/EventLog/EventLog.h"
#include "../Libraries/Transport/Transport.h"

/**< amount of memory to be allocated for heap storage must be specified for
     every ANSI C++ program with the bur_heap_size variable */
unsigned long bur_heap_size = 0xFFFFF;

/**< Zählervariablen*/
int counter_close = 0;
int counter_tcp = 0;
/**< Instanz des Eventloggers*/
EventLogger EvLog_1((unsigned long) &strlogbook, &EvLogInstance_1);
/**< Instanz der Tcp Server Klasse*/
TCPServer TCPS_0(50007, &TcpServer_instance_0,
				&EvLog_1);

/**< Instanz der Transport Klasse*/
Transport transport;

void _INIT ProgramInit(void)
{
	Main_state = 0;
	counter_close = 0;
	counter_tcp = 0;
	TCPS_0.registerListener(&transport);
	transport.registerServer(&TCPS_0);
	TCPS_0.deinit();
}

void _CYCLIC ProgramCyclic(void)
{
    switch (Main_state)
    {
        case 0:
            /**< Starte Server und warte auf Client*/
            TCPS_0.init();
            if (TCPS_0.status == TCPServer::READY)
                Main_state = 1;
            else if ((TCPS_0.status == TCPServer::STOP) || (TCPS_0.status == TCPServer::ERROR))
                Main_state = 255;
            break;
        case 1:
            /**< Lese und Schreibe mit Client*/
            TCPS_0.sync();
            if (TCPS_0.status == TCPServer::READY)
                Main_state = 1;
            else if ((TCPS_0.status == TCPServer::STOP) || (TCPS_0.status == TCPServer::ERROR))
                Main_state = 10;
            break;
        case 10:
            /**< Schließe Verbindung*/
            TCPS_0.close_sockets();
            if (TCPS_0.status == TCPServer::READY)
                Main_state = 11;
            break;
        case 11:
            /**< Warte 5 Sekunden und starte Server erneut*/
            if (counter_close > 500)
            {
                counter_close = 0;
                Main_state = 0;
            }
            else
                counter_close += 1;
            break;
        default:
            break;
    }
    /**< Sende alle 100 ms Messdaten zu Pywisp*/
	if (transport.runExp()) {
    	if (counter_tcp >= 10) {
            transport.sendData();
            transport.lTime += 100;
			counter_tcp = 0;
		} else {
			counter_tcp += 1;
        }
	}
}

void _EXIT ProgramExit(void)
{
	// Insert code here
    TCPS_0.close_sockets();
}