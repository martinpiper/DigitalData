# Digital data device for Proteus simulation software

## Using

TestJig/Design.pdsprj : Contains an example schematic that tests the digital data device.

!!Debugging note!!
When trying to debug this, always use the Proteus Start simulation button or use the menu Proteus->Debug->start option. Don't use the F12 key as this causes an exception.



## Syntax

* Number...	is any number, decimal or hex prefixed with "$" or "0x". Numbers can also be simple bitwise expressions if prefixed with "=". e.g. =$02 | $01 | 256
* ;	is a comment
* dNumber	Sets the next value to output to be flagged with the _MEMWRITE output, waits for the next input clock
* sNumber	Sets the output but does not wait for the input clock
* <NumberBitIndex,NumberBitIndex,...,@NumberByteOffset,>NumberLength,<Filename	Reads binary data from a file to the NumberBitIndex list, using an optional NumberByteOffset and optional NumberLength, from optional Filename
* +NumberBitIndex,NumberBitIndex,...,+	Sets which bit indexes are auto-incremented with every data byte output
* bNumber	Using the current input NumberBitIndex list, only set those bits from the Number and wait for the next input clock
* ^+Number	Waits for the input data Number positive edge
* ^-Number	Waits for the input data Number negative edge
* wNumberMask,NumberValue	Waits for the input data NumberValue using the mask wNumberMask
* @time:Number	Waits for the simulation time Number to be reached
* !Filename	Includes the new Filename from this point, parsing the lines until the end of the included file, then the parsing continues from the next line
* .label = value	Assigns a string replacement label with a value
* :emit text message	Writes a commented text message to all active recording instances. Very useful when trying to synchronise test input dat with output recorded data.

