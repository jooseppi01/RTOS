*** Settings ***
Library  SerialLibrary

*** Variables ***
${com}   COM7
${board}   nRF5340

${ledon}   000001X
${return}	1X

${error}   000066X
${error_return}	-1X


*** Test Cases ***
Connect Serial
	Log To Console  Connecting to ${com} ${board}
	Add Port  ${com}  baudrate=115200   encoding=ascii
	Port Should Be Open  ${com}
	Reset Input Buffer
	Reset Output Buffer

Correct_out
	Send Led On
	${read} =   Read Until  terminator=58  encoding=ascii
	Log To Console  Received "${read}"
	#Should Be Equal As Integers  ${read}  ${return}
	Should Be Equal As Strings  ${read}  ${return}

Sleep 3s
	Log To Console  Sleeping for 3s
	Sleep  3s

incorrect_out
	Send error_data
	${read} =   Read Until  terminator=58  encoding=ascii
	Log To Console  Received "${read}"
	#Should Be Equal As Integers  ${read}  ${error_return}
	Should Be Equal As Strings  ${read}  ${error_return}


Disconnect Serial
	Log To Console  Disconnecting ${board}
	[TearDown]  Delete Port  ${com}

*** Keywords ***
Send Led On
	Log To Console  Send Led ON
	Write Data  ${ledon}  encoding=ascii
	Log To Console  Send Command ${ledon} to ${com}

Send error_data
	Log To Console  error_data
	Write Data  ${error}  encoding=ascii
	Log To Console  Send Command ${error} to ${com}





