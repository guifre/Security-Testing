#!/bin/bash
# usage  ./checker proxylist
# output to workingproxylist

function check {
 	data=(${1//:/ });
        result=`printf "GET http://www.google.com/humans.txt  HTTP/1.0\r\n\r\n" | nc -w 20 ${data[0]} ${data[1]} 2>/dev/null`;
        if  [[ $result == *researchers* ]] ;
        then
                echo $1 >> workingproxylist4
        fi
}
counter=1;
while read line; do
        check $line &
	counter=$[$counter+1];
	if [ $counter -gt 2000 ]; then
		sleep 25;
		counter=1;
	fi
done < $1

