#!/bin/sh

# This test can only run if netcat is present
#
if ! type nc >/dev/null; then exit 77; fi

PORT=54325			# Some port that is unlikely to be in use

# Start 11 "servers" that redirect to each other
#
# There appear to be three versions of nc: (1) option -p is not
# allowed when -l is present (Mac OS X); (2) -p is required when -l is
# present (GNU netcat, traditional Hobbit version); and (3) -p is
# optional (recent OpenBSD versions).
#
i=-1
while [ $(( ( i += 1 ) <= 10 )) -ne 0 ]; do
  p=$(($PORT + $i))
  q=$(($p + 1))
  printf "HTTP/1.1 302\r\nLocation: http://127.0.0.1:$q/\r\n\r" |\
    (nc -q0 -l -n -p $p || nc -l -n $p) &
done

sleep 1				# Give the servers time to start

./hxextract body http://127.0.0.1:$PORT/ 2>&1 | grep -q 'Too many links'

code=$?

kill %1
kill %2
kill %3
kill %4
kill %5
kill %6
kill %7
kill %8
kill %9
kill %10
kill %11
wait

exit $code
