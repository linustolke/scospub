# Server side tools #

## Prepare work units ##

Workunits are prepared by the scospub\_work\_generator program.

The program regularly:
  * Fetches a list of all projects and their sources.
  * Contact the source repositories to check for changes.
  * If any changes is seen in the repository, a new work unit is created and fed into the boinc structure.

It also regularly:
  * Checks to see if there is a new source path configured and tests that for validity.

## Handle result from [Application](Application.md) ##

When the [Application](Application.md) has uploaded a result, the result is validated.
Initially the simple\_bitwise\_validator is used from the boinc installation.

Then the information is assimilated.
The scospub\_assimilator program takes care of this.
The result is then added into the scospub-specific tables in the database and can then be seen by the result presentation.

## CentralHost web server installation ##

The presentation of results and the configuration of projects uses
the boinc server web pages.
Including:
  * Base url
  * Web server configuration
  * php include files
  * Database (new tables for the scospub-level results)

### Changes in configuration ###

The project configuration files are in html/scosconf.
/scosconf in the URL.

The scripts change the database directly.

### Present the result ###

The presentation of results is in the html/scospres
both in the scospub project and in the installed
boinc server web pages.
It is /scospres in the URL.

The scripts fetch information directly from the database to present it.


## File/Project layout ##

From the SCOSPUB perspective it is the tool that is important and
not what OS project it is applied to.
From the CheckHost and CheckHost administrators perspective,
each tool is a new download from the CentralHost and it could be used on several projects.
The large-files boinc mechanism could be used to help in this.

This also means that we can install the [Application](Application.md) and tools directly from
Makefiles in the scosconf project.

There are also some ReasonsForHavingFileLayoutByProject.

In the projects/

&lt;WHATEVER&gt;

 directory on the CentralHost
the following files are installed by "make install" in the scospub project:

> apps/scospubapp/scospubapp

&lt;VERSION&gt;



&lt;PLATFORM&gt;

/scospubapp

&lt;VERSION&gt;



&lt;PLATFORM&gt;



This is then copied to the download directory by the bin/update\_versions function meaning that after every new version of the scospubapp the script must be run.

All tools are treated as input files and downloaded using the Boinc file download mechanism. They are copied, together with the config files needed, to the download directory and then copied to the right download directory by the boinc infrastructure.

Templates are simplified.

The following templates are installed:

> templates/scospub0\_wu
> templates/scospub0\_result
> templates/scospub1\_wu
> templates/scospub1\_result

where the number in the name reflects how many input files except for
job.xml that are provided and how many output files that are handled.

The job.xml is generated separate for every job
depending on the configurations of the project done
by the [OpenSourceProjectAdministator](OpenSourceProjectAdministator.md)s
using the web interface.

## The database ##

### Table scos\_project: Project information ###
One line per project.
Active controls if the project should be built or not.

Project 1 is a dummy template project.

### Table scos\_tool: References to tools ###
Immutable fields:
> Connection to project,
> Toolname,
> Config.
Active=true signify the current configuration for a project.
Active=false signify historic information only available for reference.

Templates for allowed tools are entered with connection to project as 1.
Active then means available or not.

### Table scos\_source: References to source tree paths ###
All fields immutable except active.
Active=true signify the current configuration for a project.
Active=false signify historic information only available for reference.

### Table scos\_result ###
One row for each reported result.
Each result references a tool (active or passive).

Each result references one or more sources tree paths and contains their
version/date using a scos\_result\_source\_reference table.