**The UART driver retrieves data a byte at a time** by reading the UART control registers; this pattern is called **programmed I/O**, since software is driving the data movement. Programmed I/O is simple, but **too slow** to be used at high data rates.

Devices that need to **move lots of data at high speed** typically use **direct memory access (DMA)**. DMA device hardware **directly writes incoming data to RAM, and reads outgoing data from RAM**. 
- Modern disk and network devices use DMA.
- A driver for a DMA device would prepare data in RAM, and then use a single write to a control register to tell the device to process the prepared data