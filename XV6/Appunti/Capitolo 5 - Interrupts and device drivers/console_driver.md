The console driver accepts characters typed by a human, via the UART serial-port hardware attached to the RISC-V cpu. **The console driver accumulates a line of input at a time**, processing special input characters such as backspace and control-u (kill line). User processes, such as the shell, use the **read system call to fetch lines of input from the console**. **When you type input to xv6 in QEMU, your keystrokes are delivered to xv6 by way of QEMU’s simulated UART hardware**.

The UART hardware that the driver talks to is a 16550 chip [13] emulated by QEMU. On a real computer, a 16550 would manage an RS232 serial link connecting to a terminal or other computer.

**When running QEMU, the UART it’s connected to your keyboard and display.** The UART hardware appears to software as a set of **memory-mapped** control registers. There are a handful of UART control registers, each the width of a byte. 





### UART control registers examples
For example, the LSR (Line Status Register) register contains bits that indicate whether input characters are waiting to be read by the software. These characters (if any) are available for reading from the RHR (Receive Holding Register) register. Each time one is read, the UART hardware deletes it from an internal FIFO of waiting characters, and clears the “ready” bit in LSR when the FIFO is empty.

The UART transmit hardware is largely independent of the receive hardware; if software writes a byte to the THR (Transmit Holding Register), the UART transmits that byte.


# IMPORTANTE
- l'input della tastiera (read("console")) finisce automaticamente dentro ad una coda FIFO dell'UART consumabile un byte alla volta leggendo RHR
- l'output sullo schermo (write("console)) viene prodotto automaticamente scrivendo un byte alla volta dentro a THR

**conclusione**: l'UART è responsabile di comunicare allo schermo i caratteri da mostrare e di recuperare i caratteri digitati sulla tastiera


# IMPORTANTE
Xv6’s main calls _consoleinit_ to initialize the UART hardware. This code configures the UART to **generate a receive interrupt when the UART receives each byte of input**, and a **transmit complete interrupt each time the UART finishes sending a byte of output**.

**conclusioni**:
- ogni carattere digitato finisce dentro alla coda del UART -> che è configurato per generare un interrupt ogni volta che riceve un byte di input
    -> **ogni pressione della tastiera genera un receive interrupt!**
- UART predispone un registro di trasmissione scrivibile da software -> UART è configurato per generare un interrupt ogni volta che il contenuto del registro è stato trasmesso con successo -> la destinazione della trasmissione è il monitor
    -> **ogni volta che scrivo THT è come se stessi scrivendo sul monitor (dopo che ho aspettato l'interrupt)**









### Console input
Calls to the read system call make their way through the kernel to _consoleread_ (funzione registrata da consoleinit()).

_consoleread_:
- waits for **input to arrive via interrupts** (pressioni della tastiera) and be buffered in _cons.buf_
- copies the input to user space
- and, after a whole line has arrived, returns to the user process.

If the user hasn’t typed a full line yet, any reading processes will wait in the sleep call.



When the user types a character, the UART hardware asks the RISC-V to raise an interrupt, which activates xv6’s trap handler. The trap handler calls *devintr*, which in turn calls, **uartintr**. 

uartintr reads any waiting input characters from the UART hardware and hands them to **consoleintr**; it doesn’t wait for characters, since future input will raise a new interrupt


The job of *consoleintr* is to accumulate input characters in cons.buf until a whole line arrives. consoleintr treats backspace and a few other characters specially. When a newline arrives, consoleintr wakes up a waiting **consoleread** (if there is one).  Once woken, consoleread will observe a full line in cons.buf, copy it to user space, and return (via the system call machinery) to user space.
- notare nel codice che consolintr gestisce solo le interruzioni causate dall'aver digitato. Le transmission completed interrupts sono gestite da uartintr




### Console output
dentro a consoleintr viene fatto l'echo di ogni carattere digitato. 

Possiamo scrivere sulla console (e quindi sul monitor) anche da programma (vedi printf()). 


A write system call on a file descriptor connected to the console eventually arrives at **uartputc** tramite **consolewrite**. The device driver maintains a (software) output buffer (uart_tx_buf) so that writing processes do not have to wait for the UART to finish sending; instead, uartputc appends each character to the buffer, calls **uartstart to start the device transmitting** (if it isn’t already), and returns. The only situation in which uartputc waits is if the buffer is already full.


Each time the UART finishes sending a byte, it generates an interrupt. uartintr calls uartstart, which checks that the device really has finished sending, and hands the device the next buffered output character. Thus **if a process writes multiple bytes to the console**, typically the first byte will be sent by uartputc’s call to uartstart, and **the remaining buffered bytes will be sent by uartstart calls from uartintr as transmit complete interrupts arrive**.
- uartstart ritorna se l'uart è occupato a trasmettere; non trasmette l'intero buffer tutto in una volta
- non ci sono quindi più thread che fanno uartstart, è una catena di uartstart che parte con il primo uartputc 


