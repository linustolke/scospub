# Step by Step instruction #

  1. Install the dependencies for boinc (server part from http://boinc.berkeley.edu/trac/wiki/SoftwarePrereqsUnix)
  1. Create a user for this (_scospub_)
  1. Change directory to a build directory in the user home directory
```
$ su scospub
$ cd
$ mkdir svn
$ cd svn
$ 
```
  1. Check out the boinc server project (according to http://boinc.berkeley.edu/trac/wiki/SourceCode):
```
$ svn co http://boinc.berkeley.edu/svn/branches/server_stable boinc
$ 
```
  1. Build the boinc project, see http://boinc.berkeley.edu/trac/wiki/BuildSystem
```
$ cd boinc
$ ./_autosetup
$ ./configure -C --disable-client
$ make
$ 
```
  1. Install the boinc server part see http://boinc.berkeley.edu/trac/wiki/MakeProject
```
$ cd tools
$ ./make_project my-scospub-installation
$ cd ../..
$ 
```
  1. Check out the SCOSPUB software:
```
$ svn co http://scospub.googlecode.com/svn/trunk scospub
$ 
```
  1. Configure the project:
```
$ cd scospub
$ ./_autosetup
$ ./configure -C --with-boinc=`pwd`/../boinc --with-project=/where/you/installed/the/boinc/server/project
$ make
$ make install
$ 
```
> > Your server project is by default in $HOME/projects/my-scospub-installation.
  1. Add paths to the Apache web server configuration and restart the server.
```
As root:
# cp htmp/scospub.httpd.conf /etc/apache2/sites-available
# ln -s /etc/apache2/sites-available/scospub.httpd.conf /etc/apache2/sites-enabled/400-scospub
# apache2ctl restart
# 
```
  1. Add scospubapp as an application in /where/you/installed/the/boinc/server/project/project.xml
```
  ...
  <app>
    <name>scospubapp</name>
    <user_friendly_name>The SCOSPUB Application</user_friendly_name>
  </app>
  ...
</boinc>
```
  1. Add work generator, validator and assimilator as daemons in /where/you/installed/the/boinc/server/project/config.xml
```
  ...
  <daemons>
    ...
    <daemon>
      <cmd>
        scospub_work_generator
      </cmd>
    </daemon>
    <daemon>
      <cmd>
        sample_bitwise_validator -app scospubapp
      </cmd>
    </daemon>
    <daemon>
      <cmd>
        scospub_assimilator -app scospubapp
      </cmd>
    </daemon>
    ...
  </daemons>
  ...
</boinc>
```
  1. Activate the application and servers.
```
$ cd /where/you/installed/the/boinc/server/project/
$ bin/xadd
$ bin/update_versions
<answer yes!>
$ bin/stop
$ bin/start
$ 
```


# Prerequisites #

  1. Same prerequisites as used by boinc server.
  1. subversion client installed and network connection available.

# Notes #

Linus is working on a debian host so there might be debian-specific stuff
in the project. If you find anything, lets fix it.

## Notes on configuring BOINC ##

locality\_scheduling is a good idea to enable for the project.
The big files would then be the static check tools.

# Downloads #

There are no provided downloads. You need to work from the repository.