Exercise: Communication CAN between 2 STM32F767ZI (Normal Mode)



Purpose 



\*MCU A -> Transmit a message each 500 ms with a counter.

\*MCU B -> Receive the message, read the counter and turn on/off a LED depending of the value



This confirms:



\*A correct GPIO(TX/RX) configuration

\*A correct bit timing

\*Real transmission and reception on CAN bus 

\*Filtering working



Logic in MCU A (Transmitter)

\*CAN1 configured as Normal Mode

\*Filtering configures as List Mode to accept specific IDEs

\*Each 500 ms 

&nbsp; \*Increment a counter from 0x00 to 0xFF

&nbsp; \*Upload a mailbox with:

&nbsp;   \*Standard ID: 0x123

&nbsp;   \*DLC: 1

&nbsp;   \*DATA\[0]: counter 

&nbsp; \*Using CAN1\_TX\_IRQHandler to know when transmit another message



&nbsp;

&nbsp; 

Logic in MCU B (Receiver)

\*Configure CAN as normal mode

\*Filtering configures as List Mode to accept specific IDEs

\*Read FIFO
\*If the ID is 0x123, take the value

\*If the value is even - LED ON

\*If the value is odd - LED OFF 





USART3 is used with interruptions to debug the value received

TIMER4 is used to implement a independent counter without lock CPU

