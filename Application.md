# Application #

The Application is a BOINC application.
It is downloaded from the server and run on each CheckHost.

These could exist for several combinations of OS and CPU according to the BOINC
handling of different system types.

The requirements are:
  * Simple, straight-forward.
  * Linked with the BOINC library and implementing the BOINC API.

Use a purposeful [[scospub application](scospubapp.md)] that knows the concept of repositories (subversion and whatever). The application starts the correct application and handles problems in a smart way. There are some ReasonsForNotUsingTheWrapperApplication.
It could also eventually be improved to do some basic security checks on the invoked programs.

The specification on what to run is partly handled by the [[scospub\_work\_generator](scospub_work_generator.md)] work generator and partly by the Application. It is communicated in the control file, that is fed to the Application using the name sc.xml.

## Location of checked out code ##

In order not to affect the normal boinc handling of files,
the checked out files from the project's repository reside in a separate directory.
The layout is:
> /tmp/scospub/

&lt;project&gt;

/<configured path>

All instances of applications from the same 

&lt;OSPROJ&gt;

 share the same
directory so
some locking or other protection mechanism
is needed to allow
multiple boinc clients.

## Syntax of Control File ##

Similar to that of the wrapper application.

### Layout ###
```
<scospub>
  <project id='2'>argouml-core-model</project>
  <team id='1'>argouml</team>
  <tool id='3'>checkstyle</tool>
  <config>sunchecks</config>
  <source>
    <svn id='4'>
      <url>http://argouml.tigris.org/svn/argouml/trunk/src/argouml-core-model/src</url>
      <checkoutdir>dir</checkoutdir>
      <username>guest</username>
      <password></password>
      <revision>15021</revision>
    </svn>
  </source>
  <javatask>
    <stdout_filename>out1</stdout_filename>
    <stderr_filename>out2</stderr_filename>
    <command_line>-cp checkstyle-all-4.3.jar com.puppycrawl.tools.checkstyle.Main -c checkstyle-sun_checks.xml</command_line>
    <args>-r %r/dir</args>
    <ignore_exit>1</ignore_exit>
  </javatask>
</scospub>
```

#### project ####
#### team ####
#### tool ####
#### config ####
#### Tasks ####
##### task #####
##### javatask #####