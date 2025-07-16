### Buffering in device drivers
A general pattern to note is the decoupling of device activity from process activity via buffering and interrupts.

The console driver can process input even when no process is waiting to read it; a subsequent read will see the input. Similarly, processes can send output without having to wait for the device. 

This decoupling can increase performance by allowing **processes to execute concurrently with device I/O**, and is particularly important when the device is slow (as with the UART, non voglio aspettare la fine della trasmissione quando faccio una write) or needs immediate attention (as with echoing typed characters). This idea is sometimes called I/O concurrency.