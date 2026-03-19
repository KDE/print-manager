#!/bin/sh

TESTDIR=mock/print

for i in $TESTDIR/*.conf; do
    pname=$(grep "printer-name" $i | awk '{print $4}')
    desc=$(grep "printer-info" $i | awk -v FPAT='"[^"]+"' '{print $1}')
    filename="${i%.*}"
    filename="${filename##*/}"
    uri=ipp://localhost:8000/ipp/print/${filename}
    echo Creating printer: "$pname => $uri"
    sudo lpadmin -p ${pname//\"/} -D "${desc//\"/}" -E -v $uri -m everywhere
done
