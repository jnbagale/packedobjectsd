#!/bin/bash
echo "POD test"
init()
{ 
    schemafile=../schema/personnel.xsd
    xmlfile=../schema/personnel.xml

    send-receive()
    {
	for i in {1..1}
	do
	    ./../src/packedobjectsdtest --xml $xmlfile --schema $schemafile  > pod-result.log 2>&1
	done
    }
 
    send-receive
    cat pod-result.log
}
init
rm pod-result.log	

