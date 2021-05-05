IP=$1
PROTOCOL=${2:-tcp}
TYPE=${3:-simple}

if [ "$PROTOCOL" == "tcp" ]
then
    if [ "$TYPE" == "scaled" ]
    then
        echo test | nc $IP 8001 -w1
    else
        echo test | nc $IP 9001 -w1
    fi
else
    if [ "$TYPE" == "scaled" ]
    then
        echo test | nc -u $IP 8002 -w1
    else
        echo test | nc -u $IP 5001 -w1
    fi
fi
