<?php
$cvs_version_tracker[]="\$Id: configsource.php$";  //Generated automatically - do not edit

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/translation.inc");

init_session();
db_init();

$project = $_REQUEST["project"];

if (!$project || $project <= 0) {
    // We don't know what project this is.
    header("Location: projects.php");
    exit;
}

$id = $_REQUEST["id"];

$url = $_REQUEST["url"];
$username = $_REQUEST["username"];
$password = $_REQUEST["password"];
if ($_REQUEST["active"]) {
    $active = 1;
} else {
    $active = 0;
}

$type=1;

if ($url && $url != "") {
    if (!$id) {
	mysql_query("INSERT INTO scos_source "
	    . "SET project='$project', type=$type, "
	    . "url='$url', username='$username', password='$password', "
	    . "valid='unknown', active=1");
	$result = mysql_query("SELECT LAST_INSERT_ID() AS id");
	$id = mysql_fetch_object($result)->id;
    } else {
	mysql_query("UPDATE scos_source "
	    . "SET project='$project', type=$type, "
	    . "url='$url', username='$username', password='$password', "
	    . "valid='unknown', active=$active "
	    . "WHERE id=$id");
    }
    header("Location: ?project=$project&id=$id");
    exit;
} else {
    if ($_REQUEST["delete"] == "yes") {
	mysql_query("DELETE FROM scos_source "
	    . "WHERE project=$project AND id=$id");
	header("Location: projects.php");
        exit;
    }
}

page_head(tr(SCOSC_TITLE));

echo "<A HREF='projects.php'>Projects</A>
";

$type = 0;
$url = "";
$username = "";
$password = "";
$rooturl = "";
$uuid = "";
$lastrevision = 0;
$valid = "";
$active = -1;

if ($id > 0) {
    $result = mysql_query("SELECT * FROM scos_source "
	. "WHERE id=$id AND project=$project");

    if ($val = mysql_fetch_object($result)) {
	$project = $val->project;
	$type = $val->type;
	$url = $val->url;
	$username = $val->username;
	$password = $val->password;
	$rooturl = $val->rooturl;
	$uuid = $val->uuid;
	$lastrevision = $val->lastrevision;
	$valid = $val->valid;
	$active = $val->active;
    }
    mysql_free_result($result);
}

echo "
<FORM METHOD='get' ACTION='configsource.php'>
  <INPUT type='hidden' name='project' value='$project'/>
  <INPUT type='hidden' name='id' value='$id'/>
";

echo "
  <TABLE>
    <TR>
      <TD>Type:</TD>
      <TD>$type</TD>
    </TR>
    <TR>
      <TD>URL:</TD>
      <TD><INPUT type='text' name='url' value='$url' size='70'/></TD>
    </TR>
    <TR>
      <TD>Username:</TD>
      <TD><INPUT type='text' name='username' value='$username'/></TD>
    </TR>
    <TR>
      <TD>Password:</TD>
      <TD><INPUT type='password' name='password' value='$password'/></TD>
    </TR>
    <TR>
      <TD>Rooturl:</TD>
      <TD>$rooturl</TD>
    </TR>
    <TR>
      <TD>UUID:</TD>
      <TD>$uuid</TD>
    </TR>
    <TR>
      <TD>Last revision:</TD>
      <TD>$lastrevision</TD>
    </TR>
    <TR>
      <TD>Valid:</TD>
      <TD>$valid</TD>
    </TR>
    <TR>
      <TD>Active:</TD>
      <TD><INPUT type='checkbox' name='active'";
if ($active != 0) {
    echo ' checked=1';
}
echo "/>
    </TR>
  </TABLE>
  <INPUT type='submit' value='Submit'/>
</FORM>
<FORM METHOD='get' ACTION='configsource.php'>
  <INPUT type='hidden' name='project' value='$project'/>
  <INPUT type='hidden' name='id' value='$id'/>
  <INPUT type='hidden' name='delete' value='yes'/>
  <INPUT type='submit' value='Delete'/>
</FORM>
";

if ($valid == "unknown") {
    echo "
    <FORM METHOD='get' ACTION='configsource.php'>
	<INPUT type='hidden' name='project' value='$project'/>
	<INPUT type='hidden' name='id' value='$id'/>
	<INPUT type='submit' value='Check for validity'/>
    </FORM>
    ";
}

page_tail();

?>
