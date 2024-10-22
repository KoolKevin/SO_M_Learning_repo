A pipe is a small __kernel buffer__ exposed to processes as a pair of file descriptors, one for reading and one for writing. Writing data to one end of the pipe makes that data available for reading from the other end of the pipe. Pipes provide a way for processes to communicate.

Pipes may seem no more powerful than temporary files, the pipeline:

    echo hello world | wc

could be implemented without pipes as

    echo hello world >/tmp/xyz; wc </tmp/xyz

Pipes have at least __three advantages__ over temporary files in this situation.
- First, pipes automatically clean themselves up; with the file redirection, a shell would have to be careful to remove /tmp/xyz when done.
- Second, pipes can pass arbitrarily long streams of data, while file redirection requires enough free space on disk to store all the data.
- Third, pipes allow for __parallel execution__ of pipeline stages, while the file approach requires the first program to finish before the second starts.
