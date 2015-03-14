# Parts #

There are two essential parts of the project,
differing in how they are deployed and because of that they have
completely separate requirements.
  * ServerSideTools
  * [Application](Application.md)

## ServerSideTools ##

The ServerSideTools run, together with the BOINC server on the CentralHost to
  * Prepare work packages
  * Accept and take care of changes in configuration and new Projects
  * Handle result from application
  * Present the result
  * Do configuration of a Project

Essentially these are deployed only once.

## [Application](Application.md) ##

The [Application](Application.md) is a BOINC application.
It is downloaded from the server and run on each CheckHost.

These could exist for several combinations of OS and CPU according to the BOINC
handling of different system types.

# Other ideas #

We should strive to create as small work packages as possible.