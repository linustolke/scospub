The first attempt was with running the wrapper application and just generating
a job.xml file containing commands.

This didn't work very well during the development of the other parts since a small error in the script made a command fail and then the wrapper restarted this command indefinately.

If there is some problem for the client, downloading from the repository or running the tool, it would be better if the client fails completely.

Another problem was that extra "tools" had to be distributed to start svn and java.