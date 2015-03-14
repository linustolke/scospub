# Perform a static check #

The CheckHost does this continously.
This is taken care of by the BOINC client and the [Application](Application.md).
  1. Download a work package from the CentralHost.
  1. Start the work package:
    1. Download the appropriate tool
    1. Download the source from the project source repository
  1. Run the tool
  1. Upload the result.
  1. The CentralHost enters the result in a database.

The result could also be uploaded to some project repository for published result.