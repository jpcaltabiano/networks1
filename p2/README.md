How to run:
Navigate to the directory, run `make clean` then `make` then `./p2`

Note 1: (Important) My code works perfectly fine when TracingLevel = 5. However, for some reason I have not been able to find out, changing the TracingLevel value to anything other than 5 causes the risk of the program running incorrectly to increase significantly. This is increasibly apparent for larger message numbers, ie messages to send > roughly 100. I tried running the program with differing variables with tracing level = 5 100 times. The errors seen when tracing level = 1 were not present for any of the runs. 

I have no idea why the tracing level affects performance and cannot being to explain this phenomena. I thought it may have something to do with calculating RTT on the fly, but a static RTT value did not change anything. Although incredibly frustrating, I cannot fix this problem. To the grader, if you would be so kind as to please try to run my program with trace level = 5 to see the intended, correct behavior. If the message number is relatively small there should be no issues, but if it is roughly greater than 100, there can be what appears to be continuous timerinterrupts and no termination. If this occurs please please try running with trace level = 5 to see that it really does work. 

Note 2: There is a rare chance that up to 1 packet my be received incorrectly because of sheer luck (or lack thereof).
For example, using my checksum algorithm, the following messages have the same checksum if their seqnum and acknums are correct and the same.

dPMVY]`cfilo2ux{~$'*

bPMVY]`clifo3ux{~$'*

The differences are small and end up evening out the checksums, even though my checksum algorithm is dependent on message data, order of chars in message, seqnum, and acknum