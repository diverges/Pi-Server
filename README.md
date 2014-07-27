# Pi Server

Basic server designed to manage python application on a Raspberry Pi. Meant to allow client applications on same network to initiate various programs on the Pi. Listens on port 2525.

Author: Miguel Sotolongo (diverges)  

## Commands
**exit:** terminate client connection

**shutdown:** shutdown server, no jobs must be running

**echo <message>:** returns message to client

**jobs:** lists all currently running jobs server side

**exec <py/file.py> [arg0] [arg1]...:** executes file.py with arguments

**kill <PID>:** kill job with PID

## Testing
Make the server and execute the generated 'pi_server'.

Telnet can be used to connect to the server on localhost:

telnet localhost 2525

Trying ::1...

Connection failed: Connection refused

Trying 127.0.0.1...

Connected to localhost.

Escape character is '^]'.

Welcome! Type 'exit' to terminate connection.

echo Testing Server

Testing Server

exec py/sleep.py 35

exec py/sleep.py 45

jobs

shutdown

** Server should be shutdown from client application. **

## What's next?
